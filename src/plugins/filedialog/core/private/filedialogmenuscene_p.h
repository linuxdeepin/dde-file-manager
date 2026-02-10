// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGMENUSCENE_P_H
#define FILEDIALOGMENUSCENE_P_H

#include "filedialogplugin_core_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace filedialog_core {

class FileDialogMenuScene;
class FileDialogMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class FileDialogMenuScene;
public:
    explicit FileDialogMenuScenePrivate(FileDialogMenuScene *qq);

private:
    FileDialogMenuScene *q { nullptr };
};

}

#endif   // FILEDIALOGMENUSCENE_P_H
