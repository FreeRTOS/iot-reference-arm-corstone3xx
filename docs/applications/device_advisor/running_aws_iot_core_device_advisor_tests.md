# Running AWS IoT Core Device Advisor tests

Device Advisor is a cloud-based, fully managed test capability for validating
IoT devices during device software development. Follow the [link](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor.html)
to learn more about the device advisor.

## Creating an IoT thing

Follow the instructions described in the section listed below to create an [IoT thing][creating-an-iot-thing-for-your-device], policy, IAM Policy and IAM role
for your device.

## Setting up IAM Roles and Policies

* Create an IAM role to use as your device role.
  * As part of creating the policy, the topic and topic filter shall be assigned a value `*` and the `clientId` should match the IoT thing name.
* Create a custom-managed policy for an IAM user to use Device Advisor
* Create an IAM user to use Device Advisor (AWS recommendation)

1. Go to the [IAM Dashboard](https://us-east-1.console.aws.amazon.com/iam/home?region=us-west-2#/home).
2. In the left navigation pane under **Access management** select **Policies**, then click on **Create policy**.
    * Select the `IoT` service, then click on **Next**.
    * Under `IoT` **Actions allowed** select the following: `Connect`, `Publish`, `Subscribe`, `Receive` and
      `RetainPublish`.\
      Under **Resources**: Ether keep it on `All`, or specify the **client**, **topic** and **topicfilter** with
      clicking on **Add ARNs**.
      Make sure that you select the same **Resource region** that you specified when creating your **IoT Thing**,
      or specify it as `*`. You can set the **Resource client** as your **IoT Thing** name or `*`. In the end you should see:
      ```text
      client:         arn:aws:iot:eu-west-1:{account-id}:client/*
      topic:          arn:aws:iot:eu-west-1:{account-id}:topic/*
      topicfilter:    arn:aws:iot:eu-west-1:{account-id}:topicfilter/*
      ```
      then click on **Next**.
    * Give a **Policy name** in which you can include the **region** you are using, like:
      `Proj-device-advisor-eu-west-1-any`, and click on **Create policy**.
3. In the left navigation pane under **Access management** select **Roles**, then click on **Create role**.
    * Select `Custom trust policy`, and set the following **Trust policy**:
      ```json
      {
          "Version": "2012-10-17",
          "Statement": [
              {
                  "Sid": "AllowAwsIoTCoreDeviceAdvisor",
                  "Effect": "Allow",
                  "Principal": {
                      "Service": "iotdeviceadvisor.amazonaws.com"
                  },
                  "Action": "sts:AssumeRole"
              }
          ]
      }
      ```
      then click on **Next**.
    * Under **Permission policies** select the policy that you previously defined, like:
      `Proj-device-advisor-eu-west-1-any`.\
      Under **Set permission boundary** select `Use a permissions boundary to control the maximum role permissions` if
      you have predefined boundary's, then select your project admin boundary.
      Then click on **Next**.
    * Set the role name, like: `Proj-device-advisor-role-eu-west-1-any`, then click on **Create role**.

For more information follow the instructions described in the
[page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-setting-up.html#da-iam-role)
to create a policy for your IoT thing and then a device advisor role.

## Creating AWS IoT Core Qualification test suite

Follow the instructions described the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-create-suite)
to create AWS IoT Core Qualification test suite.

- The `Trigger Topic` property should be set to the value of `deviceAdvisorTOPIC_FORMAT` macro available at [aws_device_advisor_task.h](../../../applications/helpers/device_advisor/inc/aws_device_advisor_task.h) for `TLS Receive Maximum Size Fragments` test.
- The Device role should be a previously created IAM Role, like: `Proj-device-advisor-role-eu-west-1-any`.
- **Make sure that this role is also set up for the same region, that your test is using, or any!**

## Configuring the application to connect to AWS IoT Core Device Advisor

Now that you have created an AWS Thing and attached the certificates and
policies to it, the representative values must be added to your application to
ensure connectivity with AWS IoT Core Device Advisor.

Set the macro `appCONFIG_DEVICE_ADVISOR_TEST_ACTIVE` in
`applications/<application_name>/configs/app_config/app_config.h` to 1.

Within the application directory that you are using, edit the
`applications/<application_name>/configs/aws_configs/aws_clientcredential.h` file and set values for specified
user defines called out below.

`clientcredentialMQTT_BROKER_ENDPOINT`

* Set this to the device advisor endpoint. You can get the device advisor end
  point either via AWS CLI or AWS IoT console:
  * AWS CLI
    * To get the device advisor end point via [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-getting-started.html),
      run:
        ```bash
        aws iotdeviceadvisor get-endpoint
        ```
  * AWS IoT console
    * To get the device advisor end point via AWS IoT console, follow the
      instructions in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/da-console-guide.html).
      The device advisor end point is shown at the step 12.

`clientcredentialIOT_THING_NAME`

* Set this to the name of the thing you set (e.g. MyThing).

Save and close the file.

## Building the application

> ⚠️ **If you’ve built a different application, run the following commands before proceeding**
```bash
git submodule deinit --all -f
git submodule update --init --recursive
```

To build the application, run the following command:

```bash
./tools/scripts/build.sh ${APPLICATION_NAME} --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315/corstone320> --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```

* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's paths during the Thing creation.

Or, run the command below to perform a clean build:

```bash
./tools/scripts/build.sh ${APPLICATION_NAME} --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315/corstone320> --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
```

## Running the application

We need to start the device advisor tests before running the application. If
not then, device advisor rejects the connection requests from the application.

Follow the instructions described in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-run-test-suite)
to start the [previously created](#creating-aws-iot-core-qualification-test-suite) AWS IoT Core Qualification test suite.

Now run the application by running the following command:

```bash
./tools/scripts/run.sh ${APPLICATION_NAME} --target <corstone300/corstone310/corstone315/corstone320>
```

Once the device advisor has completed all the tests, you can download the AWS
IoT Core Qualification report by following the instructions in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-qualification-report).

[creating-an-iot-thing-for-your-device]: ../aws_iot/setting_up_aws_connectivity.md#creating-an-iot-thing-for-your-device
[creating-a-policy-and-attach-it-to-your-certificate]: ../aws_iot/setting_up_aws_connectivity.md#creating-a-policy-and-attach-it-to-your-certificate

## Note
AWS IoT Core Device Advisor Tests are supported on `keyword-detection` application using `Arm GNU Toolchain (arm-none-eabi-gcc)` only.
