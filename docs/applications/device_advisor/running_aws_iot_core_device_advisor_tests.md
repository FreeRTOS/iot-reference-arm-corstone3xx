# Running AWS IoT Core Device Advisor tests

Device Advisor is a cloud-based, fully managed test capability for validating
IoT devices during device software development. Follow the [link](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor.html)
to learn more about the device advisor.

## Creating an IoT thing

Follow the instructions described in the section listed below to create an IoT thing for your device.

* [IoT thing][creating-an-iot-thing-for-your-device]

## Creating roles and policies

Follow the instructions described in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-setting-up.html#da-iam-role) to create a policy for your IoT thing and then a device advisor role.

* Create an IAM role to use as your device role.
  * As part of creating the policy, the topic and topic filter shall be assigned a value `*` and the `clientId` should match the IoT thing name.
* Create a custom-managed policy for an IAM user to use Device Advisor
* Create an IAM user to use Device Advisor (AWS recommendation)

## Creating AWS IoT Core Qualification test suite

Follow the instructions described the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-create-suite)
to create AWS IoT Core Qualification test suite.

The `Trigger Topic` property should be set to the value of `deviceAdvisorTOPIC_FORMAT` macro available at [aws_device_advisor_task.h](../../../applications/helpers/device_advisor/inc/aws_device_advisor_task.h) for `TLS Receive Maximum Size Fragments` test.

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

To build the application, run the following command:

```bash
./tools/scripts/build.sh ${APPLICATION_NAME} --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315> --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS>
```

* The `certificate pem's path` and `private key pem's path` should be the downloaded key's and certificate's paths during the Thing creation.

Or, run the command below to perform a clean build:

```bash
./tools/scripts/build.sh ${APPLICATION_NAME} --certificate_path <certificate pem's path> --private_key_path <private key pem's path> --target <corstone300/corstone310/corstone315> --toolchain GNU --conn-stack <FREERTOS_PLUS_TCP/IOT_VSOCKET> --psa-crypto-implementation <TF-M/MBEDTLS> -c
```

## Running the application

We need to start the device advisor tests before running the application. If
not then, device advisor rejects the connection requests from the application.

Follow the instructions described in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-run-test-suite)
to start the [previously created](#creating-aws-iot-core-qualification-test-suite) AWS IoT Core Qualification test suite.

Now run the application by running the following command:

```bash
./tools/scripts/run.sh ${APPLICATION_NAME} --target <corstone300/corstone310/corstone315>
```

Once the device advisor has completed all the tests, you can download the AWS
IoT Core Qualification report by following the instructions in the [page](https://docs.aws.amazon.com/iot/latest/developerguide/device-advisor-console-tutorial.html#device-advisor-console-qualification-report).

[creating-an-iot-thing-for-your-device]: ../aws_iot/setting_up_aws_connectivity.md#creating-an-iot-thing-for-your-device
[creating-a-policy-and-attach-it-to-your-certificate]: ../aws_iot/setting_up_aws_connectivity.md#creating-a-policy-and-attach-it-to-your-certificate

## Note
AWS IoT Core Device Advisor Tests are supported on `keyword-detection` application using `Arm GNU Toolchain (arm-none-eabi-gcc)` only.
