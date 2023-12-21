# CliClient - The Content Management System

这是一个CMake项目。
使用CLion / Visual Studio / Visual Studio Code开发和调试这个项目。

## 项目配置

以VSC为例。安装这些软件：
- Git
- CMake
- GCC / Clang / MSYS2
- Visual Studio Code

从Gitee克隆这个项目。
```shell
git clone https://gitee.com/douexpectaname/the_cms.git
```

用VSC打开这个文件夹。
```shell
code ./the_cms
```

配置VSC的C/C++环境。
1. 安装[C/C++插件](vscode:extension/ms-vscode.cpptools)
2. 选择工具链。参考：[Get started with the CMake Tools Visual Studio Code extension on Linux](https://code.visualstudio.com/docs/cpp/cmake-linux#_select-a-kit)

## 编译
用Shift-Ctrl-P（⇧⌘P在Mac）打开命令窗口。用CMake: Build指令编译项目。或者，点击下方Build按钮。

![build-button](https://code.visualstudio.com/assets/docs/cpp/cpp/cmake-build-v2.png)

## 项目结构

有两个CMake子项目。

- tcms: 主程序
- tcms_tests: 单元测试

tcms_tests引入tcms的所有组件。

tcms的源代码在src/和include/目录下。
tcms_tests的代码在tests/目录下。

都用了一个宏来过滤.cpp文件，所以每新建一个.cpp文件，需要同步CMake配置，
而不是将它添加到CMakeLists.txt中。
