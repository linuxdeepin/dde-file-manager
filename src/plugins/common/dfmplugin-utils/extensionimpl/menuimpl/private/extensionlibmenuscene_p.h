// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONLIBMENUSCENE_P_H
#define EXTENSIONLIBMENUSCENE_P_H

#include "dfmplugin_utils_global.h"

#include "dfm-base/interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_utils {

class ExtensionLibMenuScene;
class ExtensionLibMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class ExtensionLibMenuScene;

public:
    explicit ExtensionLibMenuScenePrivate(ExtensionLibMenuScene *qq);

private:
    ExtensionLibMenuScene *q { nullptr };
    bool hiddenActiosn { false };
};

}   // namespace dfmplugin_utils

#endif   // EXTENSIONLIBMENUSCENE_P_H
