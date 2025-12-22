#! /usr/bin/env python3
#
# Copyright 2026 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import json
import os
import re
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple, Sequence, Union

import urllib.request
import urllib.parse

GITLAB_API = (
    (f"{os.environ.get('CI_SERVER_URL', '').rstrip('/')}/api/v4")
    if os.environ.get("CI_SERVER_URL")
    else None
)
TOKEN = os.environ.get("BD_USER_TOKEN")
PROJECT_ID = os.environ.get("CI_PROJECT_ID")
MR_IID = os.environ.get("CI_MERGE_REQUEST_IID")
BD_URL = (
    os.environ.get("BLACKDUCK_URL", "").rstrip("/")
    if os.environ.get("BLACKDUCK_URL")
    else None
)
BD_API_TOKEN = os.environ.get("BLACKDUCK_API_TOKEN")
BD_PROJECT_NAME = os.environ.get("BLACKDUCK_PROJECT_NAME")


# Call GitLab API using PRIVATE-TOKEN auth and return
# (status_code, response_body) as a tuple. Fails fast when
# required CI variables are missing. Does not log secrets.
def api_request(method: str, path: str, data: Optional[dict] = None) -> Tuple[int, str]:
    if not TOKEN:
        raise SystemExit("GitLab token is required (BD_USER_TOKEN).")
    if not GITLAB_API:
        raise SystemExit("CI_SERVER_URL is not set; cannot form GitLab API URL.")
    url = f"{GITLAB_API}{path}"
    headers = {
        "PRIVATE-TOKEN": TOKEN,
        "Content-Type": "application/json",
    }
    payload = json.dumps(data).encode("utf-8") if data is not None else None
    req = urllib.request.Request(url, data=payload, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req) as resp:
            return resp.getcode(), resp.read().decode("utf-8")
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode("utf-8", errors="replace")


# Fetch the merge request JSON. Used to obtain diff refs and
# metadata required for inline discussion positions.
def get_mr_details() -> Dict[str, Any]:
    code, body = api_request("GET", f"/projects/{PROJECT_ID}/merge_requests/{MR_IID}")
    if code != 200:
        raise SystemExit(f"Failed to fetch MR details: {code} {body}")
    return json.loads(body)


# Extract diff reference SHAs (base/start/head) for composing
# valid GitLab inline discussion positions.
def get_mr_diff_refs() -> Dict[str, str]:
    mr = get_mr_details()
    refs = mr.get("diff_refs") or {}
    return {
        "base_sha": refs.get("base_sha"),
        "start_sha": refs.get("start_sha"),
        "head_sha": refs.get("head_sha"),
    }


# Post an inline discussion comment on the MR at new_path:new_line.
# Returns True on success; False when GitLab rejects the position
# (e.g., file not part of the MR diff).
def post_inline_comment(
    refs: Dict[str, str], file_path: str, line: int, body: str
) -> bool:
    position = {
        "base_sha": refs["base_sha"],
        "start_sha": refs["start_sha"],
        "head_sha": refs["head_sha"],
        "position_type": "text",
        "new_path": file_path,
        "new_line": int(line),
    }
    payload = {"body": body, "position": position}
    code, resp = api_request(
        "POST",
        f"/projects/{PROJECT_ID}/merge_requests/{MR_IID}/discussions",
        payload,
    )
    if code == 201:
        return True
    sys.stderr.write(f"Inline comment failed for {file_path}:{line} -> {code} {resp}\n")
    return False


# Post a top-level MR note (non-inline). Useful for summaries and
# for findings that cannot be attached inline.
def post_general_note(body: str) -> None:
    payload = {"body": body}
    code, resp = api_request(
        "POST",
        f"/projects/{PROJECT_ID}/merge_requests/{MR_IID}/notes",
        payload,
    )
    if code not in (201, 200):
        sys.stderr.write(f"Failed to post MR note: {code} {resp}\n")


# Convert absolute paths to repo-relative paths when possible so
# GitLab can match them to files in the MR diff.
def convert_absolute_paths_to_relative(path_str: str) -> str:
    p = Path(path_str)
    if p.is_absolute():
        try:
            p = p.relative_to(Path.cwd())
        except Exception:
            # fallback to basename search in repo
            candidates = list(Path.cwd().rglob(p.name))
            if candidates:
                return candidates[0].as_posix()
            return p.name
    return p.as_posix()


# Minimal Black Duck REST client to locate BD project version and read
# unconfirmed snippet matches for file and line details.
class BlackDuckClient:
    # Create a client with BD base URL and long-lived API token.
    # Short-lived bearer/CSRF tokens are fetched on demand.
    def __init__(self, base_url: str, api_token: str):
        self.base = base_url.rstrip("/")
        self.api_token = api_token
        self.bearer: Optional[str] = None
        self.csrf: Optional[str] = None

    # Authenticate via /api/tokens/authenticate and cache bearer/CSRF
    # tokens. No-op if already authenticated.
    def _ensure_auth(self) -> None:
        if self.bearer:
            return
        url = f"{self.base}/api/tokens/authenticate"
        req = urllib.request.Request(url, method="POST")
        req.add_header("Authorization", f"token {self.api_token}")
        with urllib.request.urlopen(req) as resp:
            data = json.loads(resp.read().decode("utf-8", "ignore"))
            self.bearer = data.get("bearerToken")
            self.csrf = resp.headers.get("X-CSRF-TOKEN")

    # Issue an authenticated JSON request to a BD REST path with
    # optional query params. Returns parsed JSON (dict).
    def _request(
        self,
        method: str,
        path: str,
        params: Optional[Union[Dict[str, str], Sequence[Tuple[str, str]]]] = None,
    ) -> Dict[str, Any]:
        self._ensure_auth()
        url = f"{self.base}{path}" if path.startswith("/") else path
        if params:
            q = urllib.parse.urlencode(params)
            url = f"{url}?{q}"
        req = urllib.request.Request(url, method=method)
        if self.bearer:
            req.add_header("Authorization", f"Bearer {self.bearer}")
        if self.csrf:
            req.add_header("X-CSRF-TOKEN", self.csrf)
        req.add_header("Accept", "application/json")
        with urllib.request.urlopen(req) as resp:
            return json.loads(resp.read().decode("utf-8", "ignore"))

    # Find a project by exact name. Uses server-side filtering when
    # available; falls back to filtering client-side.
    def find_project(self, name: str) -> Optional[Dict[str, Any]]:
        try:
            data = self._request("GET", "/api/projects", params={"q": f"name:{name}"})
            for it in data.get("items", []):
                if it.get("name") == name:
                    return it
        except Exception:
            data = self._request("GET", "/api/projects")
            for it in data.get("items", []):
                if it.get("name") == name:
                    return it
        return None

    # Find a project version by name (e.g., MR-<iid>). Returns the
    # version object or None when not found.
    def find_version(
        self, project: Dict[str, Any], version_name: str
    ) -> Optional[Dict[str, Any]]:
        href = project.get("_meta", {}).get("href") or project.get("_meta", {}).get(
            "_href"
        )
        if not href:
            return None
        url = f"{href}/versions"
        try:
            data = self._request(
                "GET", url, params={"q": f"versionName:{version_name}"}
            )
            for it in data.get("items", []):
                if (
                    it.get("versionName") == version_name
                    or it.get("name") == version_name
                ):
                    return it
        except Exception:
            data = self._request("GET", url)
            for it in data.get("items", []):
                if (
                    it.get("versionName") == version_name
                    or it.get("name") == version_name
                ):
                    return it
        return None

    # Return a list of (file_path, start_line, end_line?) tuples for
    # unconfirmed snippet matches in the given project version.
    def find_unconfirmed_snippet_matches(
        self, version: Dict[str, Any]
    ) -> List[Tuple[str, int, Optional[int]]]:
        matches: List[Tuple[str, int, Optional[int]]] = []
        vhref = version.get("_meta", {}).get("href") or version.get("_meta", {}).get(
            "_href"
        )
        if not vhref:
            print("Version has no href metadata.")
            return matches

        m = re.search(r"/api/projects/([^/]+)/versions/([^/?#]+)", vhref)
        if not m:
            print("Version href does not match expected pattern.")
            return matches

        project_id, version_id = m.group(1), m.group(2)

        offset = 0
        limit = 100
        while True:
            params = [
                ("filter", "bomMatchType:snippet"),
                ("filter", "bomMatchReviewStatus:not_reviewed"),  # unconfirmed snippets
                ("limit", str(limit)),
                ("offset", str(offset)),
            ]
            api_path = (
                f"/api/internal/projects/{project_id}/versions/"
                f"{version_id}/source-bom-entries"
            )
            data = self._request(
                "GET",
                api_path,
                params=params,
            )

            items = data.get("items", [])
            if not items:
                break

            for it in items:
                snips = it.get("fileSnippetBomComponents") or []
                for s in snips:
                    # only unconfirmed snippets
                    if s.get("reviewStatus") != "NOT_REVIEWED":
                        continue

                    starts = s.get("sourceStartLines") or s.get("matchStartLines") or []
                    ends = s.get("sourceEndLines") or s.get("matchEndLines") or []

                    # choose a repo-relative path for GitLab comments
                    fp = (
                        it.get("commentPath")
                        or (it.get("compositePath") or {}).get("path")
                        or it.get("name")
                    )

                    for i, sl in enumerate(starts):
                        if isinstance(sl, str) and sl.isdigit():
                            sl = int(sl)
                        if not isinstance(sl, int):
                            continue
                        el = ends[i] if i < len(ends) else None
                        if isinstance(el, str) and el.isdigit():
                            el = int(el)
                        matches.append((fp, sl, el if isinstance(el, int) else None))

            # pagination
            if len(items) < limit:
                break
            offset += limit
        return matches


# Entry point: Find unconfirmed snippet matches and post inline
# MR comments or a short summary note.
def main() -> None:
    if not (PROJECT_ID and MR_IID and GITLAB_API and TOKEN):
        raise SystemExit("Missing required CI variables to post comments.")

    matches: List[Tuple[str, int, Optional[int]]] = []
    if BD_URL and BD_API_TOKEN and BD_PROJECT_NAME and MR_IID:
        try:
            bd = BlackDuckClient(BD_URL, BD_API_TOKEN)
            proj = bd.find_project(BD_PROJECT_NAME)
            if proj:
                ver = bd.find_version(proj, f"MR-{MR_IID}")
                if ver:
                    matches.extend(bd.find_unconfirmed_snippet_matches(ver))
        except Exception as e:
            sys.stderr.write(f"Black Duck REST lookup failed: {e}\n")

    if not matches:
        post_general_note(
            (
                "[Black Duck] Unconfirmed snippet policy triggered, "
                "but file/line details were not found. "
                "Please review Black Duck findings for this MR."
            )
        )
        return

    refs = get_mr_diff_refs()
    fallbacks: List[str] = []
    for fp, start, end in matches:
        rel = convert_absolute_paths_to_relative(fp)
        line_range = f"{start}-{end}" if end and end != start else str(start)
        body = f"* Unconfirmed snippet match in {rel} at lines {line_range}.\n"
        if not post_inline_comment(refs, rel, start, body):
            fallbacks.append(body)

    if fallbacks:
        fallback_text = "\n".join(fallbacks)
        post_general_note(
            "Some findings could not be attached inline (file(s) not in diff):\n\n"
            f"{fallback_text}"
        )


if __name__ == "__main__":
    main()
