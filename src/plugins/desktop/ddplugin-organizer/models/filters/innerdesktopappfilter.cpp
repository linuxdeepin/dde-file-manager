// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "innerdesktopappfilter.h"

#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

InnerDesktopAppFilter::InnerDesktopAppFilter(QObject *parent)
    : QObject(parent), ModelDataHandler()
{
    keys.insert("desktopComputer", DesktopAppUrl::computerDesktopFileUrl());
    keys.insert("desktopTrash", DesktopAppUrl::trashDesktopFileUrl());
    keys.insert("desktopHomeDirectory", DesktopAppUrl::homeDesktopFileUrl());

    hidden.insert("desktopComputer", false);
    hidden.insert("desktopTrash", false);
    hidden.insert("desktopHomeDirectory", false);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.desktop")) {
        gsettings = new QGSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
        connect(gsettings, &QGSettings::changed, this, &InnerDesktopAppFilter::changed);
        update();
    }
#endif
}

void ddplugin_organizer::InnerDesktopAppFilter::update()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (gsettings) {
        for (auto iter = hidden.begin(); iter != hidden.end(); ++iter) {
            auto var = gsettings->get(iter.key());
            if (var.isValid())
                iter.value() = !var.toBool();
            else
                iter.value() = false;
        }
    }
#endif
}

void InnerDesktopAppFilter::refreshModel()
{
    dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionModel_Refresh", false, 50, false);
}

bool InnerDesktopAppFilter::acceptInsert(const QUrl &url)
{
    bool ret = hidden.value(keys.key(url), false);
    return !ret;
}

QList<QUrl> InnerDesktopAppFilter::acceptReset(const QList<QUrl> &urls)
{
    auto allUrls = urls;
    // hide the desktop file if gseting set it to off.
    for (auto itor = allUrls.begin(); itor != allUrls.end();) {
        if (hidden.value(keys.key(*itor), false)) {
            itor = allUrls.erase(itor);
        } else {
            ++itor;
        }
    }

    return allUrls;
}

bool InnerDesktopAppFilter::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    return acceptInsert(newUrl);
}

void InnerDesktopAppFilter::changed(const QString &key)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
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
#endif
}
