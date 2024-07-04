#!/usr/bin/python3

#  Copyright (c) 2023-2024 Arm Limited. All rights reserved.
#  SPDX-License-Identifier: Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

# Usage can be found at : /docs/components/aws_iot/aws_tool.md

from enum import Enum
import os
import time
import traceback as tb
import re
from functools import reduce

# JSON is used for policy creation
import json

# used to communicate with AWS services.
import boto3
import botocore

# used for the CLI, and argument parsing.
import click
import logging

DEFAULT_LOG_LEVEL = "warning"
# Default path of OTA binary directory is build/
DEFAULT_BUILD_DIR = "build"
DEFAULT_CREDENTIALS_PATH = "certificates"
DEFAULT_OTA_BINARY = "keyword-detection-update_signed.bin"
# Signature used to sign new firmware (update) image.
# Can be found in the build directory.
# If a file not found error occurs due to this file,
# try to build one of the FRI applications (e.g. keyword-detection)
DEFAULT_OTA_UPDATE_SIGNATURE_FILENAME = "update-signature.txt"

CREATE_NEW_CERTIFICATE = ""

OTA_JOB_NAME_PREFIX = "AFR_OTA-"

for var in [
    "AWS_REGION",
    "AWS_ACCESS_KEY_ID",
    "AWS_SECRET_ACCESS_KEY",
    "AWS_SESSION_TOKEN",
]:
    if not os.getenv(var):
        raise ValueError(var + " is not set in environment")

AWS_REGION = os.getenv("AWS_REGION")
iot = boto3.client("iot", AWS_REGION)
s3 = boto3.client("s3", AWS_REGION)
iam = boto3.client("iam", AWS_REGION)
sts_client = boto3.client("sts", AWS_REGION)

validCredentials = False
try:
    sts_client.get_caller_identity()
    validCredentials = True
except botocore.exceptions.ClientError:
    pass
if not validCredentials:
    raise ValueError(
        "Your AWS client API keys (which include 'AWS_SESSION_TOKEN') "
        "are invalid or expired. Try re-exporting them."
    )


@click.group()
def cli():
    pass


def read_whole_file(path, mode="r"):
    with open(path, mode) as fp:
        return fp.read()


class ApplicationType(Enum):
    # These must be the subdirectory for each application
    # in the `applications` folder.
    KEYWORD_DETECTION = "keyword_detection"
    OBJECT_DETECTION = "object_detection"
    SPEECH_RECOGNITION = "speech_recognition"
    UNDEFINED = "DEFAULT"

    def app_type_from_string(s):
        for app in AWS_APPLICATIONS:
            if s == app.value:
                return app
        return ApplicationType.UNDEFINED


AWS_APPLICATIONS = [
    ApplicationType.KEYWORD_DETECTION,
    ApplicationType.OBJECT_DETECTION,
    ApplicationType.SPEECH_RECOGNITION,
]


class Flags:
    def __init__(
        self,
        bucket_name="",
        role_name="",
        build_dir=DEFAULT_BUILD_DIR,
        ota_binary=DEFAULT_OTA_BINARY,
        ota_update_signature_filename=DEFAULT_OTA_UPDATE_SIGNATURE_FILENAME,
        target_application=ApplicationType.UNDEFINED,
    ):
        """
        Create an object holding metadata needed for OTA with AWS.

        Parameters:
        build_dir: directory to where the OtA update binary is.
            E.g. "build/" contains "keyword-detection-update_signed.bin"
        """
        self.BUILD_DIR = build_dir
        self.AWS_ACCOUNT_ID = boto3.client("sts").get_caller_identity().get("Account")
        self.AWS_ACCOUNT_ARN = boto3.client("sts").get_caller_identity().get("Arn")
        self.THING_NAME = None
        self.THING_ARN = None
        self.POLICY_NAME = None
        self.OTA_BUCKET_NAME = bucket_name
        self.OTA_ROLE_NAME = role_name
        self.OTA_BINARY = ota_binary
        self.OTA_BINARY_FILE_PATH = os.path.join(self.BUILD_DIR, self.OTA_BINARY)
        self.UPDATE_ID = None
        self.OTA_UPDATE_PROTOCOLS = ["MQTT"]
        self.OTA_UPDATE_TARGET_SELECTION = "SNAPSHOT"
        self.bucket = None
        self.file = None
        self.thing = None
        self.policy = None
        self.keyAndCertificate = None
        self.certificate = None
        self.privateKey = None
        self.publicKey = None
        self.endPointAddress = None
        self.role = (
            None
            if not _does_role_exist(role_name)
            else iam.get_role(RoleName=role_name)["Role"]
        )
        self.OTA_ROLE_ARN = None if self.role is None else self.role["Arn"]
        logging.info("Target application is: " + target_application.value)
        # Used to update 'aws_clientcredential.h'
        self.targetApplication = target_application
        # Used for error handling cleanup.
        self.bucketHasBeenCreated = False
        self.certificateHasBeenCreated = False
        self.roleHasBeenCreated = False
        self.updateHasBeenCreated = False
        self.policyHasBeenCreated = False
        self.thingHasBeenCreated = False
        # JSONS
        self.POLICY_DOCUMENT = {
            "Version": "2012-10-17",
            "Statement": [
                {
                    "Effect": "Allow",
                    "Action": [
                        "iot:Publish",
                        "iot:Receive",
                        "iot:Subscribe",
                        "iot:Connect",
                    ],
                    "Resource": [
                        "arn:aws:iot:" + AWS_REGION + ":" + self.AWS_ACCOUNT_ID + ":*"
                    ],
                }
            ],
        }
        self.ASSUME_ROLE_POLICY_DOCUMENT = {
            "Version": "2012-10-17",
            "Statement": [
                {
                    "Effect": "Allow",
                    "Principal": {
                        "AWS": self.AWS_ACCOUNT_ARN,
                        "Service": [
                            "iot.amazonaws.com",
                            "s3.amazonaws.com",
                            "iam.amazonaws.com",
                        ],
                    },
                    "Action": ["sts:AssumeRole"],
                }
            ],
        }
        self.IAM_OTA_PERMISSION = {
            "Version": "2012-10-17",
            "Statement": [
                {
                    "Effect": "Allow",
                    "Action": ["iam:GetRole", "iam:PassRole"],
                    "Resource": "arn:aws:iam::"
                    + self.AWS_ACCOUNT_ID
                    + ":role/"
                    + self.OTA_ROLE_NAME,
                }
            ],
        }
        self.S3_OTA_PERMISSION = {
            "Version": "2012-10-17",
            "Statement": [
                {
                    "Effect": "Allow",
                    "Action": ["s3:GetObjectVersion", "s3:GetObject", "s3:PutObject"],
                    "Resource": ["arn:aws:s3:::" + self.OTA_BUCKET_NAME + "/*"],
                }
            ],
        }
        signaturePath = os.path.join(
            self.BUILD_DIR,
            ota_update_signature_filename,
        )
        signature = "unavailable"
        try:
            signature = read_whole_file(signaturePath)
        except FileNotFoundError:
            pass  # this check is done at the CLI stage.
        self.OTA_UPDATE_FILES = [
            {
                "fileName": "non_secure image",
                "fileLocation": {
                    "s3Location": {
                        "bucket": self.OTA_BUCKET_NAME,
                        "key": self.OTA_BINARY,
                    }
                },
                "codeSigning": {
                    "customCodeSigning": {
                        "signature": {
                            "inlineDocument": bytearray(
                                signature.strip(),
                                "utf-8",
                            )
                        },
                        "certificateChain": {"certificateName": "0"},
                        "hashAlgorithm": "SHA256",
                        "signatureAlgorithm": "RSA",
                    },
                },
            }
        ]


def set_log_level(loglevel):
    logging.basicConfig(level=loglevel.upper())


class StdCommand(click.core.Command):
    def __init__(self, *args, **kwargs):
        """
        Defines parsing of command line arguments, e.g. routes
        'delete-thing' or 'delete-policy' arguments to their
        respective functions.
        See click API for more info.

        Parameters:
        *args: usually an empty tuple.
        **kwargs (dict): E.g.
            {'name': 'delete-thing',
            'callback': <function delete_thing>,
            'params': [<Option thing_name>,
            <Option prune>],
            'help': None}
        """
        super().__init__(*args, **kwargs)
        self.params.insert(
            0,
            click.core.Option(
                ("--log_level",),
                help="Provide logging level. \
                    Example --log_level debug, default="
                + DEFAULT_LOG_LEVEL,
                default=DEFAULT_LOG_LEVEL,
            ),
        )

    def invoke(self, ctx):
        set_log_level(ctx.params["log_level"])
        super().invoke(ctx)


# same as the above, but with the default log level set at info,
# so that listing functions do print by default
class ListingCommand(click.core.Command):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.params.insert(
            0,
            click.core.Option(
                ("--log_level",),
                help="Provide logging level. \
                    Example --log_level debug, default="
                + "info",
                default="info",
            ),
        )

    def invoke(self, ctx):
        set_log_level(ctx.params["log_level"])
        super().invoke(ctx)


def _does_thing_exist(thing_name):
    """
    Parameters:
    thing_name (string): name of the AWS IoT Thing attached to the device.

    Returns:
    bool: True if thing exists on the AWS cloud.
        False otherwise.
    """
    try:
        iot.describe_thing(thingName=thing_name)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            return False
        else:
            raise ex
    return True


def _does_policy_exist(policy_name):
    """
    Parameters:
    policy_name (string): name of the policy (permnission set).

    Returns:
    bool: True if the policy exists on the AWS cloud.
        False otherwise.
    """
    try:
        iot.get_policy(policyName=policy_name)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            return False
        else:
            raise ex
    return True


def _does_job_exist(job_name):
    """
    Parameters:
    job_name (string): name of the job (set of operations).
        E.g. the OTA firmware download operation could be a job.

    Returns:
    bool: True if the job exists on the AWS cloud.
        False otherwise.
    """
    try:
        iot.describe_job(jobId=job_name)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            return False
        else:
            raise ex
    return True


def _does_bucket_exist(bucket_name):
    """
    Parameters:
    bucket_name (string): name of the bucket (cloud storage resource).

    Returns:
    bool: True if the bucket exists.
        False otherwise.
    """
    try:
        s3.get_bucket_location(Bucket=bucket_name)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "NoSuchBucket":
            return False
        else:
            raise ex
    return True


def _does_role_exist(iam_role_name):
    """
    Parameters:
    iam_role_name (string): name of the role (set of task-related permissions)

    Returns:
    bool: True if the role exists on the AWS cloud.
        False otherwise.
    """
    if iam_role_name is None or iam_role_name == "":
        return False
    try:
        iam.get_role(RoleName=iam_role_name)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "NoSuchEntity":
            return False
        else:
            raise ex
    return True


def key_to_pem_formatter(text):
    return (
        text.replace("\\n", "")
        .replace("\\", "")
        .replace('"    "', "\n")
        .replace('"', "")
        + "\n"
    )


def replace_between(start, end, replaceWith, target):
    return re.sub(start + "(.*?)" + end, start + replaceWith + end, target, re.DOTALL)


def _write_credentials(flags: Flags, credentials_path):
    """
    This function writes certificates ('credentials') for any Things
    created to credential files in the designated directory.
    Will create the 'credentials_path' directory if it does not exist.
    Credentials are found from the 'flags' parameter.

    This function assumes that the application type is not undefined.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
        (e.g. credentials).
    credentials_path (string): directory to write new files to.
    """
    fileDir = os.path.dirname(os.path.realpath("__file__"))
    # Create folder storing credentials if it does not exist.
    try:
        logging.info(
            "Creating/opening directory '"
            + credentials_path
            + "' to store credentials generated for Thing : '"
            + flags.THING_NAME
            + "'"
        )
        os.makedirs(os.path.join(fileDir, credentials_path), exist_ok=True)
    except Exception:
        err_msg = (
            "Failed to write credentials, could not make path: '"
            + credentials_path
            + "'. This may be due to an invalid path being provided. "
        )
        logging.error(err_msg)
    # Errors can occur here, e.g. due to invalid path names.
    # This script will not sanitise path inputs, so checking them is
    # the responsibility of the user.
    """
    Assuming the name of the new Thing is <your_thing_name>.
    The below section of code saves:
        1. the certificate of the new Thing to
            'thing_certificate_<your_thing_name>.pem.crt"
        2. the private key of the new Thing to
            'thing_private_key_<your_thing_name>.pem.key"
        3. the public key of the new Thing to
            'thing_public_key_<your_thing_name>.pem.key"
    """
    # This script does not sanitise THING_NAME for file name conventions.
    certificateFile = os.path.join(
        fileDir, credentials_path, "thing_certificate_" + flags.THING_NAME + ".pem.crt"
    )
    privateKeyFile = os.path.join(
        fileDir, credentials_path, "thing_private_key_" + flags.THING_NAME + ".pem.key"
    )
    publicKeyFile = os.path.join(
        fileDir, credentials_path, "thing_public_key_" + flags.THING_NAME + ".pem.key"
    )
    with open(certificateFile, "w") as file:
        file.write(key_to_pem_formatter(flags.certificate))
        logging.info("Saved Thing certificate to: " + certificateFile)
    with open(privateKeyFile, "w") as file:
        file.write(key_to_pem_formatter(flags.privateKey))
        logging.info("Saved Thing private key to: " + privateKeyFile)
    with open(publicKeyFile, "w") as file:
        file.write(key_to_pem_formatter(flags.publicKey))
        logging.info("Saved Thing public key to: " + publicKeyFile)
    """
    The below writes to `aws_clientcredential.h`.
    This can be specified to overwrite `aws_clientcredential.h`
    in an example application directory.
    """
    template_has_correct_format = (
        lambda contents: "clientcredentialMQTT_BROKER_ENDPOINT " in contents
        and "clientcredentialIOT_THING_NAME " in contents
    )
    template_has_been_edited = (
        lambda contents: "dummy.endpointid.amazonaws.com" not in contents
        or "dummy_thingname" not in contents
    )
    # Try to find a template for 'aws_clientcredential.h'
    template = ""
    # We assume flags.targetApplication is not ApplicationType.UNDEFINED
    credentialFileTemplate = os.path.join(
        fileDir,
        "applications/",
        flags.targetApplication.value,
        "configs/aws_configs/",
        "aws_clientcredential.h",
    )
    logging.info("Checking template: " + credentialFileTemplate)
    try:
        with open(credentialFileTemplate, "r") as file:
            template = file.read()
    except FileNotFoundError:
        logging.warning("The file '" + credentialFileTemplate + "' was not found")
    if not template_has_correct_format(template):
        err_msg = (
            "Your aws_clientcredential.h file at "
            + credentialFileTemplate
            + " has an incorrect format or does not exist. For this script to \
                work, please reset the `aws_clientcredential.h` file located \
                in your application directory to the original state. The \
                script relies on `clientcredentialMQTT_BROKER_ENDPOINT` and \
                `clientcredentialIOT_THING_NAME` being `#define`d within the \
                header file."
        )
        logging.warning(err_msg)
        return False
    if template_has_been_edited(template):
        warn_msg = (
            "Your aws_clientcredential.h file at "
            + credentialFileTemplate
            + " has been edited. "
        )
        logging.warning(warn_msg)
        option = input(
            "Overwrite the existing credentials at '"
            + credentialFileTemplate
            + "'? (Y/N) (Defaults to Y) "
        )
        if len(option) > 0 and option[0].lower() == "n":
            return False
    """
    The below code block modifies the example 'aws_clientcredential.h' found:
    1. Find the line which defines 'clientcredentialMQTT_BROKER_ENDPOINT'.
    2. Replace the default definition with the user's endpoint.

    Similarly update the definition of 'clientcredentialIOT_THING_NAME'.

    For example,
    `#define clientcredentialMQTT_BROKER_ENDPOINT    "dummy.endpointid.amazonaws.com"`
    becomes
    `#define clientcredentialMQTT_BROKER_ENDPOINT "<your_thing_name>"`.
    """
    template = replace_between(
        "clientcredentialMQTT_BROKER_ENDPOINT ",
        "\n",
        '"' + flags.endPointAddress["endpointAddress"] + '"',
        template,
    )
    template = replace_between(
        "clientcredentialIOT_THING_NAME ", "\n", '"' + flags.THING_NAME + '"', template
    )
    credentialFile = ""
    if flags.targetApplication in AWS_APPLICATIONS:
        credentialFile = os.path.join(
            fileDir,
            "applications/",
            flags.targetApplication.value,
            "configs/aws_configs/aws_clientcredential.h",
        )
    with open(credentialFile, "w") as file:
        file.write(template)
    logging.info("Wrote AWS client credentials to: " + credentialFile)
    return True


def _parse_credentials(flags: Flags):
    """
    Modifies the keys and certificates in 'flags' to ensure correct formatting.
    E.g. adds double quotations and next lines.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
              (e.g. credentials).
    """
    flags.certificate = repr(flags.keyAndCertificate["certificatePem"]).replace(
        r"'", r'"'
    )
    flags.certificate = flags.certificate.replace(r"\n", r'\\n"\\\n    "')
    flags.certificate = flags.certificate.replace(
        r'"-----END CERTIFICATE-----\\n"\\\n    ""',
        r'"-----END CERTIFICATE-----"\n\n\n',
    )
    flags.privateKey = repr(flags.keyAndCertificate["keyPair"]["PrivateKey"]).replace(
        r"'", r'"'
    )
    flags.privateKey = flags.privateKey.replace(r"\n", r'\\n"\\\n    "')
    flags.privateKey = flags.privateKey.replace(
        r'"-----END RSA PRIVATE KEY-----\\n"\\\n    ""',
        r'"-----END RSA PRIVATE KEY-----"\n\n\n',
    )
    flags.publicKey = repr(flags.keyAndCertificate["keyPair"]["PublicKey"]).replace(
        r"'", r'"'
    )
    flags.publicKey = flags.publicKey.replace(r"\n", r'\\n"\\\n    "')
    flags.publicKey = flags.publicKey.replace(
        r'"-----END PUBLIC KEY-----\\n"\\\n    ""', r'"-----END PUBLIC KEY-----"\n\n\n'
    )


def _create_credentials(flags: Flags, credentials_path):
    """
    For the metadata in flags, will attempt to create and save certificates
    to the directory provided.
    In the event of a failure (exception), removes all AWS resources created
    by this script (for the current command).

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
      (e.g. credentials).
    credentials_path (string): directory to write certificates generated to.

    Returns:
    bool: True if creation was successful. Throws an exception otherwise.
    """
    try:
        # KeyAndCertificate is of a Dictionary data type.
        flags.keyAndCertificate = iot.create_keys_and_certificate(setAsActive=True)
        flags.endPointAddress = iot.describe_endpoint(endpointType="iot:Data-ATS")
        _parse_credentials(flags)
        if not _write_credentials(flags, credentials_path):
            raise RuntimeError()
        logging.info(
            "New certificate ARN: " + flags.keyAndCertificate["certificateArn"]
        )
    except Exception as e:
        logging.error("Failed creating and saving credentials.")
        print_exception(e)
        cleanup_aws_resources(flags)
        raise e
    flags.certificateHasBeenCreated = True
    return True


def _get_credential_arn(flags: Flags, existing_certificate_arn, credentials_path):
    """
    Will create credentials (and save into 'credentials_path') if needed.
    Otherwise, if use_existing_credentials_arn is not the empty string (""),
    will use existing credentials.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
      (e.g. credentials).
    existing_certificate_arn (string): a string uniquely identifying
        an AWS resource. Otherwise known as an ARN.
    credentials_path: directory to write newly created credentials into.

    Returns:
    string: the ARN (uniquely identifying string) for the certificate
        provided or created.
    """
    if existing_certificate_arn == CREATE_NEW_CERTIFICATE:
        if not _create_credentials(flags, credentials_path):
            cleanup_aws_resources(flags)
            exit(1)
        certificate_arn = flags.keyAndCertificate["certificateArn"]
    else:
        certificate_arn = existing_certificate_arn
        flags.endPointAddress = iot.describe_endpoint(endpointType="iot:Data-ATS")
    return certificate_arn


def _create_thing(flags: Flags, Name, certificate_arn):
    """
    Create an AWS IoT Thing with certificate attached.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
      (e.g. credentials).
    certificate_arn (string): unique string identifying the certificate
        to attach to the Thing.

    Returns:
    bool: True if creation is successful
        False if the thing already exists.
        False if the certificate is not found.

    Raises:
    Exception: if any error except the provided certificate_arn's certificate
        not being found occurs.
    """
    try:
        if _does_thing_exist(flags.THING_NAME):
            # Then there's already a thing with that name.
            logging.warning("The thing named " + Name + " already exists.")
            return False

        flags.thing = iot.create_thing(thingName=flags.THING_NAME)
        logging.info("Created thing:" + flags.THING_NAME)

        iot.attach_thing_principal(
            thingName=flags.THING_NAME,
            principal=certificate_arn,
        )
        logging.info(
            "Attached Certificate: "
            + flags.endPointAddress["endpointAddress"]
            + " to Thing: "
            + flags.THING_NAME
        )
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            logging.error("No certificate found with the ARN: " + certificate_arn)
            return False
    except Exception as e:
        logging.error("failed creating a thing")
        print_exception(e)
        cleanup_aws_resources(flags)
        raise e
    flags.thingHasBeenCreated = True
    return True


# Define command-line interface for Thing creation command.
@cli.command(cls=StdCommand)
@click.option(
    "--thing_name", prompt="Enter Thing Name", help="Name of Thing to be created."
)
@click.option(
    "--existing_certificate_arn",
    default=CREATE_NEW_CERTIFICATE,
    help="Use the provided certificate ARN instead of creating a new one",
)
@click.option(
    "-a",
    "--credentials_path",
    default=DEFAULT_CREDENTIALS_PATH,
    show_default=True,
    help="The path where the credentials are stored. If not specified, \
         credentials in certificates are updated",
)
@click.option(
    "--build_dir",
    help="Override the default build directory",
    default=DEFAULT_BUILD_DIR,
)
@click.option(
    "--ota_binary",
    help="Override the default OTA file used",
    default=DEFAULT_OTA_BINARY,
)
@click.option(
    "--target_application",
    required=True,
    help="Updates the target application's aws_clientcredential.h automatically. \
        Accepted values: "
    + reduce(
        lambda y, z: y + ", " + z, map(lambda x: "'" + x.value + "'", AWS_APPLICATIONS)
    )
    + ".",
)
@click.pass_context
def create_thing_only(
    ctx,
    thing_name,
    existing_certificate_arn,
    credentials_path,
    build_dir,
    ota_binary,
    target_application,
    log_level,
):
    target = ApplicationType.app_type_from_string(target_application)
    if target_application == ApplicationType.UNDEFINED:
        logging.error(
            "The provided target application '"
            + target_application
            + "' is not passed using the CLI."
        )
        ctx.exit(1)
    ctx.flags = Flags(
        build_dir=build_dir,
        ota_binary=ota_binary,
        target_application=target,
    )
    ctx.flags.THING_NAME = thing_name
    certificate_arn = _get_credential_arn(
        ctx.flags, existing_certificate_arn, credentials_path
    )

    if not _create_thing(ctx.flags, thing_name, certificate_arn):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    ctx.exit(0)


def _create_policy(flags: Flags, Name, certificate_arn):
    """
    Creates an AWS policy in the region specified by your environment
    variables.
    Will delete this command's results if policy creation fails.
    E.g. cannot create-thing-and-policy and only the Thing is created.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
      (e.g. credentials).
    Name (string): name of the policy to be created.
    certificate_arn (string): the ARN, used to find a certificate for
        the policy.

    Returns:
    bool: True on successful creation.
        False if the policy already exists.
        False if 'certificate_arn' is invalid (the certificate does not exist).

    Raises:
    Exception: if any exception occurs other than the policy already existing
        or a certificate not being found.
    """
    if certificate_arn is None:
        logging.error(
            "Cannot have undefined 'certificate_arn' " "when creating a policy."
        )
        return False
    try:
        flags.POLICY_NAME = Name
        flags.policy = iot.create_policy(
            policyName=flags.POLICY_NAME,
            policyDocument=json.dumps(flags.POLICY_DOCUMENT),
        )

        iot.attach_principal_policy(
            policyName=flags.POLICY_NAME,
            principal=certificate_arn,
        )
        logging.info(
            "Attached Policy: "
            + flags.POLICY_NAME
            + " to Certificate: "
            + flags.endPointAddress["endpointAddress"]
        )
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceAlreadyExistsException":
            logging.warning("The policy named " + Name + " already exists")
            return False
        elif ex.response["Error"]["Code"] == "ResourceNotFoundException":
            logging.error("No certificate found with the ARN: " + certificate_arn)
            return False
        else:
            logging.error("failed creating a policy")
            print_exception(ex)
            cleanup_aws_resources(flags)
            raise ex
    except Exception as e:
        logging.error("failed creating a policy")
        print_exception(e)
        cleanup_aws_resources(flags)
        raise e
    flags.policyHasBeenCreated = True
    return True


# Define command-line interface for Policy creation command.
@cli.command(cls=StdCommand)
@click.option(
    "--policy_name", prompt="Enter Policy Name", help="Name of Policy to be created."
)
@click.option(
    "--thing_name",
    prompt="Enter Thing Name",
    help="If you create a new certificate, you must name the thing\
          this credential will be attached to.",
)
@click.option(
    "--existing_certificate_arn",
    # default must be "" for compatibility with get_credential_arn()
    default=CREATE_NEW_CERTIFICATE,
    help="Use the provided certificate ARN instead of creating a new one",
)
@click.option(
    "-a",
    "--credentials_path",
    default=DEFAULT_CREDENTIALS_PATH,
    show_default=True,
    help="The path where the credentials are stored. If not specified, \
        credentials in certificates are updated",
)
@click.option(
    "--build_dir",
    help="Override the default build directory",
    default=DEFAULT_BUILD_DIR,
)
@click.option(
    "--ota_binary",
    help="Override the default OTA file used",
    default=DEFAULT_OTA_BINARY,
)
@click.pass_context
def create_policy_only(
    ctx,
    policy_name,
    thing_name,
    existing_certificate_arn,
    credentials_path,
    build_dir,
    ota_binary,
    log_level,
):
    ctx.flags = Flags(
        build_dir=build_dir,
        ota_binary=ota_binary,
    )
    ctx.flags.THING_NAME = thing_name
    ctx.flags.POLICY_NAME = policy_name
    if existing_certificate_arn is None:
        # then we create a new credential, and require a thing name
        if thing_name is not None:
            ctx.flags.THING_NAME = thing_name
        else:
            logging.error(
                (
                    "No thing name specified. Use an existing certificate with "
                    '"--existing_certificate_arn" or create a new one by '
                    'specifying a thing name with "--thing_name"'
                )
            )
            ctx.exit(1)

    certificate_arn = _get_credential_arn(
        ctx.flags, existing_certificate_arn, credentials_path
    )

    if not _create_policy(ctx.flags, policy_name, certificate_arn):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    ctx.exit(0)


def _create_aws_bucket(flags: Flags, Name):
    """
    Creating AWS S3 Bucket.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
      (e.g. credentials).
    Name (string): name of the bucket to be created.

    Returns:
    bool: True if creation is successful.
        False if a bucket with that Name already exists.
    """

    if _does_bucket_exist(Name):
        logging.warning("The bucket named " + Name + " already exists")
        return False
    # checking existance and creating can conflict if run too close
    # and throw an error
    time.sleep(1)
    try:
        flags.OTA_BUCKET_NAME = Name
        flags.bucket = s3.create_bucket(
            Bucket=flags.OTA_BUCKET_NAME,
            ACL="private",
            CreateBucketConfiguration={"LocationConstraint": AWS_REGION},
        )
        logging.info("Created bucket: " + flags.OTA_BUCKET_NAME)
        s3.put_bucket_versioning(
            Bucket=flags.OTA_BUCKET_NAME, VersioningConfiguration={"Status": "Enabled"}
        )
    except Exception as e:
        logging.error("failed creating an aws bucket")
        print_exception(e)
        cleanup_aws_resources(flags)
        raise e
    flags.bucketHasBeenCreated = True  # for cleanup
    return True


# Define command-line interface for Bucket creation command.
@cli.command(cls=StdCommand)
@click.option(
    "--bucket_name", prompt="Enter Bucket Name", help="Name of Bucket to be created."
)
@click.option(
    "--build_dir",
    help="Override the default build directory",
    default=DEFAULT_BUILD_DIR,
)
@click.option(
    "--ota_binary",
    help="Override the default OTA file used",
    default=DEFAULT_OTA_BINARY,
)
@click.pass_context
def create_bucket_only(ctx, bucket_name, build_dir, ota_binary, log_level):
    ctx.flags = Flags(
        build_dir=build_dir,
        ota_binary=ota_binary,
    )
    if not _create_aws_bucket(ctx.flags, bucket_name):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    ctx.exit(0)


def _create_iam_role(flags: Flags, Name, permissions_boundary=None):
    """
    Create OTA update service role.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
    (e.g. credentials).
    Name (string): name of the role to be created.
    permissions_boundary (string): ARN of the managed policy, used to
    set the permissions boundary of the role.
        See AWS documentation for more detail.

    Returns:
    bool: True if creation is successful.
        False if a role of that Name already exists.
        False if the permissions boundary provided does not permit
        creating roles.
    """
    try:
        flags.OTA_ROLE_NAME = Name
        if flags.OTA_ROLE_NAME is None or flags.OTA_ROLE_NAME == "":
            logging.error("The role name is undefined.")
            return False
        if permissions_boundary is None:
            logging.warn(
                "About to create role "
                + flags.OTA_ROLE_NAME
                + " without a permissions boundary."
            )
            flags.role = iam.create_role(
                RoleName=flags.OTA_ROLE_NAME,
                AssumeRolePolicyDocument=json.dumps(flags.ASSUME_ROLE_POLICY_DOCUMENT),
            )["Role"]
        else:
            logging.info(
                "About to create role: '"
                + flags.OTA_ROLE_NAME
                + "', with boundary: "
                + permissions_boundary
            )
            flags.role = iam.create_role(
                RoleName=flags.OTA_ROLE_NAME,
                AssumeRolePolicyDocument=json.dumps(flags.ASSUME_ROLE_POLICY_DOCUMENT),
                PermissionsBoundary=permissions_boundary,
            )["Role"]
        logging.info("Role Created")
        iam.attach_role_policy(
            RoleName=flags.OTA_ROLE_NAME,
            PolicyArn=("arn:aws:iam::aws:policy/service-role/" "AWSIoTRuleActions"),
        )
        iam.attach_role_policy(
            RoleName=flags.OTA_ROLE_NAME,
            PolicyArn=(
                "arn:aws:iam::aws:policy/service-role/" "AmazonFreeRTOSOTAUpdate"
            ),
        )
        iam.attach_role_policy(
            RoleName=flags.OTA_ROLE_NAME,
            PolicyArn=("arn:aws:iam::aws:policy/service-role/" "AWSIoTLogging"),
        )
        iam.attach_role_policy(
            RoleName=flags.OTA_ROLE_NAME,
            PolicyArn=(
                "arn:aws:iam::aws:policy/service-role/" "AWSIoTThingsRegistration"
            ),
        )
        logging.info("Attached all policies to Role")
        iam.put_role_policy(
            RoleName=flags.OTA_ROLE_NAME,
            PolicyName="iam-permissions",
            PolicyDocument=json.dumps(flags.IAM_OTA_PERMISSION),
        )
        iam.put_role_policy(
            RoleName=flags.OTA_ROLE_NAME,
            PolicyName="s3-permissions",
            PolicyDocument=json.dumps(flags.S3_OTA_PERMISSION),
        )
        flags.OTA_ROLE_ARN = flags.role["Arn"]
        logging.info("Attached IAM and S3 Policy")
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "EntityAlreadyExists":
            logging.warning("The role named " + Name + " already exists")
            return False
        elif ex.response["Error"]["Code"] == "AccessDenied":
            correctBoundaryFormat = (
                "'arn:aws:iam::"
                + flags.AWS_ACCOUNT_ID
                + ":policy/<your_permission_boundary_policy>'"
            )
            if permissions_boundary is not None:
                logging.error(
                    "Unauthorized operation. Your permission boundary "
                    + permissions_boundary
                    + (
                        " might not be enough to create a role. Have you "
                        "checked the --permissions_boundary parameter: "
                        "1. is an ARN, 2. links to an existing boundary."
                    )
                )
                if ":policy/" not in permissions_boundary:
                    logging.warning(
                        (
                            "Your permissions boundary does not contain"
                            "':policy/'. The correct format is: "
                        )
                        + correctBoundaryFormat
                    )
            else:
                logging.error(
                    (
                        "Unauthorized operation. You might need to add "
                        "the permission boundary with --permissions_boundary"
                    )
                    + correctBoundaryFormat
                )
            return False
        else:
            logging.error("failed creating an iam role")
            print_exception(ex)
            cleanup_aws_resources(flags)
            raise ex
    except Exception as e:
        logging.error("failed creating an iam role")
        print_exception(e)
        cleanup_aws_resources(flags)
        raise e
    flags.roleHasBeenCreated = True
    return True


# Define command-line interface for Role creation command.
@cli.command(cls=StdCommand)
@click.option(
    "--iam_role_name",
    prompt="Enter IAM Role Name",
    help="Name of IAM Role to be created.",
)
@click.option(
    "--build_dir",
    help="Override the default build directory",
    default=DEFAULT_BUILD_DIR,
)
@click.option(
    "--ota_binary",
    help="Override the default OTA file used",
    default=DEFAULT_OTA_BINARY,
)
@click.option(
    "--permissions_boundary",
    help="restricted users might need to use a permission \
        boundary to create new iam roles. \
        E.g. 'arn:aws:iam:<account_no>:policy/<boundary_name>'",
    default=None,
)
@click.pass_context
def create_iam_role_only(
    ctx,
    iam_role_name,
    build_dir,
    ota_binary,
    permissions_boundary,
    log_level,
):
    ctx.flags = Flags(
        bucket_name="",
        role_name=iam_role_name,
        build_dir=build_dir,
        ota_binary=ota_binary,
    )
    if not _create_iam_role(ctx.flags, iam_role_name, permissions_boundary):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    ctx.exit(0)


def _wait_for_status(id, action, timeout=20, timeout_step=3):
    """
    Waits for the OTA job passed to stop executing, returning True
    if the job's status is COMPLETE after execution.

    Parameters:
    id (string): the OTA update id (as seen in AWS).
    action (string): name of the associated OTA action.
    timeout (int): how many seconds to wait for a status until timing out.
    timeout_step (int): checks for a status every 'timeout_step' seconds.

    Returns:
    bool: True if the action's final status is COMPLETE, otherwise False.
        False on timeout or an unexpected OTA status returned.
    """
    success_status = action.upper() + "_COMPLETE"
    failure_status = action.upper() + "_FAILED"
    status = "WAIT_FOR_STATUS_FAILED"
    for i in range(0, timeout, timeout_step):
        res = None
        try:
            res = iot.get_ota_update(otaUpdateId=id)
        except Exception:
            logging.warning("Could not get ota update status for id " + id)
            return False
        else:
            status = res["otaUpdateInfo"]["otaUpdateStatus"]
            if status == success_status:
                return True
            elif status == failure_status:
                return False
            logging.debug("OTA update status:" + status)
        time.sleep(timeout_step)
    # on timeout
    msg = (
        "Unexpected OTA update status returned, expected '"
        + success_status
        + "' or '"
        + failure_status
        + "' for action '"
        + action
        + "'. Instead received: '"
        + status
        + "'"
    )
    logging.warning(msg)
    return False


def _wait_for_job_deleted(job_name, timeout=20):
    """
    Parameters:
    job_name (string): name of the job waited on.
    timeout (int): seconds to wait for deletion before timing out.

    Returns:
    bool: True if the job does not exist.
        True if the job is deleted.
        False otherwise (e.g. timeout).
    """
    job_status = None
    step = 2
    for i in range(0, timeout, step):
        if not _does_job_exist(job_name):
            return True
        res = iot.describe_job(jobId=job_name)
        job_status = res["job"]["status"]
        if job_status == "DELETION_IN_PROGRESS" or job_status == "DELETE_IN_PROGRESS":
            # we need to wait for the deletion to finish before returning
            time.sleep(step)
            logging.info(
                "Job deletion in progress. Waiting another "
                + str(timeout - i)
                + " seconds for deletion."
            )
            continue
        else:
            return False
    # on timeout
    logging.debug(
        "wait_for_job_deleted timed out. The job " + job_name + "is still running"
    )
    return False


def _wait_for_ready_to_assume(role_arn, timeout=20):
    """
    Parameters:
    role_arn (string): the ARN uniquely identifying the role waited on.
    timeout (int): seconds to wait before timing out.

    Returns:
    bool: True if role is free to assume.
        False otherwise.
    """
    for i in range(0, timeout, 2):
        try:
            sts_client.assume_role(
                RoleArn=role_arn,
                RoleSessionName="check_assuming_role_permission",
            )
        except botocore.exceptions.ClientError as ex:
            if ex.response["Error"]["Code"] == "AccessDenied":
                # we likely just need to wait longer
                pass
            else:
                logging.warning(
                    "Failed to assume role: " + ex.response["Error"]["Message"]
                )
                return False
        except Exception as ex:
            logging.warning("Failed to assume role: " + str(ex))
            return False
        else:
            # no error reported? that mean we're free to assume the role
            return True
        time.sleep(2)
    # timing out
    return False


def _create_aws_update(flags: Flags, ota_name):
    """
    Create an AWS IoT firmware update.
    If creation fails, will delete all AWS entities created during
    this command.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
    (e.g. credentials).
    ota_name: name of the OTA to be created.

    Returns:
    bool: True if creation succeeds.
        False otherwise. E.g. returns False if another OTA is using
        the 'ota_name' provided.
    """
    # The role might exist, but not be usable yet. Leave some time for
    # it to propagate in AWS if needed
    if not _wait_for_ready_to_assume(flags.OTA_ROLE_ARN):
        logging.error("Failed to assume role to create an ota update")
        return False

    # check if there's a pending OTA job with the same name
    ota_job_name = OTA_JOB_NAME_PREFIX + ota_name
    # Wait for deletion to complete before creating a new OTA.
    if not _wait_for_job_deleted(ota_job_name):
        logging.error(
            "The job "
            + ota_job_name
            + " already exists. It needs to be deleted before trying to create \
                another ota job with the same name"
        )
        return False

    try:
        flags.THING_ARN = iot.describe_thing(thingName=flags.THING_NAME)["thingArn"]

        # Push firmware to test bucket
        flags.file = open(flags.OTA_BINARY_FILE_PATH, "rb")
        s3.put_object(
            Bucket=flags.OTA_BUCKET_NAME, Key=flags.OTA_BINARY, Body=flags.file
        )
        flags.file.close()
        logging.info(
            "Added " + flags.OTA_BINARY + " to S3 bucket " + flags.OTA_BUCKET_NAME
        )

        # Create update job
        iot.create_ota_update(
            otaUpdateId=ota_name,
            targets=[flags.THING_ARN],
            protocols=flags.OTA_UPDATE_PROTOCOLS,
            targetSelection=flags.OTA_UPDATE_TARGET_SELECTION,
            files=flags.OTA_UPDATE_FILES,
            roleArn=flags.OTA_ROLE_ARN,
        )

        if not _wait_for_status(ota_name, "create"):
            logging.error("failed creating aws update.")
            res = iot.get_ota_update(otaUpdateId=ota_name)
            if res["otaUpdateInfo"]["otaUpdateStatus"] == "CREATE_FAILED":
                logging.error(
                    "Update failed to be created: "
                    + res["otaUpdateInfo"]["errorInfo"]["message"]
                )
            else:
                logging.error(
                    "Update failed to be created or timed out \
                    during creation. Last status: "
                    + res["otaUpdateInfo"]["otaUpdateStatus"]
                )
            return False
        flags.UPDATE_ID = ota_name
        logging.info("Created update " + flags.UPDATE_ID)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceAlreadyExistsException":
            logging.warning("The ota update " + ota_name + " already exists")
            return True
        else:
            logging.error(
                "failed creating aws update: " + ex.response["Error"]["Message"]
            )
            return False
    except Exception as e:
        logging.error("failed creating aws update.")
        print_exception(e)
        cleanup_aws_resources(flags)
        raise e
    flags.updateHasBeenCreated = True
    return True


# Define command-line interface for OTA Update creation command.
@cli.command(cls=StdCommand)
@click.option(
    "--thing_name",
    prompt="Enter Thing Name",
    help="Name of the existing Thing to use when creating the update.",
)
@click.option(
    "--policy_name",
    prompt="Enter Policy Name",
    help="Name of the existing Policy to use when creating the update.",
)
@click.option(
    "--bucket_name",
    prompt="Enter Bucket Name",
    help="Name of the existing Bucket to use when creating the update.",
)
@click.option(
    "--iam_role_name",
    prompt="Enter IAM Role Name",
    help="Name of the existing IAM Role to use when creating the update.",
)
@click.option(
    "--update_name",
    prompt="Enter Update ID: ",
    help="Update ID to create.",
)
@click.option(
    "--build_dir",
    help="Override the default build directory",
    default=DEFAULT_BUILD_DIR,
)
@click.option(
    "--ota_binary",
    help="Override the default OTA file used",
    default=DEFAULT_OTA_BINARY,
)
@click.pass_context
def create_update_only(
    ctx,
    thing_name,
    policy_name,
    bucket_name,
    iam_role_name,
    update_name,
    build_dir,
    ota_binary,
    log_level,
):
    ctx.flags = Flags(
        bucket_name=bucket_name,
        role_name=iam_role_name,
        build_dir=build_dir,
        ota_binary=ota_binary,
        ota_update_signature_filename=DEFAULT_OTA_UPDATE_SIGNATURE_FILENAME,
    )
    signaturePath = os.path.join(
        ctx.flags.BUILD_DIR, DEFAULT_OTA_UPDATE_SIGNATURE_FILENAME
    )
    try:
        open(signaturePath, "r")
    except FileNotFoundError:
        logging.error(
            "The OTA update signature was not found at '"
            + signaturePath
            + "'. It is not possible to upload OTA updates."
        )
        ctx.exit(1)

    # Need previously created things and policy to setup an OTA update job
    ctx.flags.THING_NAME = thing_name
    if not _does_thing_exist(thing_name):
        logging.error("no thing found with the name " + thing_name)
        ctx.exit(1)

    ctx.flags.POLICY_NAME = policy_name
    if not _does_policy_exist(policy_name):
        logging.error("no policy found with the name " + policy_name)
        ctx.exit(1)

    ctx.flags.OTA_BUCKET_NAME = bucket_name
    if not _does_bucket_exist(bucket_name):
        logging.error("no bucket found with the name " + bucket_name)
        ctx.exit(1)

    if not _does_role_exist(iam_role_name):
        logging.error("no iam role found with the name " + iam_role_name)
        ctx.exit(1)

    if not _create_aws_update(ctx.flags, update_name):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    ctx.exit(0)


# Define command-line interface for create-thing-and-policy command.
@cli.command(cls=StdCommand)
@click.option(
    "--thing_name", prompt="Enter Thing Name", help="Name of Thing to be created."
)
@click.option(
    "--policy_name", prompt="Enter Policy Name", help="Name of Policy to be created."
)
@click.option(
    "--existing_certificate_arn",
    default=CREATE_NEW_CERTIFICATE,
    help="Use the provided certificate ARN instead of creating a new one",
)
@click.option(
    "-a",
    "--credentials_path",
    default=DEFAULT_CREDENTIALS_PATH,
    show_default=True,
    help="The path where the credentials are stored. \
        If not specified, Thing credentials are stored \
        in build/generated_credentials",
)
@click.option(
    "--build_dir",
    help="Override the default build directory",
    default=DEFAULT_BUILD_DIR,
)
@click.option(
    "--ota_binary",
    help="Override the default OTA file used",
    default=DEFAULT_OTA_BINARY,
)
@click.option(
    "--target_application",
    required=True,
    help="Updates target application's aws_clientcredential.h automatically. \
        Accepted values: "
    + reduce(
        lambda y, z: y + ", " + z,
        map(lambda app: "'" + app.value + "'", AWS_APPLICATIONS),
    ),
)
@click.pass_context
def create_thing_and_policy(
    ctx,
    thing_name,
    policy_name,
    existing_certificate_arn,
    credentials_path,
    build_dir,
    ota_binary,
    target_application,
    log_level,
):
    target = ApplicationType.app_type_from_string(target_application)
    if target_application == ApplicationType.UNDEFINED:
        logging.error(
            "The provided target application '"
            + target_application
            + "' is not passed using the CLI."
        )
        ctx.exit(1)
    ctx.flags = Flags(
        build_dir=build_dir,
        ota_binary=ota_binary,
        target_application=target,
    )

    ctx.flags.THING_NAME = thing_name
    certificate_arn = _get_credential_arn(
        ctx.flags, existing_certificate_arn, credentials_path
    )

    if not _create_thing(ctx.flags, thing_name, certificate_arn):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    if not _create_policy(ctx.flags, policy_name, certificate_arn):
        # Delete all AWS entities created by the script so far.
        # I.e. delete the created Thing and Thing certificate.
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    ctx.exit(0)


# Defines Command-line interface for creating bucket, role, and OTA update.
@cli.command(cls=StdCommand)
@click.option(
    "--thing_name", prompt="Enter existing Thing Name", help="Name of Thing to be used."
)
@click.option(
    "--bucket_name", prompt="Enter Bucket Name", help="Name of Bucket to be created."
)
@click.option(
    "--iam_role_name",
    prompt="Enter IAM Role Name",
    help="Name of IAM Role to be created.",
)
@click.option("--update_name", prompt="Enter Update ID", help="Update ID to create.")
@click.option(
    "--build_dir",
    help="Override the default build directory",
    default=DEFAULT_BUILD_DIR,
)
@click.option(
    "--ota_binary",
    help="Override the default OTA file used",
    default=DEFAULT_OTA_BINARY,
)
@click.option(
    "--permissions_boundary",
    help="restricted users might need to use a permission boundary to \
        create new iam roles. \
        E.g. 'arn:aws:iam:<account_no>:policy/<boundary_name>'",
    default=None,
)
@click.pass_context
def create_bucket_role_update(
    ctx,
    thing_name,
    bucket_name,
    iam_role_name,
    update_name,
    build_dir,
    ota_binary,
    permissions_boundary,
    log_level,
):
    ctx.flags = Flags(
        bucket_name=bucket_name,
        role_name=iam_role_name,
        build_dir=build_dir,
        ota_binary=ota_binary,
        ota_update_signature_filename=DEFAULT_OTA_UPDATE_SIGNATURE_FILENAME,
    )
    signaturePath = os.path.join(
        ctx.flags.BUILD_DIR, DEFAULT_OTA_UPDATE_SIGNATURE_FILENAME
    )
    try:
        open(signaturePath, "r")
    except FileNotFoundError:
        logging.error(
            "The OTA update signature was not found at '"
            + signaturePath
            + "'. It is not possible to upload OTA updates."
        )
        ctx.exit(1)
    if permissions_boundary is None:
        logging.error('Must define "--permissions_boundary" to create a role.')
        ctx.exit(1)

    # Need previously created thing to setup an OTA update job
    ctx.flags.THING_NAME = thing_name
    if not _does_thing_exist(thing_name):
        logging.error("no thing found with the name " + thing_name)
        ctx.exit(1)

    if not _create_aws_bucket(ctx.flags, bucket_name):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)

    if not _create_iam_role(ctx.flags, iam_role_name, permissions_boundary):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)

    if not _create_aws_update(ctx.flags, update_name):
        cleanup_aws_resources(ctx.flags)
        ctx.exit(1)
    ctx.exit(0)


def print_exception(ex):
    x = tb.extract_stack()[1]
    logging.error(f"{x.filename}:{x.lineno}:{x.name}: {ex}")


def _list_generic(
    client,
    listing_function_name: str,
    item_type_name: str,
    dictionary_item_name: str,
    max_listed=10,
):
    """
    This function abstracts away the implementation of listing AWS entities.

    Parameters:
    client: e.g. the boto3 AWS client connection.
    listing_function_name (string): the listing function used on AWS.
        E.g. 'list_things'.
    item_type_name (string): the type of AWS entity requested. E.g. 'things'.
    dictionary_item_name (string): the dictionary entry the objects to be
        listed fall under, in the pages returned by boto3.
    max_listed (int): the maximum number of items shown.

    Returns: None
    """
    # AWS' list_things returns 1000 results (a 'page') at a time.
    # The paginator abstracts the process of combining consecutive pages.
    paginator = client.get_paginator(listing_function_name)
    response_iterator = (
        paginator.paginate()
    )  # Returns a PageIterator. See library boto3.
    currently_listed = 0
    for page in response_iterator:
        if len(page.get(dictionary_item_name)) == 0 and currently_listed == 0:
            logging.info("No " + item_type_name + " found")
            return
        for item in page[dictionary_item_name]:
            if currently_listed >= max_listed:
                logging.info("List truncated for readability")
                return
            logging.info(item)
            currently_listed += 1
    logging.info("Listed " + str(currently_listed) + " " + item_type_name)


def _list_things(max_listed=float("inf")):
    """
    Lists all the Things accessible to the provided AWS account.
    """
    _list_generic(iot, "list_things", "things", "things", max_listed)


# Define command-line interface for Thing listing command.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed things",
    default=25,
)
@click.pass_context
def list_things(ctx, log_level, max_listed):
    _list_things(max_listed)
    ctx.exit(0)


def _list_policies(max_listed=float("inf")):
    """
    Lists all the Policies accessible to the provided AWS account.
    """
    _list_generic(iot, "list_policies", "policies", "policies", max_listed)


# Define command-line interface for Policy listing command.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed policies",
    default=25,
)
@click.pass_context
def list_policies(ctx, log_level, max_listed):
    _list_policies(max_listed)
    ctx.exit(0)


def _list_iam_roles(max_listed=float("inf")):
    """
    Lists all the Roles accessible to the provided AWS account.
    """
    _list_generic(iam, "list_roles", "roles", "Roles", max_listed)


# Define command-line interface for Role listing command.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed roles",
    default=25,
)
@click.pass_context
def list_iam_roles(ctx, log_level, max_listed):
    _list_iam_roles(max_listed)
    ctx.exit(0)


def _list_ota_updates(max_listed=float("inf")):
    """
    Lists all the OTA updates accessible to the provided AWS account.
    """
    _list_generic(iot, "list_ota_updates", "ota updates", "otaUpdates", max_listed)


# Define command-line interface for OTA Update listing command.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed ota updates",
    default=25,
)
@click.pass_context
def list_ota_updates(ctx, log_level, max_listed):
    _list_ota_updates(max_listed)
    ctx.exit(0)


def _list_jobs(max_listed=float("inf")):
    """
    Lists all the Jobs accessible to the provided AWS account.
    """
    _list_generic(iot, "list_jobs", "jobs", "jobs", max_listed)


# Define command-line interface for OTA Job listing command.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed jobs",
    default=25,
)
@click.pass_context
def list_jobs(ctx, log_level, max_listed):
    _list_jobs(max_listed)
    ctx.exit(0)


def _list_buckets(max_listed=float("inf")):
    """
    Lists all the Buckets accessible to the provided AWS account.
    """
    # list_buckets cannot be paginated, so we can't use _list_generic()
    response = s3.list_buckets()
    if len(response.get("Buckets")) == 0:
        logging.info("No buckets found")
        return
    currently_listed = 0
    for item in response["Buckets"]:
        if currently_listed >= max_listed:
            logging.info("List truncated for readability")
            return
        logging.info(item)
        currently_listed += 1
    logging.info("Listed " + str(currently_listed) + " buckets")


# Define command-line interface for Bucket listing command.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed buckets",
    default=25,
)
@click.pass_context
def list_buckets(ctx, log_level, max_listed):
    _list_buckets(max_listed)
    ctx.exit(0)


def _list_certificates(max_listed=float("inf")):
    """
    Lists all the Certificates accessible to the provided AWS account.
    """
    _list_generic(iot, "list_certificates", "certificates", "certificates", max_listed)


# Define command-line interface for Certificate listing command.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed certificates",
    default=25,
)
@click.pass_context
def list_certificates(ctx, log_level, max_listed):
    _list_certificates(max_listed)
    ctx.exit(0)


# Define command-line interface for listing all AWS entities.
@cli.command(cls=ListingCommand)
@click.option(
    "--max_listed",
    help="Will not print more than max_listed item for each category",
    default=15,
)
@click.pass_context
def list_all(ctx, log_level, max_listed):
    logging.info("Things:")
    _list_things(max_listed)
    logging.info("\n\nPolicies:")
    _list_policies(max_listed)
    logging.info("\n\nRoles:")
    _list_iam_roles(max_listed)
    logging.info("\n\nOTA Updates:")
    _list_ota_updates(max_listed)
    logging.info("\n\nJobs:")
    _list_jobs(max_listed)
    logging.info("\n\nBuckets:")
    _list_buckets(max_listed)
    logging.info("\n\nCertificates:")
    _list_certificates(max_listed)
    ctx.exit(0)


def _has_certificate_anything_attached(certificate_arn):
    """
    Returns True if the certificate located with the
    'certificate_arn' provided is attached to any Things
    or Policies.

    Parameters:
    certificate_arn (string): the unique identifier used to
        locate the certificate.

    Returns:
    bool: True if the certificate is attached to any Things or Policies.
        False otherwise.
    """
    pass
    response = iot.list_principal_things(principal=certificate_arn)
    if len(response["things"]):
        return True
    response = iot.list_attached_policies(target=certificate_arn)
    if len(response["policies"]):
        return True
    return False


def _get_certificate_id_from_arn(certificate_arn):
    """
    Parameters (string): the unique certificate identifier (ARN).

    Returns:
    string: the certificate id. I.e. the section after the last '/'.
    """
    return certificate_arn.split("/")[-1]


def _delete_thing(thing_name, prune=False):
    """
    Parameters:
    thing_name (string): the name of the IoT Thing to be deleted.
    prune (bool): if True, deletes any certificates that are only attached
        to the Thing.

    Returns:
    bool: False if the Thing does not exist.
        True otherwise.
    """
    if not _does_thing_exist(thing_name):
        logging.error("no thing found with the name " + thing_name)
        return False
    try:
        paginator = iot.get_paginator("list_thing_principals")
        response_iterator = paginator.paginate(thingName=thing_name)
        for page in response_iterator:
            if len(page.get("principals")) == 0:
                break
            for principal in page["principals"]:
                iot.detach_thing_principal(
                    thingName=thing_name,
                    principal=principal,
                )
                if prune and not _has_certificate_anything_attached(principal):
                    # if the certificate status is INACTIVE then calling
                    # _delete_certificate would do a reccursion loop
                    certificate_id = _get_certificate_id_from_arn(principal)
                    response = iot.describe_certificate(certificateId=certificate_id)
                    isActive = response["certificateDescription"]["status"] == "ACTIVE"
                    if isActive:
                        if _delete_certificate(certificate_id):
                            logging.info("Deleted certificate: " + principal)
        iot.delete_thing(thingName=thing_name)
    except Exception as ex:
        logging.error("failed deleting the thing named " + thing_name)
        print_exception(ex)
        raise ex
    return True


# Define command-line interface for Thing deletion command.
@cli.command(cls=StdCommand)
@click.option(
    "--thing_name",
    prompt="Enter thing name",
    help="Name of thing to be\
          deleted.",
)
@click.option(
    "-p",
    "--prune-certificate",
    "prune",
    help="If the deleted thing was the last item attached to a \
        certificate, delete the certificate as well.",
    is_flag=True,
)
@click.pass_context
def delete_thing(ctx, thing_name, log_level, prune):
    if _delete_thing(thing_name, prune):
        logging.info(thing_name + " deleted")
        ctx.exit(0)
    ctx.exit(1)


def _delete_policy(policy_name, prune=False):
    """
    Parameters:
    policy_name (string): the name of the IoT Policy to be deleted.
    prune (bool): if True, deletes from AWS any certificates that are
        only attached to the Policy.

    Returns:
    bool: False if the Policy does not exist.
        True otherwise (indicating successful deletion).
    """
    if not _does_policy_exist(policy_name):
        logging.error("no policy found with the name " + policy_name)
        return False
    try:
        paginator = iot.get_paginator("list_policy_principals")
        response_iterator = paginator.paginate(policyName=policy_name)
        for page in response_iterator:
            if page.get("principals") == []:
                break
            for principal in page["principals"]:
                iot.detach_principal_policy(
                    policyName=policy_name,
                    principal=principal,
                )
                if prune and not _has_certificate_anything_attached(principal):
                    # if the certificate status is INACTIVE then calling
                    # _delete_certificate would do a reccursion loop
                    certificate_id = _get_certificate_id_from_arn(principal)
                    response = iot.describe_certificate(certificateId=certificate_id)
                    isActive = response["certificateDescription"]["status"] == "ACTIVE"
                    if isActive:
                        if _delete_certificate(certificate_id):
                            logging.info("Deleted certificate: " + principal)
        iot.delete_policy(policyName=policy_name)
    except Exception as ex:
        logging.error("failed deleting the policy named " + policy_name)
        print_exception(ex)
        raise ex
    return True


# Define command-line interface for Policy deletion command.
@cli.command(cls=StdCommand)
@click.option(
    "--policy_name", prompt="Enter policy name", help="Name of policy to be deleted."
)
@click.option(
    "-p",
    "--prune-certificate",
    "prune",
    help="If the deleted policy was the last item attached to a certificate,\
          delete the certificate as well.",
    is_flag=True,
)
@click.pass_context
def delete_policy(ctx, policy_name, log_level, prune):
    if _delete_policy(policy_name, prune):
        logging.info(policy_name + " deleted")
        ctx.exit(0)
    ctx.exit(1)


def _delete_iam_role(iam_role_name, force_delete=False):
    """
    Parameters:
    iam_role_name (string): the name of the IAM Role to be deleted.
    force_delete (bool): if True, detaches all policies from the role
        before deleting it.
        If False, deletion may fail due to the role having attached policies.

    Returns:
    bool: False if the Role does not exist, has attached policies, or
        cannot be found.
        True otherwise (for success).
    """
    try:
        role = boto3.resource("iam").Role(iam_role_name)
        if force_delete:
            policy_iterator = role.attached_policies.all()
            for policy in policy_iterator:
                logging.debug("Detached " + policy.arn)
                policy.detach_role(RoleName=iam_role_name)

            role_policy_iterator = role.policies.all()
            for role_policy in role_policy_iterator:
                logging.debug("Detached " + role_policy.name)
                role_policy.delete()
        role.delete()
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "NoSuchEntity":
            logging.warning("no iam role found with the name " + iam_role_name)
            return False
        if ex.response["Error"]["Code"] == "AccessDenied":
            logging.warning("Access denied or non existant role: " + iam_role_name)
            return False
        elif ex.response["Error"]["Code"] == "DeleteConflict":
            logging.warning(
                (
                    "Role have attached policies. Try re-running with"
                    "the -f option to detach them first."
                )
            )
            return False
        else:
            logging.error("failed deleting the iam role named " + iam_role_name)
            print_exception(ex)
            raise ex
    except Exception as ex:
        logging.error("failed deleting the iam role named " + iam_role_name)
        print_exception(ex)
        raise ex
    return True


# Define command-line interface for Role deletion command.
@cli.command(cls=StdCommand)
@click.option(
    "--iam_role_name", prompt="Enter role name", help="Name of the role to be deleted."
)
@click.option(
    "-f",
    "--force-delete",
    "force_delete",
    help="Delete the AWS role irrespective of its current state",
    is_flag=True,
)
@click.pass_context
def delete_iam_role(ctx, iam_role_name, log_level, force_delete):
    if _delete_iam_role(iam_role_name, force_delete):
        logging.info(iam_role_name + " deleted")
        ctx.exit(0)
    ctx.exit(1)


def _delete_job(job_name, force_delete=False):
    """
    Parameters:
    job_name (string): the name of the IoT Job to be deleted.
    force_delete (bool): if True, will delete the Job even if it
        is IN_PROGRESS.
        Normally, it is only possible to delete COMPLETED or CANCELLED jobs.

    Returns:
    bool: True if deletion succeeds.
        False if the job does not exist, or is running and force_delete
        is False.
    """
    try:
        iot.delete_job(jobId=job_name, force=force_delete)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            logging.warning("no job found with the name " + job_name)
            return False
        else:
            logging.error("failed deleting the job named " + job_name)
            print_exception(ex)
            raise ex
    except Exception as ex:
        logging.error("failed deleting the job named " + job_name)
        print_exception(ex)
        raise ex
    return True


# Define command-line interface for Job deletion command.
@cli.command(cls=StdCommand)
@click.option(
    "--job_name", prompt="Enter role name", help="Name of the job to be deleted."
)
@click.option(
    "-f",
    "--force-delete",
    "force_delete",
    help=("Job is deleted alongside the S3 Bucket versions and objects associated"),
    is_flag=True,
)
@click.pass_context
def delete_job(ctx, job_name, log_level, force_delete):
    if _delete_job(job_name, force_delete):
        logging.info(job_name + " deleted")
        ctx.exit(0)
    ctx.exit(1)


def _delete_ota_update(ota_update_name, force_delete=False):
    """
    Parameters:
    ota_update_name (string): name of the OTA update to be deleted.
    force_delete: If True, delete the OTA update even if it is in progress.

    Returns:
    bool: True if deletion is successful.
        False otherwise.
    """
    ota_job_name = OTA_JOB_NAME_PREFIX + ota_update_name
    if not _does_job_exist(ota_job_name):
        return True
    job_status = None
    try:
        job_status = iot.describe_job(jobId=ota_job_name)["job"]["status"]
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            return False
        else:
            raise ex
    # Handle edge cases.
    if job_status == "IN_PROGRESS":
        if not force_delete:
            # we do have a job but don't force delete:
            # we won't be able to delete the ota yet
            logging.warning(
                "The ota update is currently in progress. \
                Try re-running with the -f option to delete it anyway."
            )
            return False
    # If a deletion is ongoing, give it time (until maximum timeout) to complete.
    elif job_status == "DELETION_IN_PROGRESS" and not _wait_for_job_deleted(
        ota_job_name
    ):
        logging.error(
            "Job deletion has been started but is taking an abnormal amount of time."
        )
        # Cannot delete the OTA update without the job being deleted first.
        return False

    # tries to delete the ota update
    try:
        if force_delete:
            if _does_job_exist(ota_job_name):
                _delete_job(ota_job_name, True)
                logging.info(
                    "Deleting the OTA jobs. \
                             This may take a minute..."
                )
                if not _wait_for_job_deleted(ota_job_name, 60):
                    logging.error(
                        "failed deleting the job update named " + ota_job_name
                    )
                    return False
            iot.delete_ota_update(
                otaUpdateId=ota_update_name, deleteStream=True, forceDeleteAWSJob=True
            )
        else:
            res = iot.get_ota_update(otaUpdateId=ota_update_name)
            status = res["otaUpdateInfo"]["otaUpdateStatus"]
            if status in ["DELETE_IN_PROGRESS"]:
                logging.warning(
                    (
                        "The ota update is already being deleted."
                        "Try re-running with the -f option to re-send deletion anyway."
                    )
                )
            elif status in ["CREATE_PENDING", "CREATE_IN_PROGRESS"]:
                logging.warning(
                    (
                        "The ota update is currently in progress."
                        "Try re-running with the -f option to delete it anyway."
                    )
                )
                return False
            elif status in ["CREATE_COMPLETE"]:
                iot.delete_ota_update(otaUpdateId=ota_update_name)
            elif status in ["CREATE_FAILED", "DELETE_FAILED"]:
                logging.warning(
                    'The ota update is in an error state "'
                    + status
                    + '". Try re-running with the -f \
                        option to delete it anyway.'
                )
                return False
            else:
                logging.warning(
                    'The ota update is in an unexpected state: "'
                    + status
                    + '". Try re-running with the -f \
                        option to delete it anyway.'
                )
                return False
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            logging.warning("no ota update found with the name " + ota_update_name)
            return False
        else:
            logging.error("failed deleting the ota update named " + ota_update_name)
            print_exception(ex)
            raise ex
    except Exception as ex:
        logging.error("failed deleting the ota update named " + ota_update_name)
        print_exception(ex)
        raise ex

    # check the deletion status
    timeout = 20
    timeout_step = 2
    for i in range(0, timeout, timeout_step):
        try:
            res = iot.get_ota_update(otaUpdateId=ota_update_name)
            status = res["otaUpdateInfo"]["otaUpdateStatus"]
        except botocore.exceptions.ClientError as ex:
            if ex.response["Error"]["Code"] == "ResourceNotFoundException":
                # no ota update found = sucessful deletion
                return True
            else:
                raise ex
        if status == "DELETE_IN_PROGRESS":
            time.sleep(timeout_step)
            continue
        else:
            message = res["otaUpdateInfo"].get("errorInfo")["message"]
            logging.error(
                "failed deleting the ota update named "
                + ota_update_name
                + "\n    Status: "
                + status
                + ", "
                + message
            )
            return False
    # on timeout
    return False


# Define command-line interface for OTA update deletion command.
@cli.command(cls=StdCommand)
@click.option(
    "--ota_update_name",
    prompt="Enter ota update name",
    help="Name of the ota update to be deleted.",
)
@click.option(
    "-f",
    "--force-delete",
    "force_delete",
    help=(
        "OTA update is deleted alongside the S3 Bucket versions and objects"
        "associated"
    ),
    is_flag=True,
)
@click.pass_context
def delete_ota_update(ctx, ota_update_name, log_level, force_delete):
    if _delete_ota_update(ota_update_name, force_delete):
        logging.info(ota_update_name + " deleted")
        ctx.exit(0)
    ctx.exit(1)


def _delete_bucket(bucket_name, force_delete=False):
    """
    Parameters:
    bucket_name (string): name of the S3 Bucket to be deleted.
    force_delete (bool): If True, deletes bucket versions and objects
        before deleting the bucket.

    Returns:
    bool: True if deletion succeeds.
        False if the bucket is not empty and force_delete is not enabled.
        False if the bucket does not exist.
    """
    try:
        if force_delete:
            versioning = s3.get_bucket_versioning(Bucket=bucket_name)
            bucket = boto3.resource("s3").Bucket(bucket_name)
            if versioning.get("Status") == "Enabled":
                logging.debug("Removing bucket versioning")
                bucket.object_versions.delete()
            bucket.objects.delete()
        s3.delete_bucket(Bucket=bucket_name)
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "NoSuchBucket":
            logging.warning("no bucket found with the name " + bucket_name)
            return False
        elif ex.response["Error"]["Code"] == "BucketNotEmpty":
            logging.warning(
                "The bucket is not empty. Try re-running with the -f \
                    option to clean it first."
            )
            return False
        else:
            logging.error("failed deleting the bucket named " + bucket_name)
            print_exception(ex)
            raise ex
    except Exception as ex:
        logging.error("failed deleting the bucket named " + bucket_name)
        print_exception(ex)
        raise ex
    return True


# Define command-line interface for Bucket deletion command.
@cli.command(cls=StdCommand)
@click.option(
    "--bucket_name",
    prompt="Enter bucket name",
    help="Name of the bucket to be deleted.",
)
@click.option(
    "-f",
    "--force-delete",
    "force_delete",
    help="Delete the S3 Bucket versions and objects",
    is_flag=True,
)
@click.pass_context
def delete_bucket(ctx, bucket_name, log_level, force_delete):
    if _delete_bucket(bucket_name, force_delete):
        logging.info(bucket_name + " deleted")
        ctx.exit(0)
    ctx.exit(1)


def _delete_certificate(certificate_id, force_delete=False):
    """
    Parameters:
    certificate_id (string): the ID of the certificate.
        See '_get_certificate_id_from_arn'.
    force_delete (bool): If True, deletes all Things and Policies
        using the certificate.

    Returns:
    bool: True if deletion succeeds.
        False if the certificate is attached to a Policy or Thing
        and force_delete is False.
        False if the certificate does not exist."""
    try:
        iot.update_certificate(
            certificateId=certificate_id,
            newStatus="INACTIVE",
        )

        # Try to delete any attached things and policies
        if force_delete:
            certificateArn = iot.describe_certificate(certificateId=certificate_id)[
                "certificateDescription"
            ]["certificateArn"]
            # find and delete the attached policies
            paginator = iot.get_paginator("list_attached_policies")
            response_iterator = paginator.paginate(target=certificateArn)
            for page in response_iterator:
                for policy in page["policies"]:
                    _delete_policy(policy["policyName"])
            # find and delete the attached things
            paginator = iot.get_paginator("list_principal_things")
            response_iterator = paginator.paginate(principal=certificateArn)
            for page in response_iterator:
                for thing_name in page["things"]:
                    _delete_thing(thing_name)

        # Try to delete the certificate
        iot.delete_certificate(
            certificateId=certificate_id,
            forceDelete=force_delete,
        )
    except botocore.exceptions.ClientError as ex:
        if ex.response["Error"]["Code"] == "ResourceNotFoundException":
            logging.warning("no certificate found with the name " + certificate_id)
            return False
        elif ex.response["Error"]["Code"] == "DeleteConflictException":
            logging.warning(
                (
                    "The certificate still have things or policies attached."
                    "Try re-running with the -f option "
                    "to force detach them first"
                )
            )
            return False
        else:
            logging.error("failed deleting the certificate named " + certificate_id)
            print_exception(ex)
            raise ex
    except Exception as ex:
        logging.error("failed deleting the certificate named " + certificate_id)
        print_exception(ex)
        raise ex
    return True


# Define command-line interface for Certificate deletion command.
@cli.command(cls=StdCommand)
@click.option(
    "--certificate_id",
    prompt="Enter certificate id",
    help="Id of the certificate to be deleted.",
)
@click.option(
    "-f",
    "--force-delete",
    "force_delete",
    help="Delete the certificate even if it has policies or things attached. \
        Those will be deleted as well.",
    is_flag=True,
)
@click.pass_context
def delete_certificate(ctx, certificate_id, log_level, force_delete):
    if _delete_certificate(certificate_id, force_delete):
        logging.info(certificate_id + " deleted")
        ctx.exit(0)
    ctx.exit(1)


def cleanup_aws_resources(
    flags: Flags,
):
    """
    This function is usually called if a Command fails half-way.
    This function deletes all AWS entities created so far this session.
    E.g. all newly created Things, Policies, Roles, and Updates are removed.

    Parameters:
    flags (Flags): contains metadata needed for AWS and OTA updates
        (e.g. credentials).
    """
    logging.info(
        "Command failure detected. \
                 Deleting Things, Policies, Roles, and Updates created so far."
    )
    if flags.thing and flags.thingHasBeenCreated:
        try:
            has_deleted = _delete_thing(flags.THING_NAME)
        except Exception as ex:
            print_exception(ex)
        else:
            if has_deleted:
                logging.info("Deleted thing: " + flags.THING_NAME)
                flags.thing = None
            else:
                logging.warning("Failed to delete thing: " + flags.THING_NAME)
    if flags.policy and flags.policyHasBeenCreated:
        try:
            has_deleted = _delete_policy(flags.POLICY_NAME)
        except Exception as ex:
            print_exception(ex)
        else:
            if has_deleted:
                logging.info("Deleted policy: " + flags.POLICY_NAME)
                flags.policy = None
            else:
                logging.warning("Failed to delete policy: " + flags.POLICY_NAME)
    if flags.UPDATE_ID and flags.updateHasBeenCreated:
        try:
            has_deleted = _delete_ota_update(flags.UPDATE_ID, True)
        except Exception as ex:
            print_exception(ex)
        else:
            if has_deleted:
                logging.info("Deleted update" + flags.UPDATE_ID)
                flags.UPDATE_ID = None
            else:
                logging.warning("Failed to delete update: " + flags.UPDATE_ID)
    if flags.keyAndCertificate and flags.certificateHasBeenCreated:
        try:
            has_deleted = _delete_certificate(
                flags.keyAndCertificate["certificateId"], True
            )
        except Exception as ex:
            print_exception(ex)
        else:
            if has_deleted:
                logging.info(
                    "Deleted certificate: " + flags.keyAndCertificate["certificateArn"]
                )
                flags.keyAndCertificate = None
            else:
                logging.warning(
                    "Failed to delete certificate: "
                    + flags.keyAndCertificate["certificateArn"]
                )
    if flags.bucket and flags.bucketHasBeenCreated:
        try:
            has_deleted = _delete_bucket(flags.OTA_BUCKET_NAME, True)
        except Exception as ex:
            print_exception(ex)
        else:
            if has_deleted:
                logging.info("Deleted S3 bucket: " + flags.OTA_BUCKET_NAME)
                flags.bucket = None
            else:
                logging.warning("Failed to delete S3 bucket: " + flags.OTA_BUCKET_NAME)
    if flags.role and flags.roleHasBeenCreated:
        # We should not delete the role if the ota update have
        # not been sucessfully deleted, or we'll lose our only
        # way to delete the update
        if flags.UPDATE_ID is None:
            try:
                has_deleted = _delete_iam_role(flags.OTA_ROLE_NAME, True)
            except Exception as ex:
                print_exception(ex)
            else:
                if has_deleted:
                    logging.info("Deleted iam role: " + flags.OTA_ROLE_NAME)
                    flags.bucket = None
                else:
                    logging.warning("Failed to delete iam role: " + flags.OTA_ROLE_NAME)
        else:
            logging.warning(
                "Abort deleting the role because an OTA update \
                still depends on it: "
                + flags.OTA_ROLE_NAME
            )


if __name__ == "__main__":
    cli()
