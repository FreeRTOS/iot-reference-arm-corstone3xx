# Creating an AWS IOT setup with automated scripts

An automation script is available to make setting up an OTA update easier. It is located at `tools/scripts/createIoTThings.py`.
The below will document how to set up AWS IOT for OTA updates using this script.

### AWS IOT basic concepts

In order to communicate through MQTT and/or perform an OTA update, you need to understand a few AWS concepts:

[Things](https://docs.aws.amazon.com/iot/latest/developerguide/iot-thing-management.html) are endpoints that typically represent IOT devices on the AWS side. You'll bind them to your actual device by using a certificate and RSA key.

[Policies](https://docs.aws.amazon.com/iot/latest/developerguide/thing-registry.html#attach-thing-principal) are sets of permissions you can attach to AWS entities. Typically to things and roles/users.

Those two concepts are enough to set up and use MQTT, but we need a few more if we want to perform an OTA update:

[S3 Buckets](https://docs.aws.amazon.com/AmazonS3/latest/API/API_Bucket.html) are some cloud storage. In order to run an OTA campaign, we need to store the binary somewhere, and it'll be in a bucket.

[Roles](https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles.html) are a set of permissions related to a task. As a user might not have the permission directly to perform an update, but they may be allowed to assume a role that does have that permission, thus temporarily gaining access to those permissions. To perform an OTA update you'll need to specify a role that will perform the update.

  > Note: Corporate account will likely have limitations on what roles can do. This is represented by a "permission boundary". Failure to set up this permission boundary when you create the role will prevent you from assuming that role, therefore prevent you from running an OTA update campaign. There may be additional restrictions than only a permission boundary. For instance, the role name may need to have a specific prefix. Please contact your dev ops to learn the limitation you may have regarding creating and assuming roles.

#### Prerequisites

To manually do the tasks `tools/scripts/createIoTThings.py` automates, also refer to [Setting up AWS connectivity](../../applications/aws_iot/setting_up_aws_connectivity.md) and [AWS IoT cloud connection](../../applications/aws_iot/aws_iot_cloud_connection.md).

Install python dependencies. first setting up a virtual environment:
```sh
python -m venv .venv
. .venv/bin/activate
```
Then, install the dependencies with:
```sh
python -m pip install boto3 click
```
Configure AWS credentials environment variables on command line, for example:
```sh
export AWS_ACCESS_KEY_ID=<access_key_id>
export AWS_SECRET_ACCESS_KEY=<secret_access_key>
export AWS_SESSION_TOKEN=<session_token>
export AWS_REGION=eu-west-1
```
The above keys and tokens can be found on the [AWS access portal](https://docs.aws.amazon.com/signin/latest/userguide/sign-in-urls-defined.html#access-portal-url). They are also called AWS API keys. They need to be <em>reset occasionally</em> as they expire. See [Troubleshooting](#troubleshooting) for more information.

In order to do an OTA update, you should have built an application that uses AWS, e.g. Keyword Detection. This is because the script assumes that `build/update-signature.txt` file exists when doing an OTA update.
Alternatively, you can provide the directory holding signatures and credentials as a command line argument (see `--build_dir`). Script-generated credentials will be written here.
It is still possible to create Things, Policies, and so on without a previous build existing.

#### Using the commands

Running the script with no parameter will give you the list of commands available. Additionally, every command have an `--help` option to list the accepted options.
Overall, there's three types of commands : `create-*`, `list-*` and `delete-*`.
Some commands can work on several objects, like `create-bucket-role-update` that create a bucket, a role, and an ota update in one go; or like `delete-thing -p` that delete a thing and, if that thing is the only object attached to a certificate, will delete the certificate as well.
Most commands will prompt you to enter names (for creating or deleting the object with that name). You may also enter the names directly in the commandline with options like `--thing_name`.
See `--help` for each command. For example:
```sh
python tools/scripts/createIoTThings.py --help
python tools/scripts/createIoTThings.py create-thing-and-policy --help
```
For examples of how to use each command, see [Creating AWS IoT Firmware update](#creating-aws-iot-firmware-update-jobs-using-the-automated-script).

### Create an AWS IOT setup manually

See the `docs/applications/aws_iot/setting_up_aws_connectivity.md` file for instructions on setting up IoT manually.

#### Limitations

These limitations apply for [creating AWS IoT firmware update jobs using the automated script](#creating-aws-iot-firmware-update-jobs-using-the-automated-script).
If you are re-using a role when making an OTA update, you must be able to assume the role. Most of the time, this means the credentials (e.g. `AWS_ACCESS_KEY_ID`) you are using must be associated with the account that created the role. The script will throw an error if this is not the case.

## Creating AWS IoT firmware update jobs using the automated script

Please refer to [AWS IOT basic concepts](#aws-iot-basic-concepts) and [Using the commands](#using-the-commands) for the general instructions and limitations about using the script.

Performing an OTA update will require you to:
  * Create a role with the required permissions
  * Create a bucket to store the update's binaries. Upload the update's binary.
  * Create an AWS thing. Tie the thing to your device by updating the credentials in the source code.
  * Create a policy that allows to connect to MQTT and attach it to the thing.
  * Finally, create and run the OTA update campaign

It is possible to run all subsequent commands with without any arguments, instead storing parameters in a `.json` file. See `--config_file_path` and [the overview of the .json config file](#overview-of-the-json-config-file).

Create a thing, an IOT policy, and attach the two together with:
```sh
python tools/scripts/createIoTThings.py create-thing-and-policy --thing_name <your_thing_name> --policy_name <your_policy_name> --target_application <target_application_name>
```
Where `<target_application_name>` is one of `keyword-detection`, `object-detection`, `speech-recognition`.
You must specify the `--target_application` argument if creating a Thing.
The script will update your `aws_clientcredential.h` config file automatically. If you have already modified the entries `clientcredentialMQTT_BROKER_ENDPOINT` or `clientcredentialIOT_THING_NAME` in the file, the script will warn you and ask before overwriting.
  > Note: You may also create each things and policies individually, but you'll have to make sure to pass the certificate created by the first command to the second. Certificates will be printed upon creation during the first command. Use `--use_existing_certificate_arn <your_certificate>` on the second command.

This creates and updates the `certificates` directory with the certificate of the newly created thing.
<b>It is the user's responsibility to clean up the `certificates` directory.</b>

You may now use MQTT to send and receive message for that device. See section [Observing MQTT connectivity](../../applications/aws_iot/aws_iot_cloud_connection.md#observing-mqtt-connectivity)

You may now rebuild keyword with those certificates:
```sh
./tools/scripts/build.sh keyword-detection --certificate_path certificates/thing_certificate_<your_thing_name>.pem.crt --private_key_path certificates/thing_private_key_<your_thing_name>.pem.key --target <corstone300/corstone310/corstone315> --inference <ETHOS/SOFTWARE> --audio <ROM/VSI> --toolchain <ARMCLANG/GNU> --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```
Next, we'll create the bucket, upload the binary there, create a role capable of running an OTA update, and create the update. All of those with the following command:
```sh
python tools/scripts/createIoTThings.py create-bucket-role-update --thing_name <your_thing_name> --bucket_name <your_bucket_name> --iam_role_name <prefix>-<your_role_name> --update_name <your_update_name> --ota_binary keyword-detection-update_signed.bin --permissions_boundary arn:aws:iam::<your_aws_account_id>:<your_company\'s_permission_boundary>
```
The above assumes you have `keyword-detection-update_signed.bin` saved in the directory specified by the `--ota_binary_build_dir` argument. This argument defaults to the project's root directory.
If you want to run an OTA update for another example application, use another signed binary name.
Buckets have a few rules about their name. You must not use capital case and the name must be unique. Failure to do so will trigger an `InvalidBucketName` or `AccessDenied` error.
The `<prefix>` you use is often defined by your company's AWS policies. For example, Arm uses the prefix `Proj`. Using the wrong prefix can result in failure to create roles.

And you can now run keyword and see the OTA update happening.
```sh
./tools/scripts/run.sh keyword-detection --target <corstone300/corstone310/corstone315> --audio <ROM/VSI>
```

You can clean up everything created here with:
```sh
python tools/scripts/createIoTThings.py delete-ota-update --ota_update_name <your_update_name> -f &&
python tools/scripts/createIoTThings.py delete-iam-role --iam_role_name <prefix>-<your_role_name> -f
python tools/scripts/createIoTThings.py delete-bucket --bucket_name <your_bucket_name> -f
python tools/scripts/createIoTThings.py delete-policy -p --policy_name <your_policy_name>
python tools/scripts/createIoTThings.py delete-thing -p --thing_name <your_thing_name>
```
- If the `-f` flag is set when deleting an OTA Update, the Update is deleted even if it has not yet finished.
- If the `-f` flag is set when deleting a Role, the Role is deleted even if it is attached to other AWS entities.
- If the `-f` flag is set when deleting a Bucket, the Bucket is deleted even if it is not empty.
- If the `-p` flag is set when deleting a Policy, any certificates attached to the Policy are pruned. Pruning means that if a certificate is attached to the policy and nothing else, this certificate is deleted.
- If the `-p` flag is set when deleting a Thing, certificates attached to the Thing are pruned.

  > Note: Your role is what allow you to interact with the OTA update. It is important you don't delete it before successfully deleting the OTA update or you will lose the permission to delete the update. If such a thing happen, you'll have to recreate the role and manually delete the update.

## Creating AWS IoT firmware update job (simplified)

The `create-update-simplified` command (1) creates a Thing and Policy, (2) runs build, (3) creates a bucket, role, and update.
This command also re-uses AWS entities where possible, validating entities being re-used.


To use this command:
1. Fill the following fields in the `.json` config file:
    * `thing_name` with the name of your AWS Thing.
    * `permissions_boundary` with the name of your Role's permission boundary - if applicable.
    For more information, see `python tools/scripts/createIoTThings.py create-iam-role-only --help` or [AWS IOT basic concepts](#aws-iot-basic-concepts).
    Creating a role without a permissions boundary is not supported by this script.
    * `role_prefix` with the prefix for your role. This prefix will be pre-pended to your role name with a hyphen by default. For example, with the prefix `Proj` and role name `role`, the completed role name will become `Proj-role`.
    * `target_application` with one of `keyword-detection`, `object-detection`, or `speech-recognition`. Alternatively, you can override this with a command-line argument (see `--help`).
2. Set up following [prerequisites](#prerequisites).
3. Run the command below.

```sh
python tools/scripts/createIoTThings.py create-update-simplified
```
You may optionally specify a different `.json` config file. See `--help`.

That's it. You can now run `run.sh` and wait for OTA update to complete.

If you have changed an example application and want to re-create a new OTA update, simply repeat step 3.
To create a new OTA update but keep the old one, modify the `update_name` field in the `.json` file.


### Overview of the .json config file

```json
{
    # There are only 4 required fields. These need to be filled in step (1) of using this command.
    "thing_name":"<your_thing_name>",
    "permissions_boundary":"arn:aws:iam::<account_no>:policy/<boundary_name>",
    "role_prefix":"<prefix>",
    "target_application":"",

    # Everything past this point need not be filled in

    # USED TO REPLACE e.g. '${thing_name}' with the field's value
    "format_vars":"thing_name;role_prefix;target_application;credentials_path",

    "policy_name":"",
    "bucket_name":"${thing_name}{lower}-bucket",
    "iam_role_name":"",
    "update_name":"",
    "existing_certificate_arn":"CREATE_NEW_CERTIFICATE",

    # Where to find certificates and binaries
    "credentials_path":"certificates",
    "build_dir":"build",
    "ota_binary":"${target_application}-update_signed.bin",

    # Build.sh script inputs are below
    "skip_build":"",
    "build_script_path":"",
    "private_key_path":"",
    "certificate_path":"",
    "target_platform":"",
    "inference":"",
    "audio":"",
    "toolchain":"",
    "clean_build":"",

    # Used by list-* and delete-* commands in the script
    "max_listed": "",
    "force_delete": "",

    # Default values are below.
    "policy_name_DEFAULT":"${thing_name}_policy",
    "bucket_name_DEFAULT":"${thing_name}_bucket",
    "iam_role_name_DEFAULT":"${role_prefix}-${thing_name}_role",
    "update_name_DEFAULT":"${thing_name}_update",
    "existing_certificate_arn_DEFAULT":"",
    "credentials_path_DEFAULT":"",
    "build_dir_DEFAULT":"",
    "ota_binary_DEFAULT":"",
    "skip_build_DEFAULT":"false",
    "build_script_path_DEFAULT":"./tools/scripts/build.sh",
    "private_key_path_DEFAULT":"${credentials_path}/thing_private_key_${thing_name}.pem.key",
    "certificate_path_DEFAULT":"${credentials_path}/thing_certificate_${thing_name}.pem.crt",
    "target_platform_DEFAULT":"corstone315",
    "inference_DEFAULT":"SOFTWARE",
    "audio_DEFAULT":"ROM",
    "toolchain_DEFAULT":"GNU",
    "clean_build_DEFAULT":"auto",
    "max_listed_DEFAULT":"25",
    "force_delete_DEFAULT":"false"
}
```
Note the \# comments are not valid JSON syntax and are purely included for this documentation.

Most settings in the config file are identical to those passed by the command-line to either `createIoTThings.py` or `build.sh`.
If a setting is left empty (`""`), then the script will look at the definition in `{FIELD_NAME}_DEFAULT`. E.g. if `policy_name:""`, then the script will use `"${thing_name}_policy"` as the policy name.

Some of the less obvious settings include:
- `format_vars` lists settings and variables that are used in the definitions of other settings. For more information, see [how the formatter works](#how-the-formatter-works-for-the-config-file).
- `skip_build`: if `true`, will cause `build.sh` to not run. This takes precedence over `clean_build`.
- `clean_build`: if `auto`, will run the `build.sh` script for a clean build (with the `-c` flag) only when necessary. I.e. if `aws_clientcredential` is updated by the script. Otherwise, the script runs `build.sh` not from clean. If `true`, always run `build.sh` for a clean build.
- `existing_certificate_arn` should be set to either a valid ARN for a certificate, or if you want the script to generate certificates for you, should be set to `CREATE_NEW_CERTIFICATE`.
- `target_application` can be specified in the `.json` file, and <b>if not otherwise specified on the command line</b> this value will be taken as a default by the script.
- `max_listed` is the number of items listed by each `list-*`  command, and for `list-all` the number listed by each category of AWS entity.
- `force_delete`: if `true` then all `delete-*` commands with a `force-delete` parameter will force-delete, otherwise (assuming the force delete flag is not specified on command line), these deletion commands will not force-delete. For what the term `force-delete` means, see each command's `--help` description.

Changing the `_DEFAULT` setting values is not recommended. Try to <b>change the user settings instead of the default settings</b>.
Other commands in the Python file (such as `create-policy-only`) will <b>not</b> adhere to changes made to this settings file.

Another useful tip is that the script will <b>not empty any buckets</b>. It is the responsibility of the user to periodically empty a bucket being re-used, if the user wants to avoid storing too much data in said bucket.

#### How the formatter works for the config file

The script defines some default values in terms of user-specified variables. For example, `policy_name_DEFAULT` is specified as `${thing_name}_policy`. To do this, some formatting is supported.
For these examples, assume that `"thing_name":"myTestThing"`.
Because `thing_name` is in `format_vars`, the following formatting happens (in order of priority):

1. `${thing_name}{lower}` maps to the value of `mytestthing`.
2. `${thing_name}{replace my with some}` maps to `someTestThing`. I.e. the value of `testThing` where occurrences of `my` are replaced by `some`. This is case-sensitive.
3. `${thing_name}` maps to `myTestThing`.

These are the only 3 types of formatting supported. Even `${thing_name}{lower}{replace A with B}` and similar chaining is not supported. Additionally, trying to use `}` or `{` in setting definitions may break the formatter.

If you want to add a setting, for example `update_name` to the definitions you can use in formatting, then append `;update_name` to the end of `format_vars`. This makes it possible to use `${update_name}` in the definitions of other settings.

The `target_application` setting is special because it is not defined in the `json` file but can still be mentioned in definitions.

## Cleaning up after AWS IoT firmware update job (simplified)

The `cleanup-simplified` command uses the config file from `create-update-simplified` and deletes all AWS entities described there.
Optionally, this command will check all credential files (such as certificates) to identify other Things created by the script.
These Things are deleted with their certificates.
The script identifies possibly linked AWS entities by using the .json config file to generate entity names. E.g. if the certificates for 'myTestThing' are found, and you have specified that 'policy_name' is '${thing_name}_policy', then the script will attempt to delete 'myTestThing_policy'.

To use this command:
1. Fill the following fields in the `.json` config file:
    * `thing_name` with the name of your AWS Thing.
    * `role_prefix` with the prefix for your role. This prefix will be pre-pended to your role name with a hyphen by default. For example, with the prefix `Proj` and role name `role`, the completed role name will become `Proj-role`.
2. Set up following [prerequisites](#prerequisites).
3. Run the command below.

```sh
python tools/scripts/createIoTThings.py cleanup-simplified
```

That's it. Your AWS entities created by this script should now be deleted.

The only time this command will fail to find or remove all AWS entities is if:
1. You have created an update, and deleted the role associated before the update is deleted. You need to try re-creating the role (e.g. via re-running `create-update-simplified` with the same config). You may see an error message indicating that you `cannot assume a role` to delete an OTA update.
2. You have run `create-update-simplified` with config A, then used config B which specifies a different role, policy, update, or bucket name <b>format</b> from A. For example, changing `policy_name` from `${thing_name}_policy` in A to `myTestPolicy` in B will mean that `cleanup-simplified` cannot find `${thing_name}_policy` if run with config B. To fix this, run `cleanup-simplified` with each config separately.

## Troubleshooting

##### 1. My AWS credentials are rejected, despite being accepted earlier.

AWS credentials such as `AWS_SESSION_TOKEN` occasionally expire. It is the responsibility of the user to ensure their tokens are up to date (by re-`export`ing them).
