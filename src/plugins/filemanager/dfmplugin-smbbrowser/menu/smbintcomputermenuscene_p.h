// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBINTCOMPUTERMENUSCENE_P_H
#define SMBINTCOMPUTERMENUSCENE_P_H

#include "dfmplugin_smbbrowser_global.h"

#include "dfm-base/interfaces/private/abstractmenuscene_p.h"
#include "dfm-base/file/entry/entryfileinfo.h"

namespace dfmplugin_smbbrowser {

class SmbIntComputerMenuScene;
class SmbIntComputerMenuScenePrivate : DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class SmbIntComputerMenuScene;
    DFMEntryFileInfoPointer info { nullptr };

public:
    explicit SmbIntComputerMenuScenePrivate(SmbIntComputerMenuScene *qq);
};

}

#endif   // SMBINTCOMPUTERMENUSCENE_P_H
