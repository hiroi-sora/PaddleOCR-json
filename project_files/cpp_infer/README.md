## 使用 CMake 快速构建
### 依赖安装
本项目使用 [vcpkg](https://github.com/microsoft/vcpkg) 管理第三方开源库。在开始前请先配置好`vcpkg`，并安装以下依赖库：
```shell
vcpkg install opencv dirent nlohmann-json
```
### 项目构建
进入 C++ 工程目录：
```shell
cd project_files/cpp_infer
```
开始配置：
```shell
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake .
```
`thirdparty/CMakeLists.txt` 会自动下载 `paddle` 及其相关依赖库，并集成到 CMake 构建系统中。由于网络原因，该过程可能会有些耗时。

编译安装
```shell
cmake --build build --config Release --target install
```
安装目录位于 `build/installed`

打包
```shell
cmake --build build --config Release --target package
```