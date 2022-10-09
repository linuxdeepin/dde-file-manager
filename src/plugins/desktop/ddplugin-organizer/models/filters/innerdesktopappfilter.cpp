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
#include "innerdesktopappfilter.h"

#include <dfm-base/utils/fileutils.h>

#include <dpf.h>

#include <QGSettings>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

InnerDesktopAppFilter::InnerDesktopAppFilter(QObject *parent) : QObject(parent), ModelDataHandler()
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

void ddplugin_organizer::InnerDesktopAppFilter::update()
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

void InnerDesktopAppFilter::refreshModel()
{
    dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionModel_Refresh", false, 50);
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
