find_package(OpenCV CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)

find_path(DIRENT_INCLUDE_DIRS "dirent.h")

file(GLOB PPOCR_SOURCES "*.cpp")

add_executable(${PROJECT_NAME} ${PPOCR_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${DIRENT_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE GOOGLE_GLOG_IS_A_DLL)
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /openmp)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    paddle::inference
    opencv_imgcodecs
    nlohmann_json::nlohmann_json
    gflags::gflags
    autolog::header
    glog::glog
    Shlwapi
)

install(TARGETS ${PROJECT_NAME})
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
install(IMPORTED_RUNTIME_ARTIFACTS
    opencv_imgcodecs
    glog::glog
    gflags::gflags
)

install(CODE [[
file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPS
    UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPS
    LIBRARIES $<TARGET_FILE:opencv_imgcodecs>
    DIRECTORIES $<TARGET_FILE_DIR:opencv_imgcodecs>
    PRE_INCLUDE_REGEXES $<TARGET_FILE_DIR:opencv_imgcodecs>
    PRE_EXCLUDE_REGEXES "system32"
    POST_INCLUDE_REGEXES $<TARGET_FILE_DIR:opencv_imgcodecs>
    POST_EXCLUDE_REGEXES "system32"
)
foreach(DEP_LIB ${RESOLVED_DEPS})
    file(INSTALL ${DEP_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endforeach()
]])
