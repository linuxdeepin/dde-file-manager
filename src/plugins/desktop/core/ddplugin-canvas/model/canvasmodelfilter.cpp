// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasmodelfilter.h"
#include "utils/fileutil.h"

#include <dfm-base/utils/fileutils.h>

#include <QGSettings>
#include <QDebug>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

CanvasModelFilter::CanvasModelFilter(CanvasProxyModel *m)
    : model(m)
{
}

bool CanvasModelFilter::insertFilter(const QUrl &url)
{
    return false;
}

bool CanvasModelFilter::resetFilter(QList<QUrl> &urls)
{
    return false;
}

bool CanvasModelFilter::updateFilter(const QUrl &url, const QVector<int> &roles)
{
    return false;
}

bool CanvasModelFilter::removeFilter(const QUrl &url)
{
    return false;
}

bool CanvasModelFilter::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    return false;
}

bool HiddenFileFilter::insertFilter(const QUrl &url)
{
    if (model->showHiddenFiles())
        return false;

    if (auto info = FileCreator->createFileInfo(url))
        return info->isAttributes(OptInfoType::kIsHidden);

    return false;
}

bool HiddenFileFilter::resetFilter(QList<QUrl> &urls)
{
    if (model->showHiddenFiles())
        return false;

    for (auto itor = urls.begin(); itor != urls.end();) {
        auto info = FileCreator->createFileInfo(*itor);
        if (info && info->isAttributes(OptInfoType::kIsHidden)) {
            itor = urls.erase(itor);
            continue;
        }
        ++itor;
    }

    return false;
}

bool HiddenFileFilter::updateFilter(const QUrl &url, const QVector<int> &roles)
{
    // the filemanager hidden attr changed.
    // just refresh model if file content changed.
    if (roles.contains(Global::kItemCreateFileInfoRole)) {
        // get file that removed form .hidden if do not show hidden file.
        if (!model->showHiddenFiles() && url.fileName() == ".hidden") {
            qDebug() << "refresh by hidden changed.";
            // do not refresh file info and wait 100ms to let the file atrr changed signal to refresh file
            model->refresh(model->rootIndex(), false, 100, false);
        }
    }

    return false;
}

bool HiddenFileFilter::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(oldUrl)
    return insertFilter(newUrl);
}

InnerDesktopAppFilter::InnerDesktopAppFilter(CanvasProxyModel *model, QObject *parent)
    : QObject(parent), CanvasModelFilter(model)
{

    keys.insert("desktopComputer", DesktopAppUrl::computerDesktopFileUrl());
    keys.insert("desktopTrash", DesktopAppUrl::trashDesktopFileUrl());
    keys.insert("desktopHomeDirectory", DesktopAppUrl::homeDesktopFileUrl());

    hidden.insert("desktopComputer", false);
    hidden.insert("desktopTrash", false);
    hidden.insert("desktopHomeDirectory", false);

    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop")) {
        gsettings = new QGSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
        connect(gsettings, &QGSettings::changed, this, &InnerDesktopAppFilter::changed);
        update();
    }
}

void InnerDesktopAppFilter::update()
{
    if (gsettings) {
        for (auto iter = hidden.begin(); iter != hidden.end(); ++iter) {
            auto var = gsettings->get(iter.key());
            if (var.isValid())
                iter.value() = !var.toBool();
            else
                iter.value() = false;
        }
    }
}

void InnerDesktopAppFilter::refreshModel()
{
    model->refresh(model->rootIndex());
}

bool InnerDesktopAppFilter::resetFilter(QList<QUrl> &urls)
{
    // checking that whether has hidden file.
    if (hidden.key(true, QString()).isEmpty())
        return false;

    // hide the desktop file if gseting set it to off.
    for (auto itor = urls.begin(); itor != urls.end();) {
        if (hidden.value(keys.key(*itor), false)) {
            itor = urls.erase(itor);
        } else {
            ++itor;
        }
    }

    return false;
}

bool InnerDesktopAppFilter::insertFilter(const QUrl &url)
{
    bool ret = hidden.value(keys.key(url), false);
    return ret;
}

bool InnerDesktopAppFilter::renameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    return insertFilter(newUrl);
}

void InnerDesktopAppFilter::changed(const QString &key)
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
