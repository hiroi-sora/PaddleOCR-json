find_package(OpenCV CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)

find_path(DIRENT_INCLUDE_DIRS "dirent.h")

file(GLOB PPOCR_SOURCES "*.cpp")

add_executable(ppocr ${PPOCR_SOURCES})
target_include_directories(ppocr PRIVATE ${PROJECT_SOURCE_DIR} ${DIRENT_INCLUDE_DIRS})
target_compile_definitions(ppocr PRIVATE GLOG_STATIC_DEFINE)

target_link_libraries(ppocr
PRIVATE
    paddle::inference
    opencv_ml opencv_dnn opencv_dpm opencv_hdf
    nlohmann_json::nlohmann_json
    gflags::gflags
    autolog::header
    glog::glog
)

install(TARGETS ppocr)