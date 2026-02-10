// SPDX-FileCopyrightText: 2023 - 2026 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MYEMBLEMICONPLUGIN_H
#define MYEMBLEMICONPLUGIN_H

#include <dfm-extension/emblemicon/dfmextemblemiconplugin.h>

namespace Exapmle {

class MyEmblemIconPlugin : public DFMEXT::DFMExtEmblemIconPlugin
{
public:
    MyEmblemIconPlugin();
    ~MyEmblemIconPlugin();

    DFMEXT::DFMExtEmblem locationEmblemIcons(const std::string &filePath, int systemIconCount) const DFM_FAKE_OVERRIDE;
};

}   // namespace Exapmle

#endif   // MYEMBLEMICONPLUGIN_H
