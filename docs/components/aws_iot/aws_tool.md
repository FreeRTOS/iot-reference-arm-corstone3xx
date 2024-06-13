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
The above keys and tokens can be found on the AWS online portal. They are also called AWS API keys. They need to be <em>reset occasionally</em> as they expire. See [Troubleshooting](#troubleshooting) for more information.

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
For examples of how to use each command, see [Creating AWS IoT Firmware update](#creating-aws-iot-firmware-update-job-using-the-automated-script).

### Create an AWS IOT setup manually

See the `docs/applications/aws_iot/setting_up_aws_connectivity.md` file for instructions on setting up IoT manually.

## Creating AWS IoT firmware update job using the automated script

Please refer to [AWS IOT basic concepts](#aws-iot-basic-concepts) and [Using the commands](#using-the-commands) for the general instructions and limitation about using the script.

Performing an OTA update will require you to:
  * Create a role with the required permissions
  * Create a bucket to store the update's binaries. Upload the update's binary.
  * Create an AWS thing. Tie the thing to your device by updating the credentials in the source code.
  * Create a policy that allows to connect to MQTT and attach it to the thing.
  * Finally, create and run the OTA update campaign

Create a thing, an IOT policy, and attach the two together with:
```sh
python tools/scripts/createIoTThings.py create-thing-and-policy --thing_name <your_thing_name> --policy_name <your_policy_name> --target_application <target_application_name>
```
Where `<target_application_name>` is one of `keyword_detection`, `object_detection`, `speech_recognition`.
You must specify the `--target_application` argument if creating a Thing.
The script will update your `aws_clientcredential.h` config file automatically. If you have already modified the entries `clientcredentialMQTT_BROKER_ENDPOINT` or `clientcredentialIOT_THING_NAME` in the file, the script will warn you and ask before overwriting.
  > Note: You may also create each things and policies individually, but you'll have to make sure to pass the certificate created by the first command to the second. Certificates will be printed upon creation during the first command. Use `--use_existing_certificate_arn <your_certificate>` on the second command.

This creates and updates the `certificates` directory with the certificate of the newly created thing.
<b>It is the user's responsibility to clean up the `certificates` directory.</b>

You may now use MQTT to send and receive message for that device. See section [Observing MQTT connectivity](../../applications/aws_iot/aws_iot_cloud_connection.md#observing-mqtt-connectivity)

You may now rebuild keyword with those certificates:
```sh
./tools/scripts/build.sh keyword-detection --certificate_path certificates/thing_certificate_<your_thing_name>.pem.crt --private_key_path certificates/thing_private_key_<your_thing_name>.pem.key --target <corstone300/corstone310/corstone315> --inference <ETHOS/SOFTWARE> --audio <ROM/VSI> --toolchain <ARMCLANG/GNU>
```
Next, we'll create the bucket, upload the binary there, create a role capable of running an OTA update, and create the update. All of those with the following command:
```sh
python tools/scripts/createIoTThings.py create-bucket-role-update --thing_name <your_thing_name> --bucket_name <your_bucket_name> --iam_role_name <prefix>-<your_role_name> --update_name <your_update_name> --ota_binary keyword-detection-update_signed.bin --permissions_boundary arn:aws:iam::<your_aws_account_id>:<your company\'s_permission_boundary>
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

## Troubleshooting

##### 1. My AWS credentials are rejected, despite being accepted earlier.

AWS credentials such as `AWS_SESSION_TOKEN` occasionally expire. It is the responsibility of the user to ensure their tokens are up to date (by re-`export`ing them).