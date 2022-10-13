include(FetchContent)

FetchContent_Declare(
    paddle
    URL https://paddle-wheel.bj.bcebos.com/2.0.2/win-infer/mkl/cpu/paddle_inference.zip
    URL_HASH MD5=d71dd033e30d0930695b75c498460967
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(paddle)
add_library(paddle_inference SHARED IMPORTED GLOBAL)
set_target_properties(paddle_inference PROPERTIES
    IMPORTED_LOCATION ${paddle_SOURCE_DIR}/paddle/lib/paddle_inference.dll
    IMPORTED_IMPLIB ${paddle_SOURCE_DIR}/paddle/lib/paddle_inference.lib
)
target_include_directories(paddle_inference INTERFACE ${paddle_SOURCE_DIR}/paddle/include)
add_library(paddle::inference ALIAS paddle_inference)


FetchContent_Declare(
    autolog
    GIT_REPOSITORY https://gitee.com/Double_V/AutoLog
    GIT_TAG        main
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(autolog)
add_library(autolog INTERFACE)
target_include_directories(autolog INTERFACE ${autolog_SOURCE_DIR})
add_library(autolog::header ALIAS autolog)