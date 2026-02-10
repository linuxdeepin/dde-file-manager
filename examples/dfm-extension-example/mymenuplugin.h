// SPDX-FileCopyrightText: 2023 - 2026 ~ 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MYMENUPLUGIN_H
#define MYMENUPLUGIN_H

#include <dfm-extension/menu/dfmextmenuplugin.h>

namespace Exapmle {

class MyMenuPlugin : public DFMEXT::DFMExtMenuPlugin
{
public:
    MyMenuPlugin();
    ~MyMenuPlugin();

    void initialize(DFMEXT::DFMExtMenuProxy *proxy) DFM_FAKE_OVERRIDE;
    bool buildNormalMenu(DFMEXT::DFMExtMenu *main,
                         const std::string &currentPath,
                         const std::string &focusPath,
                         const std::list<std::string> &pathList,
                         bool onDesktop) DFM_FAKE_OVERRIDE;
    bool buildEmptyAreaMenu(DFMEXT::DFMExtMenu *main, const std::string &currentPath, bool onDesktop) DFM_FAKE_OVERRIDE;

private:
    void setEmblemIcon(const std::string &filePath, const std::string &iconName);
    void clearEmblemIcon(const std::string &filePath);
    std::string removeScheme(const std::string &url);

private:
    DFMEXT::DFMExtMenuProxy *m_proxy { nullptr };
};

}   // namespace Exapmle

#endif   // MYMENUPLUGIN_H
