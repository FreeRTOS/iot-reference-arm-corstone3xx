#! /usr/bin/env python3
#
# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import os
import click

CREDENTIALS_TEMPLATE = """#ifndef AWS_CLIENT_CREDENTIAL_KEYS_H
#define AWS_CLIENT_CREDENTIAL_KEYS_H
#define keyCLIENT_CERTIFICATE_PEM {client_certificate_pem}
#define keyCLIENT_PRIVATE_KEY_PEM {client_private_key_pem}
#define keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM  {jitr_dev_cert_auth_pem}
#define keyCODE_SIGNING_PUBLIC_KEY_PEM {code_signing_public_key_pem}

#endif /* AWS_CLIENT_CREDENTIAL_KEYS_H */
"""


def create_aws_clientcredential_keys_header(
    path_to_credential_directory: str,
    path_to_client_certificate_pem: str,
    path_to_client_private_key_pem: str,
    path_to_code_signing_public_key_pem: str,
) -> None:
    """
    Create an AWS client credential keys header file.
    Args:
        path_to_credential_directory (str): Path to the credential directory.
        path_to_client_certificate_pem (str): Path to the client certificate PEM file.
        path_to_client_private_key_pem (str): Path to the client private key PEM file.
        path_to_code_signing_public_key_pem (str): Path to the code signing public
                                                   key with pem extension.
    Returns:
        None
    """
    client_private_key_pem = process_file(path_to_client_private_key_pem)
    client_certificate_pem = process_file(path_to_client_certificate_pem)
    code_signing_public_key_pem = process_file(path_to_code_signing_public_key_pem)
    path_to_credential_key_header = os.path.join(
        path_to_credential_directory, "aws_clientcredential_keys.h"
    )

    with open(path_to_credential_key_header, "w") as out_file:
        out_file.write(
            CREDENTIALS_TEMPLATE.format(
                client_certificate_pem=client_certificate_pem,
                client_private_key_pem=client_private_key_pem,
                code_signing_public_key_pem=code_signing_public_key_pem,
                jitr_dev_cert_auth_pem='''""''',
            )
        )


def process_file(input_file: str) -> str:
    """
    Process a file and return its content as a formatted string.
    Args:
        input_file (str): Path to the input file.
    Returns:
        str: Processed content as a formatted string.
    """
    processed_content = ""
    try:
        with open(input_file, "r") as in_file:
            lines = in_file.readlines()

            # Add '"' and '\n"\' and then '\n'
            for line in lines[0:-1]:
                processed_content += '"' + line.strip() + '\\n"\\\n'

            # For the last line, add '"' and '"'
            processed_content += '"' + lines[-1].strip() + '"'
        return processed_content
    except FileNotFoundError:
        print(f"File '{input_file}' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")


@click.command()
@click.argument("path_to_credential_directory", type=str)
@click.option(
    "--path-to-client-certificate-pem",
    type=str,
    help="Path of client certificate with *.pem extension",
)
@click.option(
    "--path-to-client-private-key-pem",
    type=str,
    help="Path of client private key with *.pem extension",
)
@click.option(
    "--path-to-code-signing-public-key-pem",
    type=str,
    help="Path of code signing public key with pem extension",
)
def main(
    path_to_credential_directory: str,
    path_to_client_certificate_pem: str,
    path_to_client_private_key_pem: str,
    path_to_code_signing_public_key_pem: str,
) -> None:
    """
    Main CLI entry point for creating AWS client credential keys header.
    Args:
        path_to_credential_directory (str): Path to the credential directory.
        path_to_client_certificate_pem (str): Path to the client certificate PEM file.
        path_to_client_private_key_pem (str): Path to the client private key PEM file.
        path_to_code_signing_public_key_pem (str): Path to the code signing public key
                                                   with pem extension.
    Returns:
        None
    """
    create_aws_clientcredential_keys_header(
        path_to_credential_directory,
        path_to_client_certificate_pem,
        path_to_client_private_key_pem,
        path_to_code_signing_public_key_pem,
    )


if __name__ == "__main__":
    main()
