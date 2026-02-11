// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOCOLDISPLAYUTILITIES_H
#define PROTOCOLDISPLAYUTILITIES_H

#include "dfmplugin_smbbrowser_global.h"

#include <QUrl>
#include <QStringList>
#include <QPoint>

DPSMBBROWSER_BEGIN_NAMESPACE

namespace protocol_display_utilities {

QUrl makeVEntryUrl(const QString &standardSmb);
QStringList getMountedSmb();
QStringList getStandardSmbPaths(const QStringList &devIds);
QString getSmbHostPath(const QString &devId);
QString getStandardSmbPath(const QUrl &entryUrl);
QString getStandardSmbPath(const QString &devId);
QString getDisplayNameOf(const QString &devId);
QString getDisplayNameOf(const QUrl &entryUrl);
bool hasMountedShareOf(const QString &stdHost);

}   // namespace protocol_display_utilities

namespace computer_sidebar_event_calls {

void callItemAdd(const QUrl &vEntryUrl);
void callItemRemove(const QUrl &vEntryUrl);
void callComputerRefresh();
void callForgetPasswd(const QString &stdSmb);

void sidebarMenuCall(quint64 winId, const QUrl &url, const QPoint &pos);
void sidebarItemClicked(quint64 winId, const QUrl &url);
void sidebarItemRename(quint64 windowId, const QUrl &url, const QString &name);
bool sidebarUrlEquals(const QUrl &item, const QUrl &target);

}   // namespace computer_sidebar_event_calls

namespace secret_utils {

void forgetPasswordInSession(const QString &host);

}   //namespace secret_utils

namespace ui_ventry_calls {
void addAggregatedItemForSeperatedOnlineItem(const QUrl &entryUrl);
void addAggregatedItems();
void addSeperatedOfflineItems();
}   // namespace ui_ventry_calls

DPSMBBROWSER_END_NAMESPACE

#endif   // PROTOCOLDISPLAYUTILITIES_H
