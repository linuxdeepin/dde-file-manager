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
#include "trashcorehelper.h"
#include "views/trashpropertydialog.h"

#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/standardpaths.h"

#include <dfm-framework/framework.h>

#include <QDir>
#include <QDirIterator>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trashcore;

bool TrashCoreHelper::isEmpty()
{
    QDir dir(StandardPaths::location(StandardPaths::kTrashFilesPath));

    if (!dir.exists())
        return true;

    dir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

    QDirIterator iterator(dir);

    return !iterator.hasNext();
}

QUrl TrashCoreHelper::rootUrl()
{
    QUrl url;
    url.setScheme(Global::Scheme::kTrash);
    url.setPath("/");
    return url;
}

QUrl TrashCoreHelper::toLocalFile(const QUrl &url)
{
    return QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashFilesPath) + url.path());
}

QUrl TrashCoreHelper::fromTrashFile(const QString &filePath)
{
    QUrl url;

    url.setScheme(TrashCoreHelper::scheme());
    url.setPath(filePath);

    return url;
}

QString TrashCoreHelper::scheme()
{
    return Global::Scheme::kTrash;
}

QWidget *TrashCoreHelper::createTrashPropertyDialog(const QUrl &url)
{
    static TrashPropertyDialog *trashPropertyDialog = nullptr;
    if (UniversalUtils::urlEquals(url, rootUrl()) || FileUtils::isTrashDesktopFile(url)) {
        if (!trashPropertyDialog) {
            trashPropertyDialog = new TrashPropertyDialog();
            return trashPropertyDialog;
        }
        return trashPropertyDialog;
    }
    return nullptr;
}
