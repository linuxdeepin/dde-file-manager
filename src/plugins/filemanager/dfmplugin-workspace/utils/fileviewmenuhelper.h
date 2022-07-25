/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef FILEVIEWMENUHELPER_H
#define FILEVIEWMENUHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QString>

namespace dfmplugin_workspace {

class FileView;
class FileViewMenuHelper : public QObject
{
    Q_OBJECT
public:
    explicit FileViewMenuHelper(FileView *view = nullptr);

    void showEmptyAreaMenu();
    void showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags);

    void setMenuScene(const QString &scene);

private:
    QString currentMenuScene() const;

    FileView *view { nullptr };
};

}

#endif   // FILEVIEWMENUHELPER_H
