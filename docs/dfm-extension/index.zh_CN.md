@page dfm-extension dfm-extension 
@brief 文件管理器扩展开发库

# 模块介绍

文件管理器（以下简称文管，dde-file-manager）提供一套插件开发接口来支持**右键菜单**和 **文件角标**两个需求的开发接口。

插件是一种在不需要改动并重新编译主程序本身的情况下去扩展主程序功能的一种机制。

文件管理器的插件机制是由文件管理器提供一个扩展库 `libdfm-extension.so` 来提供接口，这是一个纯 C/C++ 接口的库。第三方开发者通过加载该扩展库，按照文管提供的标准来调用和实现相应接口，共同完成定制需求的功能扩展。

在文件管理器启动时，会去检测目录 `/usr/lib/[arch]/dde-file-manager/plugins/extensions` 下的所有文件（其中 [arch] 代表平台架构，如 ARM64 下为 aarch64-linux-gnu，AMD 64下为 x86_64-linux-gnu）， 并检测是否是一个正常的动态库文件，如果是则尝试加载，因此第三方开发者应该将扩展插件**安装到此目录下**。如果加载成功，文管将会检查扩展插件是否实现了相应的**元接口**，如果通过了检查，文管将与第三方扩展插件开始正常业务交互。

@ref group_dfm-extension "接口文档"

# 如何使用模块



# 使用示例



@defgroup dfm-extension

@brief 文件管理器扩展开发库

@details 示例文档:

@subpage dfm-extension

@anchor group_dfm-extension

