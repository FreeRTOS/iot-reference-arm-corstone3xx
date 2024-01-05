# Components

The `components` subdirectory contains external libraries along with other libraries
that are developed as part of FRI.

The libraries are grouped per category within the `components` subdirectory.

`aws_iot`: Includes the components used to make a connection to AWS IoT Core.
`connectivity`: Includes the components used to establish networking.
`freertos_kernel`: Includes the FreeRTOS kernel.
`security`: Includes components that enhance security.
`tools`: Includes components providing tools used by applications.


Each comprise two parts:
* `library` - the external library source code.
* `integration` - additional integration code for the library to build application.

The `integration` subdirectory contains distinct directories:
* `src` - contains integration source code files
* `inc` - contains integration header files
* `patches` - contains patch files to be applied to the library

A modular approach is used to create the components.
Two CMake targets are created for each component:
* library target (if one is not already provided).
* library configuration target for applications to customise the target for
  their use cases.
