# Setting up AWS connectivity

To connect to the AWS cloud service you will need to setup an IoT Thing and
then set the AWS credentials of the IoT Thing within the Application.
You will need to create an [AWS Account](https://aws.amazon.com/premiumsupport/knowledge-center/create-and-activate-aws-account/)
if you don’t already have one.

## AWS account IoT setup

The instructions below will allow the application to send messages to the cloud
via MQTT as well as enable an over-the-air update.

> Note:
  Due to AWS rules, you must ensure that when logging into the
  [AWS IoT console](https://console.aws.amazon.com/iotv2/) you are using
  the same **Region** as where you created your IoT thing.  This
  rule is documented within the [MQTT Topic](https://docs.aws.amazon.com/iot/latest/developerguide/topics.html)
  page in the AWS documentation.


## Creating an IoT thing for your device

1. Login to your account and browse to the [AWS IoT console](https://console.aws.amazon.com/iotv2/).
   * If this takes you to AWS Console, click on **Services** above and then
     click on **IoT Core**.
1. In the left navigation pane **Manage** section, expand **All devices** then
   select **Things**.
   * These instructions assume you do not have any IoT things registered in
     your account.
   * Press the **Create things** button.
1. On the **Create things** page, select **Create single thing** and press the **Next** button.
1. On the **Specify thing properties** page, type a **Thing name** for your
   thing (for example `MyThing_eu_west_1`), and then press the **Next** button.
   * Adding the region name to your thing name helps to remind you which region
     the thing and topic is attached to.
   * You will need to add the thing name later to your C code.
   * There is no need to add any **Additional configuration**
1. If you want to use your own self-signed client certificate then on the **Configure device certificate** page, choose **Skip creating a certificate at this time** and then press the **Next** button. If you want to use auto-generated certificate then choose **Auto-generate a new certificate** and then press the **Next** button.
1. Skip the **Attach policies to certificate** page for now.
   * You will attach a certificate to the thing in a later step.
1. If you use auto-generated certificates then download all the keys and certificates by choosing the **Download**
   links for each.
   * Click on all the **Download** buttons and store these files in a secure
     location as you will use them later.
   * Make note of the certificate ID. You need it later to attach a policy to
     your certificate.
   * Click on **Done** once all items have downloaded.

   If you use self-signed client certificate then you will upload the certificate that is generated in the next step.


## Generating and registering your own device certificate
AWS IoT Core authenticates device connections with the help of X.509 certificates. The steps below describes how to generate self-signed device certificate and then register it with AWS IoT Core.

1. Run the ```./tools/scripts/generate_credentials.py``` Python script, that's going to generate a private key
   and a certificate that's signed  with this key.
   * Optionally you can specify metadata for the certificate. Use the ```-h``` flag for the Python script to see the available options.
  ```bash
  python3 ./tools/scripts/generate_credentials.py --certificate_valid_time <validity duration in days > \
                                                  --certificate_country_name <Country Name (2 letter code)> \
                                                  --certificate_state_province_name <State or Province Name (full name)> \
                                                  --certificate_locality_name <Locality Name (eg, city)> \
                                                  --certificate_org_name <Organization Name (eg, company)> \
                                                  --certificate_org_unit_name <Organizational Unit Name (eg, section)> \
                                                  --certificate_email_address_name <Email Address> \
                                                  --certificate_out_path <output path> \
                                                  --private_key_out_path <output path>
  ```
1. In the left navigation panel **Manager** section of the AWS IoT console,
   expand **Security**, and then select **Certificates**.
1. On the **Certificates** page, press the **Add certificates** button and select **Register certificates**.
1. Select the **CA is not registered with AWS IoT** option and upload the **certificate.pem** that's generated in the previous step.
1. Select the checkbox next to the uploaded certificate and then click on the **Activate** button.
1. Click on the **Register** button.
   * At this point, the certificate is registered and activated. In the **Security** > **Certificates** menu, you can see the new certificate.
1. Go to the  **Security** > **Certificates** menu and select the newly registered certificate.
1. On the **Things** tab click on the **Attach to things** button and select your Thing.


## Creating a policy and attach it to your certificate

1. In the left navigation pane **Manage** section of the AWS IoT console,
   expand **Security**, and then select **Policies**.
1. On the **Policies** page, press the **Create Policy** button.
   * These instructions assume you do not have any **Policies** registered
     in your account,
1. On the **Create Policy** page
   * Enter a name for the policy in the **Policy properties** section.
   * In the **Policy document** section, add 3 new policy statements to have a
     total of 4 policy statements with the following values:
     * **Policy effect** - select **Allow** for all statements.
     * **Policy Action** - select one of the actions below for each statement.
       * **iot:Connect**
       * **iot:Publish**
       * **iot:Subscribe**
       * **iot:Receive**
   * The **Policy resource** field requires an **ARN**. Sometimes this box will
     be auto-filled with your credentials.
     * If no value exists, use the following format:    (arn:aws:iot:**region:account-id:\***)
       * region (e.g. eu-west-1)
       * account-id ... This is your **Account ID Number**.
         * You can usually see this in the drop down on the top right corner
           where your login name is shown.
       * e.g. *arn:aws:iot:eu-west-1:012345678901:*
     * Replace the part, or add, after the last colon (`:`) with `*`.
       * e.g. *arn:aws:iot:eu-west-1:012345678901:\**
     * Press the **Create** button.

   > NOTE - The examples in this document are intended for development
     environments only.  All devices in your production fleet must have
     credentials with privileges that authorize only intended actions on
     specific resources. The specific permission policies can vary for your use
     case. Identify the permission policies that best meet your business and
     security requirements.  For more information, refer to [Security Best
     practices in AWS IoT Core](https://docs.aws.amazon.com/iot/latest/developerguide/security-best-practices.html).

1. In the left navigation pane **manage** section of the AWS IoT console,
   expand **Security**, and then select **Certificates**. You should see the
   certificate for the thing you created earlier.
   * Use the ID in front of the certificate and key files that you downloaded
     after creating the thing to identify your certificate.
   * Select your certificate name to take you to your Certificate page.
1. Expand the **Actions** drop down list and select **Attach policy**.
   Alternatively, press the **Attach policies** button in **Policies** tab.
1. In the **Attach policies to the certificate** dialog box
   * Choose the policy you created earlier.
     * Even though you may enable more than one policy, for now we use the
       single policy you created earlier.
   * Press the **Attach policies** button.
   * You will now see your policy listed in the **Policies** tab on the
     Certificate page.

## Configuring the application to connect to your AWS account
Now that you have created an AWS Thing and attached the certificates and
policies to it, the representative values must be added to your application to
ensure connectivity with your AWS account.

Within the application directory that you are using, edit the
`configs/aws_configs/aws_clientcredential.h` file and set values for specified
user defines called out below.

`clientcredentialMQTT_BROKER_ENDPOINT`

* Set this to the Device data endpoint name of your amazon account.
* To find this go to the navigation pane of the [AWS IoT console](https://console.aws.amazon.com/iotv2/),
  choose **Settings** (bottom left hand corner).
* On the **Settings** page, in the **Device data endpoint** section of the page
  look for **Endpoint**.  (e.g. `a3xyzzyx-ats.iot.us-east-2.amazonaws.com`).
  * Note the region may be different than these instructions.  It should match
    where your thing and policy were created due to the MQTT Topic restrictions
    discussed above.

`clientcredentialIOT_THING_NAME`

* Set this to the name of the thing you set (e.g. MyThing).

Save and close the file.

The device certificate PEM and private key PEM will be set during the build configuration.
