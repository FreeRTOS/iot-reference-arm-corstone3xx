# Writing unit tests for integration code

## Introduction

This tutorial will lay out the steps for adding unit tests for IoT Reference Integration for Arm Corstone-3xx integration code. It uses the [GoogleTest][googletest_home_page] test runner along with the [Fake Function Framework][fff_link] (`fff`) for creating mock C functions for the tests.

`fff` is a C/C++ micro-framework that allows users to create fake C functions for use in unit tests. When unit testing, this allows for functions to be mocked, with different levels of manipulation to be applied to functions such as:

* setting return values,
* counting the number of calls of a function,
* examining parameters that functions are called with.

> :bulb: There are many more functionalities available within `fff`, consult the framework documentation [here][fff_readme_contents] for further information.

The IoT Reference Integration for Arm Corstone-3xx aims to provide mocked symbols for all components it fetches. This allows for unit tests to be written for the component's integration code as well as for other components's integration code that depend on it.

`coreMQTT Agent` and `FreeRTOS-Kernel` mocks will be used as examples for which symbols to mock and how to mock them. The mocks can be found [here](../components/aws_iot/coremqtt_agent/library_mocks) and [here](../components/freertos_kernel/library_mocks) respectively.

The `coreMQTT Agent` mocks are used to write unit test for the `coreMQTT Agent`'s integration code. The unit test can be found [here](../components/aws_iot/coremqtt_agent/integration/tests/test_freertos_agent_message.cpp).

### Unit test scope

Within the IoT Reference Integration for Arm Corstone-3xx, we unit test any integration code that is added to integrate a component, this ensures the code works as intended. The code originating from the component itself is not tested by unit test, this is mocked instead. The component code is tested with integration tests from [freertos_libraries_integration_tests](https://github.com/FreeRTOS/FreeRTOS-Libraries-Integration-Tests) module, and by running applications on targets.

## Mocks

### What is a mock?

A mock is defined by the user and is intended to simulate existing symbols from a component. This allows for the unit test to precisely control the execution path in the file under test for each test case. The mocks are defined in the `components` sub-directory for the component mocked. The sub-directory structure is as follows:
``` tree
├── ${REAL_LIBRARY_NAME}/
│   ├── library_mocks/
│   │   ├── inc/
│   │   ├── src/
│   │   ├── CMakeLists.txt
│   ├── CMakeLists.txt
```

The mocks should be made available as a CMake library target so it can be linked by unit test CMake executable targets that require the mocked symbols. The library target is named after the real component where the symbols would be with the added `-mock` suffix. For example, the mocks for coreMQTT Agent are made available via the `coremqtt-agent-mock` CMake target.

### Adding mocked header files

Header files should be added following the sub-directory structure in the component being mocked. This is to ensure that their inclusion in the integration code does not generate compiler errors.

For example, if a header file is included as such:
```c
#include "component/header_1.h"
```

The folder structure within the `inc` directory should include any sub-directory, where the header file is located. In this case, the structure would be as follows:
``` tree
├── component/
│   ├── library_mocks/
│   │   ├── inc/
│   │   |   ├── component/
│   │   |   |   ├── header_1.h
│   │   ├── src/
│   │   ├── CMakeLists.txt
│   ├── CMakeLists.txt
...
```

The `inc` sub-directory should be the directory exposed by the mocked component CMake target see [Creating mocked component CMake target library][creating-mocked-component-cmake-target].

### Creating mocked component CMake target library

As the IoT Reference Integration for Arm Corstone-3xx utilises CMake as its build system, all mocks are provided as CMake library targets which can the be used throughout the project.

### Listing the mock subdirectory

To ensure the mocks directory is built only for unit testing, list it in CMake with a condition. The [coreMQTT Agent component CMake][inclusion-of-mocks-subdir] illustrates this:
```cmake
if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(library_mocks)
else()
    <CMake script relating to the real component>
endif()

```
This ensures that the mock is included in the build only when `CMAKE_CROSSCOMPILING` is disabled. When the condition is not met, the component real files are built instead of the mock.
The CMAKE_CROSSCOMPILING flag is set by CMake when not building for the host machine (e.g. for Corstone-300). This is only disabled when we are building unit tests for our host machine and for CI.
For future-proofing, we also use the BUILD_TESTING flag that is set by CTest.

The `library_mocks` directory must be added before the `tests` directory. This ensures the mocked CMake library target can be referenced in the test's `CMakeLists.txt` to link it. This subdirectory is linked in the component [main][inclusion-of-mocks-subdir] `CMakeLists.txt` file.

### Mocked component library

The CMake library target for the mocked component should be added in the `CMakeLists.txt` within the `mocks` sub-directory. The name of the mocked component should follow the name of the CMake library where the real symbols normally are with the added `-mock` suffix.
Use the CMake `add_library()` function to create the CMake library target. Source files that implement the mocked symbols should be added to that library, as seen for the `FreeRTOS-Kernel Mocks` [here](../components/freertos_kernel/library_mocks).

#### Example for adding CMake library

This section assumes the mocks have ```<source files>``` (i.e. ```.c``` files) to include.
If this is not true, look [here][example-for-adding-a-cmake-library-which-contains-only-header-files] for how to add interface libraries.

Firstly, add the library as shown below:

```cmake
add_library(${REAL_LIBRARY_NAME}-mock
    <source files>
)
```
> :bulb: Where `${REAL_LIBRARY_NAME}` is the name of the real CMake library target being mocked.

Source files that implement the mocked library should be located within a `src` subdirectory within the `mocks` subdirectory, as laid out in the tree [here][what-is-a-mock].

Next, list the `inc` subdirectory which contains all of the components mocked header files:
```cmake
target_include_directories(${REAL_LIBRARY_NAME}-mock
    PUBLIC
        inc
)
```
The `inc` sub-directory should only include header files required by the mocks. Additional sub-directories may be added in `inc` if the real library header files are accessible only by also including the path to the header file. The access specifier should be set to `PUBLIC` to enable consumers of the mocked library to access the mocked symbols.

Finally, link any required libraries for the mocks to function. `fff` is a must for mocks, as this is the framework used. Any other libraries linked here must be mock libraries, add only the ones that are required. The access specifier here should be set to `PRIVATE` to ensure only this library has access to the libraries being linked.
```cmake
target_link_libraries(${REAL_LIBRARY_NAME}-mock
    PRIVATE
        fff
        <any-other-required-mock-libraries>
)
```

#### Example for adding CMake library (with no .c source files)

```cmake
add_library(${REAL_LIBRARY_NAME}-mock INTERFACE)
target_include_directories(${REAL_LIBRARY_NAME}-mock
    INTERFACE
        inc
)
target_link_libraries(${REAL_LIBRARY_NAME}-mock
    INTERFACE
        fff
        <any-other-required-mock-libraries>v
)
```
In summary: replace every occurrence of the `PRIVATE` and `PUBLIC` access modifiers in the example above with `INTERFACE`.

#### Example for adding a CMake library which contains only header files

The primary difference is that instead of using ```PRIVATE``` and ```PUBLIC``` modifiers, we use ```INTERFACE```. Additionally, ```add_library``` must indicate this library is only an interface.

For example:

Firstly, add the library as shown below:
```cmake
add_library(${REAL_LIBRARY_NAME}-mock INTERFACE)
```

Next, list the `inc` subdirectory which contains all of the components mocked header files:
```cmake
target_include_directories(${REAL_LIBRARY_NAME}-mock
    INTERFACE
        inc
)
```

Finally, link any libraries required by the mocks.
```cmake
target_link_libraries(${REAL_LIBRARY_NAME}-mock
    INTERFACE
        fff
        <any-other-required-mock-libraries>
)
```

### Creating header files
When creating the header file, the aim is to create one which closely matches the real file. Below will lay out the process of adding items to the header files, with the aim of only adding what is strictly necessary.

#### Header file protection

The header file should include the same `#define` protection as the real file, for example, `core_mqtt_agent_message_interface.h` within the coreMQTT-Agent mocks uses the same protection as the real file, see below.

[Real File][coremqtt-agent-message-interface-header-real]
```c
#ifndef CORE_MQTT_AGENT_MESSAGE_INTERFACE_H
#define CORE_MQTT_AGENT_MESSAGE_INTERFACE_H

(contents of real file)

#endif
```

[Mock File][coremqtt-agent-message-interface-header-mock]
```c
#ifndef CORE_MQTT_AGENT_MESSAGE_INTERFACE_H
#define CORE_MQTT_AGENT_MESSAGE_INTERFACE_H

(contents of mocked file)

#endif
```

### Mocking datatypes
When mocking datatypes, it is only required to mock Datatypes that are unique to a component and are defined as part of the header files. Standard C datatypes can be used as normal and do not require mocking.

When defining the mocked datatype, for `enum` and `struct` types, ensure you only mock what is required for the unit tests. Datatypes that are not globally accessible can be skipped.

#### Struct typedef declarations

If we only needed to access certain members of the struct in the integration code under test, we would define it as
```c
typedef struct{
    type required_parameters;
} a_struct_definition_t;
```
This ensures we do not have unnecessary members defined for structs.

However, if the integration code does not access the parameters of this struct, we can simply implement it as a `typedef` to `int`.
```c
typedef int a_struct_definition_t;
```
By doing this we can simplify the mock that is being created, without affecting functionality of the integration code.

Ensure that the name you give the mocked datatype exactly matches the real datatype to ensure there are no compatibility issues.



#### Enumeration declarations

For enumerations, the same philosophy applies here that we should only mock what is required. However, you should ensure that the values assigned to each enumerator matches the real file enumeration values for consistency. It may be required to explicitly define values if only a partial definition is being done to avoid different values. See an example below:
```c
typedef enum
{
    STATUS_OK = 0,
    STATUS_ERROR,
    STATUS_TIMEOUT
} status_t;
```
Above is an example of an enum that might be used within the integration code, with the values assigned to each enum increasing by one each time. However, if the integration code only uses `STATUS_OK` and `STATUS_TIMEOUT`, it will be required to explicitly define the value assigned. See below
```c
typedef enum
{
    STATUS_OK = 0,
    STATUS_TIMEOUT = 2
} status_t;
```
By doing this, it is ensured that the value is consitstent in both the mock and real file.

### Mocking functions
Functions should be mocked using both a header and source file. The declaration of a function will be in the header file, with the definition residing in the source file.

To mock a function, `fff`'s [cheat sheet][fff-cheat-sheet] can be used as a template for the macro to use for a specific function. This gives all the available options for handling functions and the expected application of these with examples.

To declare a function, one of four Macros can be used:

* `DECLARE_FAKE_VOID_FUNC` - For `void` functions with a fixed number of parameters.
* `DECLARE_FAKE_VOID_FUNC_VARARG` - For `void` functions with a variable number of parameters.
* `DECLARE_FAKE_VALUE_FUNC` - For value returning functions with a fixed number of parameters.
* `DECLARE_FAKE_VALUE_FUNC_VARARG` - For value returning functions with a variable number of parameters.

For example from the coreMQTT Agent mocks:

The following function is declared in the `queue.h` header file within FreeRTOS-kernel:
```c
BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait);
```
This function is required within the integration code for the IoT Reference Integration for Arm Corstone-3xx. To mock it, the entry into the `queue.h` mocked header file would be as follows:
```c
DECLARE_FAKE_VALUE_FUNC(BaseType_t, xQueueSendToBack, QueueHandle_t, const void*, TickType_t);
```

The mocked implementation of the function is then defined within the `queue.c` file, where the mock has the same name in the IoT Reference Integration for Arm Corstone-3xx. The definition is as follows:
```c
DEFINE_FAKE_VALUE_FUNC(BaseType_t, xQueueSendToBack, QueueHandle_t, const void*, TickType_t);
```

There may be situations where functions are declared and defined in header and source files which have file names that do not match. In this case, ensure that the file names used for the mocks match the names used in the real files throughout.

### Mocking macros

There may be a need to mock defines that are defined in the component for use in the integration code, these can simply be copied.
```c
#define configTICK_RATE_HZ                               ( 1000 )
```

For macro functions should be mocked as regular [functions][mocking-functions].

## Creating unit test files

Unit test files should be created within a `tests` sub-directory adjacent to the `src` subdirectory containing the file being tested, as follows:

``` tree
├── src/${FILE_UNDER_TEST}.c
│   tests/
│   ├── test_${FILE_UNDER_TEST}.cpp
│   |── CMakeLists.txt
...
```
The IoT Reference Integration for Arm Corstone-3xx convention is to create [GoogleTest][google-test] test suites within a C++ file. Test files should be named as `test_${FILE_UNDER_TEST}.cpp` where `${FILE_UNDER_TEST}` is the name of the source file for the integration code.

For example, `freertos_agent_message.c` will have the test file `test_freertos_agent_message.cpp`. Every source file for the integration code should have its own test file. The source file will be referred from this point onwards as the "file under test".

### Including C header files in the test file

Use the `extern "C"` pre-processor directive to include C header files within the test file for the source file to be visible to the toolchain.

E.g.
```cpp
extern "C" {
    #include "freertos_agent_message.h"
}
```
### Creating `fff` test class

Add the `fff` global variables by adding the `DEFINE_FFF_GLOBALS` macro.

[GoogleTest][google-test] requires the use of a test suites classes, inheriting from `::testing::Test`, for running the unit tests. The class should be named as follows, in PascalCase form:
```c
Test<NameOfFileUnderTest>
```

So for the file `freertos_agent_message.c`, the test class will be called `TestFreertosAgentMessage`.

Child test suites can inherit from other test suites, with some setup already configured. For example if a set of test cases require a mutex initialisation, rather than repeating the mutex initialisation in all test cases, you could create a test suite name `TestFreertosAgentMessageWithMutexInitialized` which has already initialized the mutex being used. This example test suites should inherit from the minimum test suite and any mocked functions called during the execution of the test cases should also be reset.

#### Resetting mocked functions

All the mocked functions called during the execution of the test cases should be reset in the test suite constructor, `fff` provides the `RESET_FAKE` macro to do this.

E.g.

```c++
class TestFileUnderTest : public ::testing::Test{
public:
    TestFileUnderTest()
    {
        RESET_FAKE(executed_mocked_function_1);
        RESET_FAKE(executed_mocked_function_2);
    }
}
```

### Creating test cases

Test cases are written using [GoogleTest's macro][googletest-testing-reference] for writing tests macro.

The test name should be describing what you are testing.
For example, a test for checking if a function for initializing a mutex returns an error if a parameter (ctx) is null could be named `initializing_mutex_fails_when_context_is_nullptr`

This tells a user exactly what is being tested and what the expected outcome is.

The test case is written in snake case with no space characters.

An example of a test, taken from [coreMQTT Agent integration unit test](../components/aws_iot/coremqtt_agent/integration/tests/test_freertos_agent_message.cpp) can be seen below:

```c++
TEST_F(TestFreertosAgentMessage, sending_nullptr_message_returns_false)
{
    MQTTAgentCommand_t *command;
    EXPECT_FALSE(Agent_MessageSend(nullptr, &command, 0));
}
```

Set the expected return values of the mocked functions before calling a function to test.

E.g
```c++
TEST_F(TestFreertosAgentMessage, failing_to_receive_a_message_returns_false)
{
    xQueueReceive_fake.return_val = pdFAIL;

    MQTTAgentMessageContext_t message;
    MQTTAgentCommand_t *command;
    EXPECT_FALSE(Agent_MessageReceive(&message, &command, 1));
}
```
You can see from the above example that reception of message in the queue is set to fail before calling the function to receive a message.

> :bulb: All the available assertions to use within GoogleTest can be found [here][googletest-assertions].

### Avoiding implementation specific tests

It is the preferred to test functionality rather implementation details. If values are returned (by value or reference) by the function called in the test case, test against the returned value. This allows for the implementation of a function to be changed and the functionality tested without breaking the test case.

An example of a good test would be as follows. This will test the return value of the function using GoogleTest's `EXPECT_EQ` assertion macro.
```c++
EXPECT_EQ(0, create_mutex());
```

> :bulb: For assertion macros that compare values, such as `EXCEPT_EQ` and `EXCEPT_NE`, the expected value should be passed as the first parameter. If this assertion fails, the error message will better inform about the cause of the failure.

The following test would not be as good because it is reliant on the implementation of the function. If the function `cloud_mutex_init` is changed or removed, this test is no longer applicable and requires extra maintenance.
```c++
EXPECT_GT(cloud_mutex_init_fake.call_count, 0);
```
This will check the call count of the mocked function and pass the test if this is greater than zero.

Functions that do not have observable primary effects should be tested for their side effects to test the behavior expected. If the side effects of the functions cannot be tested there should not be test cases for the behaviour.

```c
TEST_F(TestNetworkManager, network_initialization_fails_when_config_is_nullptr)
{
    NetworkContext_t context = {};
    initialize_network(&context, nullptr);

    EXPECT_NE(
        0,
        connect_to_network(
            &context, [](NetworkContext_t *context) {
                (void)context;
                return 0;
            }
        )
    );
}

```

This is testing the function `initialize_network`, however this function does not return a value. We can however use the function `connect_to_network` to verify if the initialization has been successful. In this case, we expect an error value to be returned from `connect_to_network` as the initialization process has not been successful.

> :bulb: This can be avoided by using a Test Driven Development (TDD) approach which will ensure that all functions developed can be tested. Writing unit test cases for existing modules does not guarantee that the behavior can be tested without observing implementation details.

### Testing with configASSERT

We define the macro configASSERT to be:

```c
#define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ );
```
Where `vAssertCalled` is a void-returning fake function.

We want to test the below function:
```c
MQTTAgentCommand_t * Agent_GetCommand( uint32_t blockTimeMs )
{
    MQTTAgentCommand_t * structToUse = NULL;
    bool structRetrieved = false;

    /* Check queue has been created. */
    configASSERT( initStatus == QUEUE_INITIALIZED );

    /* Retrieve a struct from the queue. */
    structRetrieved = Agent_MessageReceive( &commandStructMessageCtx, &( structToUse ), blockTimeMs );

    if( !structRetrieved )
    {
        LogDebug( ( "No command structure available.\n" ) );
    }

    return structToUse;
}
```

We want to test that if the function is called with the command pool uninitialized, then it will error and not call `Agent_MessageReceive`.

A naive test would be:

```cpp
TEST_F(TestFreertosCommandPool, does_not_try_to_get_command_if_pool_not_initialized) {
    Agent_MessageReceive_fake.return_val = true;
    Agent_MessageSend_fake.return_val = true;

    // We expect MessageReceive to never be called on unsafe memory.
    Agent_GetCommand(20);
    expect_errors();
    EXPECT_EQ(Agent_MessageReceive_fake.call_count, 0);
}
```
The above will not work. If we do not define a custom fake for `vAssertCalled`, the program will not stop at the line `configASSERT` fails, but instead continue to the next line and call `Agent_MessageReceive`, which will cause the test to fail.

The desired behaviour is:

1. Failing an assertion causes the function under test to stop running.
2. GoogleTest still checks other assertions (such as `Agent_MessageReceive_fake.call_count equals zero`).

The method in use at the moment is to:

1. Define an `ASSERTION_FAILURE` error code and a custom fake for `vAssertCalled`.
2. Define a custom fake for `vAssertCalled`
```cpp
#define ASSERTION_FAILURE 1

/* Mocks for vAssertCalled */
void throw_assertion_failure ( const char * pcFile,
                                      unsigned long ulLine ) {
    /*
    Behaviour wanted:
    - Encounters assertion fail, stops running any more code. E.g. does not go to next line.
    - But checks all assertions in the google test program hold.
    */
    throw (ASSERTION_FAILURE);
}
```
3. Assign the `vAssertCalled` custom fake for every test (by default). Within the initialisation for `TestFreertosCommandPool()`:
```cpp
        vAssertCalled_fake.custom_fake = throw_assertion_failure;
```
4. Within tests expected to fail an assertion, catch the assertion.
```cpp
TEST_F(TestFreertosCommandPool, does_not_try_to_get_command_if_pool_not_initialized) {
    Agent_MessageReceive_fake.return_val = true;
    Agent_MessageSend_fake.return_val = true;

    // We expect MessageReceive to never be called on unsafe memory.
    try{
        Agent_GetCommand(20);
    } catch (int num) {
        if (num != ASSERTION_FAILURE) {
            throw (num);
        }
    }
    expect_errors();
    EXPECT_EQ(Agent_MessageReceive_fake.call_count, 0);
}
```

The above example is from the file [`test_freertos_command_pool.cpp`](../components/aws_iot/coremqtt_agent/integration/tests/test_freertos_command_pool.cpp).

### Integrating unit test file with CMake

#### Adding subdirectories

The unit test file is included in the test build using the CMake build system.

The `tests` sub-directory which contains the test file is to be added near the integration code file to be tested.

Just like the `mocks` sub-directory, the `tests` directory is listed conditionally for non cross-compiling test build as follows:
```cmake
if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(tests)
else()
    <CMake script relating to the real component>
endif()
```

#### Unit test's CMakeLists.txt

The snippet below shows a template for the `tests` sub-directory `CMakeLists.txt` file:
```cmake
add_executable(${FILE_UNDER_TEST_KEBAB_CASE}-test
    test_${FILE_UNDER_TEST}.cpp
    ../src/${FILE_UNDER_TEST}.c
)

target_include_directories(${FILE_UNDER_TEST_KEBAB_CASE}-test
    PRIVATE
        ${PATH_TO_FILE_UNDER_TEST_HEADER_FILE_WITH_PUBLIC_API}
)

target_link_libraries(${FILE_UNDER_TEST_KEBAB_CASE}-test
    PRIVATE
        fff
        ${REQUIRED_MOCKED_LIBRARIES}
)

iot_reference_arm_corstone3xx_add_test(${FILE_UNDER_TEST_KEBAB_CASE}-test)
```

* `add_executable`: Creates the CMake target executable and allows the listing of the test file and the file under test.
* `target_include_directories`: Includes the sub-directories that contain the header file(s) that exposes the function of the file under test. See the next bullet point for header files that are part of a mocked CMake target library.
* `target_link_libraries`: Links any library required to successfully build the executable. This includes `fff` as well as any required mocked CMake library.
* `iot_reference_arm_corstone3xx_add_test`: A helper function to make the test suite discoverable by GoogleTest.

#### The different types of mock directory

We will walk through the mocks in `components/aws_iot/coremqtt_agent`.
The directory structure for mocks is:
``` tree
coremqtt_agent
├── library_mocks
├── integration
├── library
├── CMakeLists.txt
```
This top-level contains the following elements:
- `library` is the third-party library code we want to isolate when unit testing.
- `library_mocks` contains mocks for the `library` components. It will contain a `CMakeLists.txt` file that defines the library mocks, which are exported as the library `coremqtt-agent-mock`.
- `CMakeLists.txt` is set to add `library_mocks` and `integration` as subdirectories if we are unit testing, otherwise it adds `integration` and `library`.
This assumes that `integration` contains mocks. Otherwise, `CMakeLists.txt` will not add `integration` if we are unit testing.
- `integration` has the following structure:
``` tree
coremqtt_agent / integration
├── inc
├── integration_mocks
├── src
├── tests
├── CMakeLists.txt
```
These elements are summarized:
- `inc` and `src` contain the integration code we are testing.
- `integration_mocks` contains mocks for the <b>integration code</b> within `inc` and `src`. This is exported as `coremqtt-agent-integration-mock`.
We mock integration code because if two integration files depend on each other, we need to isolate them from each other during testing. Otherwise, bugs in one file could cause failures on the other file's tests.
- `tests` has the following structure:
``` tree
coremqtt_agent / integration / tests
├── config_mocks
├── CMakeLists.txt
├── test_freertos_agent_message.cpp
... other '.cpp' test files.
```
The `config_mocks` folder in this directory contains mocks for config files in the `applications/` project root directory. For example, `applications/keyword_detection/configs/app_config/app_config.h` is mocked in this directory. This does not include mocks that are stored in `applications/helpers`, such as `applications/helpers/events/events.h`. These helper mocks are within `applications/helpers/<helper_name>/mocks`, where `<helper_name>` could be `events`.

To summarise, there are three separate `mocks` subdirectories. From top to bottom of the directory tree:
1. Library `library_mocks` for coremqtt_agent.
2. Integration `integration_mocks` for our coremqtt_agent integration code.
3. Application-specific `config_mocks`.
Additionally, some mocks for `applications/helpers` can be found within `applications/helpers/<helper_module>/mocks`. For example, there are mocks in `applications/helpers/events/mocks/...`.

It is important to note that application mocks are <b>only</b> supported for helpers and config files, not for all application code.

#### Handling dependencies in the same component as the file under test

Assume that the file under test is dependent on a file in the same directory or component as it. We also assume that some other test file somewhere uses a mocked version of `${FILE_UNDER_TEST}`. This is illustrated below.
``` tree
├── integration_mocks
│   ├── inc
│   │   ├── ${DEPENDENCY}.h
│   │   ├── ${FILE_UNDER_TEST}.h
│   ├── src
│   │   ├── ${DEPENDENCY}.c
│   │   ├── ${FILE_UNDER_TEST}.c
├── inc/
│   ├── ${FILE_UNDER_TEST}.h
│   ├── ${DEPENDENCY}.h
├── src/
│   ├── ${FILE_UNDER_TEST}.c
│   ├── ${DEPENDENCY}.c
├── tests/
│   ├── test_${FILE_UNDER_TEST}.cpp
│   |── CMakeLists.txt
...
```

We want to test `src/${FILE_UNDER_TEST}.c`, using the real `inc/${FILE_UNDER_TEST}.h` but the mock `integration_mocks/inc/${DEPENDENCY}.h` and `integration_mocks/src/${DEPENDENCY}.c`.
It is not possible to do this easily using `CMakeLists.txt` and cover every use case.
You should <b>not</b> simply include this component's `-integration-mocks` library in `tests/CMakeLists.txt`, as this will cause `tests/test_${FILE_UNDER_TEST}.cpp` to find `integration_mocks/inc/${FILE_UNDER_TEST}.h` instead of the real `inc/${FILE_UNDER_TEST}.h`.

The advised way of including the mock `integration_mocks/inc/${DEPENDENCY}.h` and `integration_mocks/src/${DEPENDENCY}.c` files is to directly copy-and-paste them into the top of the C++ test file.
In our example, this means copy-and-pasting contents to the following location in `tests/test_${FILE_UNDER_TEST}.cpp`:
```c++
#include "fff.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace std;

extern "C" {
/* The mock .h files (which do not belong to the same component) are included here */
/* E.g. '#include "task.h"' */

/* Directly copy-paste mock headers from the file under test's directory.
    Otherwise, the non-mock files are detected. */

    /* Contents of `integration_mocks/inc/${DEPENDENCY}.h` */

    /* Contents of `integration_mocks/src/${DEPENDENCY}.c` */

}

DEFINE_FFF_GLOBALS

/* Rest of tests continue below */

```

#### Exposing static functions for testing

Some files in the repository contain static functions which need to be tested. See `components/aws_iot/coremqtt_agent/integration/src/mqtt_agent_task.c` for an example.

The advised method of exposing these functions for testing is as follows:

1. Define the following STATIC macro.
```c
#ifdef UNIT_TESTING
    #define STATIC    /* as nothing */
#else /* ifdef UNIT_TESTING */
    #define STATIC    static
#endif /* UNIT_TESTING */
```
2. Replace the `static` scope of the functions under test with `STATIC`.
```c
static int dummy ( void ) {
    return 1;
}
```
Now becomes
```c
STATIC int dummy ( void ) {
    return 1;
}
```
3. In the C++ test file, `extern` the function definitions. Also ensure that `${FILE_UNDER_TEST}.h` is included.
For example:
```cpp
/* ... other contents ... */

#include "${FILE_UNDER_TEST}.h"

/* ... other contents ... */

using namespace std;

extern "C" {

/* ... other contents ... */

    extern int dummy ( void );

/* ... other contents ... */

}

DEFINE_FFF_GLOBALS

/* ... other contents ... */
```

These static functions are now only visible to the file, and any unit testing files that explicitly `extern` them.

## Building and running unit tests

Two commands will be required to build and run the unit tests. The first step is to configure the unit test build directory, this is done with the following CMake command:
```cmake
cmake -S . -B <designated_unit_test_build_directory> -GNinja
```

To build and run the unit tests, use the following command:
```cmake
cmake --build <designated_unit_test_build_directory>
ctest --test-dir <designated_unit_test_build_directory> --output-on-failure
```
This will build and then run the tests. You will see an output for each test, with either pass or fail, and an output at the end stating the total number of tests passed. For any tests that fail, an output will be present for debugging purposes.

[fff_link]:https://github.com/meekrosoft/fff
[fff_readme_contents]: https://github.com/meekrosoft/fff/blob/master/README.md?plain=1#L8
[inclusion-of-mocks-subdir]:../components/aws_iot/coremqtt_agent/CMakeLists.txt
[creating-mocked-component-cmake-target]:#creating-mocked-component-cmake-target-library
[what-is-a-mock]:#what-is-a-mock
[coremqtt-agent-message-interface-header-real]:https://github.com/FreeRTOS/coreMQTT-Agent/blob/main/source/include/core_mqtt_agent_message_interface.h
[coremqtt-agent-message-interface-header-mock]: ../components/aws_iot/coremqtt_agent/library_mocks/inc/core_mqtt_agent_message_interface.h
[fff-cheat-sheet]:https://github.com/meekrosoft/fff#cheat-sheet
[mocking-functions]:#mocking-functions
[google-test]: https://github.com/google/googletest/tree/main
[googletest-testing-reference]: http://google.github.io/googletest/reference/testing.html
[googletest_home_page]: http://google.github.io/googletest/
[googletest-assertions]:https://github.com/google/googletest/blob/main/docs/reference/assertions.md
[example-for-adding-a-cmake-library-which-contains-only-header-files]:#example-for-adding-a-cmake-library-which-contains-only-header-files
