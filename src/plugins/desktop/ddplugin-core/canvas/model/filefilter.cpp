/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "filefilter.h"
#include "canvasmodel.h"
#include "base/standardpaths.h"

#include "utils/fileutils.h"

#include <QGSettings>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DSB_D_USE_NAMESPACE

FileFilter::FileFilter(QAbstractItemModel *_model)
    : model(_model)
{

}

bool FileFilter::fileTraversalFilter(const QUrl &url)
{
    return false;
}

bool FileFilter::fileDeletedFilter(const QUrl &url)
{
    Q_UNUSED(url)
    return false;
}

bool FileFilter::fileCreatedFilter(const QUrl &url)
{
    Q_UNUSED(url)
    return false;
}

bool FileFilter::fileRenameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(oldUrl)
    Q_UNUSED(newUrl)
    return false;
}

bool FileFilter::fileUpdatedFilter(const QUrl &url)
{
    Q_UNUSED(url)
    return false;
}

bool CustomHiddenFilter::fileDeletedFilter(const QUrl &url)
{
    auto info = FileCreator->createFileInfo(url);
    if (info && info->isHidden()) {
        CanvasModel *canvasmodel = qobject_cast<CanvasModel *>(model);
        if (canvasmodel && !canvasmodel->showHiddenFiles())
            return true;
    }

    return false;
}

bool CustomHiddenFilter::fileCreatedFilter(const QUrl &url)
{
    auto info = FileCreator->createFileInfo(url);
    if (info && info->isHidden()) {
        CanvasModel *canvasmodel = qobject_cast<CanvasModel *>(model);
        if (canvasmodel && !canvasmodel->showHiddenFiles())
            return true;
    }

    return false;
}

bool CustomHiddenFilter::fileUpdatedFilter(const QUrl &url)
{
    CanvasModel *canvasmodel = qobject_cast<CanvasModel *>(model);
    // the filemanager hidden attr changed.
    // get file that removed form .hidden if do not show hidden file.
    if (canvasmodel && !canvasmodel->showHiddenFiles() && url.fileName() == ".hidden") {
        qDebug() << "refresh by hidden changed.";
        canvasmodel->refresh(canvasmodel->rootIndex());
        return true;
    }

    return false;
}

InnerDesktopAppController::InnerDesktopAppController(QAbstractItemModel *model, QObject *parent)
    : QObject(parent)
    , FileFilter(model)
{

    keys.insert("desktopComputer", DesktopAppUrl::computerDesktopFileUrl());
    keys.insert("desktopTrash", DesktopAppUrl::trashDesktopFileUrl());
    keys.insert("desktopHomeDirectory", DesktopAppUrl::homeDesktopFileUrl());

    hidden.insert("desktopComputer", false);
    hidden.insert("desktopTrash", false);
    hidden.insert("desktopHomeDirectory", false);

    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop")) {
        gsettings = new QGSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
        connect(gsettings, &QGSettings::changed, this, &InnerDesktopAppController::changed);
        update();
    }
}

void InnerDesktopAppController::update()
{
    if (gsettings) {
        for (auto iter = hidden.begin() ; iter != hidden.end(); ++iter) {
            auto var = gsettings->get(iter.key());
            if (var.isValid())
                iter.value() = !var.toBool();
            else
                iter.value() = false;
        }
    }
}

void InnerDesktopAppController::refreshModel()
{
    if (CanvasModel *canvasmodel = qobject_cast<CanvasModel *>(model))
        canvasmodel->refresh(canvasmodel->rootIndex()); // todo delay refresh
}

bool InnerDesktopAppController::fileTraversalFilter(const QUrl &url)
{
    bool ret = hidden.value(keys.key(url), false);
    return ret;
}

bool InnerDesktopAppController::fileCreatedFilter(const QUrl &url)
{
    bool ret = hidden.value(keys.key(url), false);
    return ret;
}

bool InnerDesktopAppController::fileRenameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (!keys.key(oldUrl).isEmpty()|| !keys.key(newUrl).isEmpty()) {
        refreshModel();
        return true;
    }

    return false;
}

void InnerDesktopAppController::changed(const QString &key)
{
    if (hidden.contains(key)) {
        auto var = gsettings->get(key);
        bool old = hidden.value(key);
        if (var.isValid())
            hidden[key] = !var.toBool();
        else
            hidden[key] = false;

       if (old != hidden.value(key))
           refreshModel();
    }
}
