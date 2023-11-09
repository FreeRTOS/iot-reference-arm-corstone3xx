# ML Embedded Evaluation Kit

The ML Embedded Evaluation Kit is for building and deploying Machine Learning (ML) applications
targeted for Arm Cortex-M CPUs and Arm Ethos-U NPU.


## Configuration

Available build options and their default values are documented in the [Build options] section of the ML Embedded
Evaluation Kit documentation.

If you would like to override any defaults, you can specify a list of options in `ML_CMAKE_ARGS`. For example:

```cmake
set(ML_CMAKE_ARGS
    -D TARGET_SUBSYSTEM=sse-310
)
```

The above code snippet selects the `sse-310` target subsystem for Corstone-310 instead of the default `sse-300` for Corstone-300.

> :bulb: This must be done *before* your application's `CMakeLists.txt` adds the IoT Reference Integration for Arm Corstone-3xx.

## Build targets

The ML Embedded Evaluation Kit contains a number of components. Instead of building everything, you can save build time
and disk space by specifying a list of components you need in `ML_TARGETS`. For example:

```cmake
set(ML_TARGETS kws)
```

The above code snippet limits the build to the `kws` (keyword spotting) use case and its dependencies.

> :bulb: This must be done *before* your application's `CMakeLists.txt` adds the IoT Reference Integration for Arm Corstone-3xx.

## Using in your application

The ML Embedded Evaluation Kit outputs a number of libraries as `*.a` archives in its build directory
(`${ml-embedded-evaluation-kit_BINARY_DIR}/lib`), such as
`${ml-embedded-evaluation-kit_BINARY_DIR}/lib/libtensorflow-microlite.a` which is TensorFlow Lite for Microcontrollers.
You can link your application against the library archives as needed.

In order for your application to access the API headers for the libraries, you also need to make various paths in the ML
Embedded Evaluation Kit's source directory (`${ml-embedded-evaluation-kit_SOURCE_DIR}`) available to your application as
include directories.

For example, the IoT Reference Integration for Arm Corstone-3xx repository creates helper libraries named `ml-kit`, `ml-kit-kws` and `ml-kit-asr` in
its [`libraries/ml-kit/CMakeLists.txt`][fri-ml-kit-cmake], with library archives and include directories added to those helper
libraries. The [keyword-detection] and [speech-recognition] applications link the helper libraries they need, in order to inherit the library
archives for linking and include directories for API headers.

> :bulb: Which library archives and include directories are relevant depends on a number of factors, including
>
> * the [configuration](#configuration) set via `ML_CMAKE_ARGS`
> * the [build targets](#build-targets) set via `ML_TARGETS`
> * which libraries your application actually uses
>
> For this reason, the IoT Reference Integration for Arm Corstone-3xx does not provide helper targets in `components` that cover all
> possible uses cases, therefore the user application needs to specify its own list of library archives and include
> paths to take from the ML Embedded Evaluation Kit.

## Documentation

For more information about the ML Embedded Evaluation Kit, see the [ML Embedded Evaluation Kit documentation].

[Build options]: https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/HEAD/docs/sections/building.md#build-options
[fri-ml-kit-cmake]: ../../../../applications/libraries/ml-kit/CMakeLists.txt
[keyword-detection]: ../../../../applications/keyword_detection/CMakeLists.txt
[speech-recognition]: ../../../../applications/speech_recognition/CMakeLists.txt
[ML Embedded Evaluation Kit documentation]: https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/HEAD/docs/documentation.md
