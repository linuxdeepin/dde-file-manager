// SPDX-FileCopyrightText: 2023 - 2026 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "myemblemiconplugin.h"

#include <sys/types.h>
#include <sys/xattr.h>
#include <ulimit.h>

namespace Exapmle {

USING_DFMEXT_NAMESPACE

MyEmblemIconPlugin::MyEmblemIconPlugin()
    : DFMEXT::DFMExtEmblemIconPlugin()
{
    registerLocationEmblemIcons([this](const std::string &filePath, int systemIconCount) {
        return locationEmblemIcons(filePath, systemIconCount);
    });
}

MyEmblemIconPlugin::~MyEmblemIconPlugin()
{
}

DFMExtEmblem MyEmblemIconPlugin::locationEmblemIcons(const std::string &filePath, int systemIconCount) const
{
    DFMExtEmblem emblem;

    // 一个文件的角标最多只有 4 个，当系统角标的数量已经达到 4 个时则无法添加扩展角标了
    // 此外，如果扩展角标添加的位置被系统角标占用，那么扩展角标将无法被显示
    if (systemIconCount >= 4)
        return emblem;

    // 从文件扩展属性中获取右键菜单扩展添加的角标属性
    char buffer[FILENAME_MAX] { 0 };
    ssize_t result = getxattr(filePath.c_str(), "user.icon", buffer, FILENAME_MAX);
    if (result == -1)
        return emblem;

    // 添加角标 icon 到文件图标的左下角
    std::string strBuffer { buffer };
    if (!strBuffer.empty()) {
        std::vector<DFMExtEmblemIconLayout> layouts;
        DFMExtEmblemIconLayout iconLayout { DFMExtEmblemIconLayout::LocationType::BottomLeft, strBuffer };
        layouts.push_back(iconLayout);
        emblem.setEmblem(layouts);
    }

    return emblem;
}

}   // namespace Exapmle
