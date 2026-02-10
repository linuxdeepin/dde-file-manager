// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCOREHELPER_H
#define TRASHCOREHELPER_H

#include "dfmplugin_trashcore_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QIcon>

namespace dfmplugin_trashcore {

class TrashCoreHelper
{
public:
    inline static QString scheme()
    {
        return DFMBASE_NAMESPACE::Global::Scheme::kTrash;
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("user-trash-symbolic");
    }
    static QUrl rootUrl();
    static QWidget *createTrashPropertyDialog(const QUrl &url);
    static std::pair<qint64, int> calculateTrashRoot();
};

}

#endif   // TRASHCOREHELPER_H
