#! /usr/bin/env python3
#
# Copyright 2023 Arm Limited and/or its affiliates
# <open-source-office@arm.com>
# SPDX-License-Identifier: MIT

import argparse

CREDENTIALS_TEMPLATE = """#ifndef AWS_CLIENT_CREDENTIAL_KEYS_H
#define AWS_CLIENT_CREDENTIAL_KEYS_H

#define keyCLIENT_CERTIFICATE_PEM {client_certificate_pem}

#define keyCLIENT_PRIVATE_KEY_PEM {client_private_key_pem}

#define keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM  {jitr_dev_cert_auth_pem}


#endif /* AWS_CLIENT_CREDENTIAL_KEYS_H */
"""


def main(args):
    client_private_key_pem = format_pem_to_c(args.client_private_key_pem)
    client_certificate_pem = format_pem_to_c(args.client_certificate_pem)
    with open("aws_clientcredential_keys.h", "w") as out_file:
        out_file.write(
            CREDENTIALS_TEMPLATE.format(
                client_certificate_pem=client_certificate_pem,
                client_private_key_pem=client_private_key_pem,
                jitr_dev_cert_auth_pem='''""''',
            )
        )


def format_pem_to_c(file):
    formatted = ""
    with open(file, "r") as f:
        lines = f.readlines()
        for line in lines[0:-1]:
            formatted += '"' + line.strip() + '\\n"\\\n'
        formatted += '"' + lines[-1].strip() + '"'
    return formatted


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--client_certificate_pem",
        help="the path of client certificate with pem extension",
        required=True,
    )
    parser.add_argument(
        "--client_private_key_pem",
        help="the path of client private key with pem extension",
        required=True,
    )
    main(parser.parse_args())
