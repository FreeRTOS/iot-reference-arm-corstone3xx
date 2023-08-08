# Launch Arm Virtual Hardware Instance

To utilize the Arm Virtual Hardware, you will need to create an [AWS Account](https://aws.amazon.com/premiumsupport/knowledge-center/create-and-activate-aws-account/)
if you don't already have one.

## Launching the instance in EC2 [(AWS on getting started)](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/EC2_GetStarted.html)

1. Go to [EC2](https://console.aws.amazon.com/ec2/v2/) in the AWS Web Console.
1. Select **Launch Instance** which will take you to a wizard for launching the
   instance.
    > Arm Virtual Hardware for Corstone-300 is available as a public beta on
      AWS Marketplace. To help you get started, AWS are offering more than 100
      hours of free AWS EC2 CPU credits for the first 1,000 qualified users.
      Click [here](https://www.arm.com/company/contact-us/virtual-hardware) to
      find out more.

     * **Step 1: Create a Name for your Instance** - To clearly identify the
       instance you are about to create you will need to apply a descriptive
       name.  It can be as simple as "J. Doe's AVH Instance".

     * **Step 2: Choose an [Amazon Machine Image (AMI)](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/AMIs.html)**
        * In the Search box, type `Arm Virtual Hardware` and then hit "enter"
          to find the item called <ins>"Arm Virtual Hardware" - "By Arm"</ins>.
          > Select: **Arm Virtual Hardware By Arm | Version 1.3.1**
          * NOTE: If you do not see the expected items, make sure the
            <ins>**AWS Marketplace AMIs**</ins> tab is selected.
        * Click on "Select" for that item. This image contains all the software
          necessary to build and run the Arm FreeRTOS Featured Reference
          Integration.
          * This will raise a subscription page/pop-up titled,
            **Arm Virtual Hardware**.
          * You will note that the subscription is free from Arm, but
            <ins>AWS does charge for the costs of the instances themselves
            according to the pricing chart provided.</ins>
        * You must select "Continue" if you want to move forward.

     * **Step 3: Choose an Instance Type** - Select one of the instance types
       from the list.
        * We recommend the **c5.large**.
        * **Important:** Charges accrue while the instance is running and to a
          lesser degree when stopped.
        * Terminating the instance stops any charges from occurring.

     * **Step 4: Key pair (login)**
       * To ensure easy connection when using SSH from a local terminal, it is
         recommended to create a key pair at this time.
       * Click on **Create new key pair**
       * Enter a descriptive name, e.g. My_Key_Pair_AVH_us-west-2 or simply
         **MyKeyPair**
         * To keep track of all the different things you create, we recommend
           adding your active region to the name.
       * Using the defaults options is fine.
       * Click on **Create key pair**
       * A private key is downloaded, place this in a location you will not
         forget as it is used later.
       * Once saved, you must fix the permissions of the file wherever you just
         stored it:
       ```sh
            chmod 400 MyKeyPair.pem
        ```

     * **Step 5: Configure storage** - To ensure enough disk drive space to
       contain the entire build image.  Set the amount of storage to
       "1x **30** GiB".

     * **Final Step:** From here you may select **Review and Launch** to move
       directly to the launch page or continue to configure instance details if
       you need to set any custom settings for this instance.

## Selecting the instance

Once you complete the wizard by initiating the instance launch you will see a
page that allows you to navigate directly to the new instance. You may click
this link or go back to your list of instances and find the instance through
that method.

Whichever way you choose, find your new instance and select its instance ID to
open the page to manage the instance.

## Connecting to the instance:

Choose your terminal connection type (AWS-Web-Console or Local-Console)
* AWS-Web-Console
   * Go to [EC2](https://console.aws.amazon.com/ec2/v2/) in the AWS Web Console
   * Click on "Instances"
   * Find the instance you created earlier
   * Click on the instance
   * Select Connect to open an SSH terminal session to the instance in your
     browser
     * Ensure the User name field is set to `ubuntu`.
   * Select the Connect button to open the session.
     * This will put you in a browser window where you will have an SSH
       terminal window ready for your input.
* Local-Console
   * Open your favorite terminal program or linux shell application and connect
     to the AVH AMI instance:
   * AWS requires you to use a secure connection, using the instance
     certificate you downloaded earlier.
   * e.g. ssh -i .ssh/MyKeyPair.pem ubuntu@\<**Public IPv4 DNS**\>
     * Look in your instance page at [EC2](https://console.aws.amazon.com/ec2/v2/)
       AWS Web Console for the **Public IPv4 DNS** value of your Instance.

   Example
   ```sh
   ssh -i .ssh/MyKeyPair.pem ubuntu@ec2-xxx.xxx.xxx.xxx.eu-west-1.compute.amazonaws.com
   ```
   If you do not know your instance value, refer to the **AWS-Web-Console**
   instructions just above to get this information.

## Terminate Arm Virtual Hardware Instance

When you are done using the AMI instance at the end of the day, you need to
make sure you shut it down properly or you may be charged for usage you did not
actually use. This can be done as follows:

1. Go to [EC2](https://console.aws.amazon.com/ec2/v2/) in the AWS Web Console.
1. Select the instance to stop.
1. Click on `Instance state` and select `Stop Instance` in the drop down menu.
