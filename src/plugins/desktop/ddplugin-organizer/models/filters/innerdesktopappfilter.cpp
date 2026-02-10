// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
