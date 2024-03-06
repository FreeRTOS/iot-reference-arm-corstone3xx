# ML Embedded Evaluation Kit

The ML Embedded Evaluation Kit is for building and deploying Machine Learning (ML) applications
targeted for Arm Cortex-M CPUs and Arm Ethos-U NPU.


## Configuration

Available build options and their default values are documented in the [Build options] section of the ML Embedded
Evaluation Kit documentation.
The minimum set of options that need to be defined are the following:

* `ETHOS_U_NPU_ID`
* `ETHOS_U_NPU_CONFIG_ID`
* `ETHOSU_TARGET_NPU_CONFIG`
* `ETHOS_U_NPU_MEMORY_MODE`

In addition to the above, `ML_USE_CASE` build option describes which TFLite model and API libraries should be generated.
`ML_MODEL` is the name of the CMake module under `components/ai/ml_embedded_evaluation_kit/integration/cmake/model/`
that handles the generation of C++ source code model from the TFlite model.

## Using in your application

After setting the build options, include [SetupMlEmbeddedEvaluationKitLibraries.cmake] to handle the configuration of
necessary ML libraries and running Vela compiler on the default models for the set `ETHOSU_TARGET_NPU_CONFIG`.

In order for your application to access the API headers for the libraries, you need to link to the two resulting static
libraries: `${ML_USE_CASE}_api`, `${ML_USE_CASE}_model`.
These libraries provide the necessary include paths to the respective APIs from the ML Embedded Evaluation Kit.
In addition, you need to provide a JSON file containing the metadata for the model to use.
This is provided to the build system by setting the path to the JSON file with the
`ML_USE_CASE_RESOURCES_FILE` CMake variable.

## Documentation

For more information about the ML Embedded Evaluation Kit, see the [ML Embedded Evaluation Kit documentation].

[Build options]: https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/HEAD/docs/sections/building.md#build-options
[SetupMlEmbeddedEvaluationKitLibraries.cmake]: ../../../../components/ai/ml_embedded_evaluation_kit/integration/cmake/SetupMlEmbeddedEvaluationKitLibraries.cmake
[ML Embedded Evaluation Kit documentation]: https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/HEAD/docs/documentation.md
