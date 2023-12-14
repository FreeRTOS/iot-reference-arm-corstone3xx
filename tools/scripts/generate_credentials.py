#! /usr/bin/env python3
#
# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import argparse
import datetime
from pathlib import Path
from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes


def main(args):
    key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
    )
    with open(Path(args.private_key_out_path).parent / "private_key.pem", "wb") as f:
        f.write(
            key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption(),
            )
        )

    subject = issuer = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, args.certificate_country_name),
            x509.NameAttribute(
                NameOID.STATE_OR_PROVINCE_NAME, args.certificate_state_province_name
            ),
            x509.NameAttribute(NameOID.LOCALITY_NAME, args.certificate_locality_name),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, args.certificate_org_name),
            x509.NameAttribute(
                NameOID.ORGANIZATIONAL_UNIT_NAME, args.certificate_org_unit_name
            ),
            x509.NameAttribute(
                NameOID.EMAIL_ADDRESS, args.certificate_email_address_name
            ),
        ]
    )
    cert = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(datetime.datetime.now(datetime.timezone.utc))
        .not_valid_after(
            datetime.datetime.now(datetime.timezone.utc)
            + datetime.timedelta(days=int(args.certificate_valid_time))
        )
        .sign(key, hashes.SHA256())
    )

    with open(Path(args.certificate_out_path).parent / "certificate.pem", "wb") as f:
        f.write(cert.public_bytes(serialization.Encoding.PEM))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--certificate_valid_time",
        help="the days until the certificate's expiration",
        default="365",
        required=False,
    )
    parser.add_argument(
        "--certificate_country_name",
        help="used for the certificate's meta data",
        required=True,
    )
    parser.add_argument(
        "--certificate_state_province_name",
        help="used for the certificate's meta data",
        required=True,
    )
    parser.add_argument(
        "--certificate_locality_name",
        help="used for the certificate's meta data",
        required=True,
    )
    parser.add_argument(
        "--certificate_org_name",
        help="used for the certificate's meta data",
        required=True,
    )
    parser.add_argument(
        "--certificate_org_unit_name",
        help="used for the certificate's meta data",
        default="",
        required=False,
    )
    parser.add_argument(
        "--certificate_email_address_name",
        help="used for the certificate's meta data",
        default="",
        required=False,
    )
    parser.add_argument(
        "--certificate_out_path",
        help="the path where certificate.pem will be generated",
        default=".",
        required=False,
    )
    parser.add_argument(
        "--private_key_out_path",
        help="the path where private_key.pem will be generated",
        default=".",
        required=False,
    )
    main(parser.parse_args())
