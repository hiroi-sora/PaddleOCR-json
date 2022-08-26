## 项目构建指南

本项目基于 [PaddleOCR release/2.6（2022.8.24）](https://github.com/PaddlePaddle/PaddleOCR/tree/release/2.6)。请务必从相同版本的PaddleOCR开始搭建环境。

1. 按照官方说明 [Visual Studio 2019 Community CMake 编译指南](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.6/deploy/cpp_infer/docs/windows_vs2019_build.md) ，完成CPU版本demo的编译。这份说明很详细，一般不会有什么坑了。（一定要使用PaddleOCR v2.6版）
2. 可以使用我的资源文件整合包以保证各工具版本相同。
   
    秒传链接：`11a28975913aa0e1a5016436b4330adc#346fc54d33f2efd5d31eac9fb400f17d#705185184#PaddleOCR-json_构建资源文件.7z`

    包含内容：

| 文件                       | 文件名                                             | 备注                     |
| -------------------------- | -------------------------------------------------- | ------------------------ |
| vs2019Community 在线安装包 | vs_community__3734f7bbd0e841348e33f252cffea2d9.exe | 工作负载勾选C++桌面开发  |
| cmake-3.24.1               | cmake-3.24.1-windows-x86_64.msi                    |                          |
| opencv                     | opencv-4.6.0-vc14_vc15.exe                         | 建议解压到本项目同目录下 |
| cpu_avx_mkl版预测库        | paddle_inference.zip                               |                          |
| PaddleOCR项目代码v2.6      | PaddleOCR-2.6.0.zip                                | 一定要使用v2.6！         |
| 模型库                     | 模型库（内含v2和v3的简中模型库）                   |                          |

1. 若vs成功编译项目生成ppocr.exe，但vs提示`无法启动程序……系统找不到指定的文件`：
解决方案资源管理器上，ALL_BUILD上右键→属性→常规，将`输出目录`改成exe生成的目录，即为……cpp_infer\build\Release。（写绝对路径）
4. 修改生成文件名：ALL_BUILD和ppocr上（两个都要）右键→属性→常规，将`目标文件名`改成`PaddleOCR_json`或任何你喜欢的名字。
5. 当官方demo编译成功可以运行后，将本项目的`project_files\cpp_infer`下`include`和`src`两个目录中的文件替换到工程相同路径中。