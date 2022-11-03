/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TRASHCOREHELPER_H
#define TRASHCOREHELPER_H

#include "dfmplugin_trashcore_global.h"

#include "dfm-base/dfm_global_defines.h"

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
};

}

#endif   // TRASHCOREHELPER_H
