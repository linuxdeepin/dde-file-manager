/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef FILEOPERATORHELPER_H
#define FILEOPERATORHELPER_H

#include "dfmplugin_sidebar_global.h"
#include "dfm_global_defines.h"

#include <QObject>
#include <QUrl>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarView;
class FileOperatorHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperatorHelper)
public:
    static FileOperatorHelper *instance();

    void pasteFiles(quint64 windowId, const QList<QUrl> &srcUrls, const QUrl &targetUrl, const Qt::DropAction &action);

private:
    explicit FileOperatorHelper(QObject *parent = nullptr);
};

#define FileOperatorHelperIns DPSIDEBAR_USE_NAMESPACE::FileOperatorHelper::instance()

DPSIDEBAR_END_NAMESPACE

#endif   // FILEOPERATORHELPER_H
