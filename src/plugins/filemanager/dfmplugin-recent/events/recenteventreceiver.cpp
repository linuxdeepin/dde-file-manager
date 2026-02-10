// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recenteventreceiver.h"
#include "utils/recentmanager.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/systempathutil.h>

#include <dfm-framework/dpf.h>

#include <QDir>
#include <QTimer>

Q_DECLARE_METATYPE(QDir::Filters);
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_recent;

RecentEventReceiver *RecentEventReceiver::instance()
{
    static RecentEventReceiver ins;
    return &ins;
}

void RecentEventReceiver::handleWindowUrlChanged(quint64 winId, const QUrl &url)
{
    if (url.scheme() == RecentHelper::scheme()) {
        QTimer::singleShot(0, this, [=] {
            QDir::Filters f = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
            dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetFilter", winId, f);
        });
    }
}

void RecentEventReceiver::handleRemoveFilesResult(const QList<QUrl> &urls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    if (ok && !urls.isEmpty()) {
        RecentManager::instance()->reloadRecent();
    }
}

void RecentEventReceiver::handleFileRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(winId)
    Q_UNUSED(errMsg)

    if (!ok || renamedUrls.isEmpty())
        return;
    RecentManager::instance()->reloadRecent();
}

void RecentEventReceiver::handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    Q_UNUSED(srcUrls)

    if (!ok || destUrls.isEmpty())
        return;
    RecentManager::instance()->reloadRecent();
}

bool RecentEventReceiver::customColumnRole(const QUrl &rootUrl, QList<ItemRoles> *roleList)
{
    if (rootUrl.scheme() == RecentHelper::scheme()) {
        roleList->append(kItemFileDisplayNameRole);
        roleList->append(kItemFilePathRole);
        roleList->append(kItemFileLastReadRole);
        roleList->append(kItemFileSizeRole);
        roleList->append(kItemFileMimeTypeRole);

        return true;
    }

    return false;
}

bool RecentEventReceiver::customRoleDisplayName(const QUrl &url, const ItemRoles role, QString *displayName)
{
    if (url.scheme() != RecentHelper::scheme())
        return false;

    if (role == kItemFilePathRole) {
        displayName->append(tr("Path"));
        return true;
    }

    if (role == kItemFileLastReadRole) {
        displayName->append(tr("Last access"));
        return true;
    }

    return false;
}

bool RecentEventReceiver::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (url == RecentHelper::rootUrl()) {
        *iconName = "dfm_recent";
        return true;
    }
    return false;
}

bool RecentEventReceiver::sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);

    if (url.scheme() == RecentHelper::scheme()) {
        QVariantMap map;
        map["CrumbData_Key_Url"] = RecentHelper::rootUrl();
        map["CrumbData_Key_DisplayText"] = tr("Recent");
        map["CrumbData_Key_IconName"] = RecentHelper::icon().name();
        mapGroup->push_back(map);
        return true;
    }

    return false;
}

bool RecentEventReceiver::isTransparent(const QUrl &url, TransparentStatus *status)
{
    if (url.scheme() == RecentHelper::scheme()) {
        *status = TransparentStatus::kUntransparent;
        return true;
    }

    return false;
}

bool RecentEventReceiver::checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    Q_UNUSED(urlTo)
    Q_ASSERT(action);

    if (urls.isEmpty())
        return false;

    if (urls.first().scheme() == RecentHelper::scheme()) {
        *action = Qt::CopyAction;
        return true;
    }

    return false;
}

bool RecentEventReceiver::handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (fromUrls.isEmpty() || !toUrl.isValid())
        return false;

    if (fromUrls.first().scheme() == RecentHelper::scheme() && toUrl.scheme() == "trash") {
        RecentHelper::removeRecent(fromUrls);
        return true;
    }

    return false;
}

bool RecentEventReceiver::handlePropertydialogDisable(const QUrl &url)
{
    if (url != RecentHelper::rootUrl())
        return false;

    return true;
}

RecentEventReceiver::RecentEventReceiver(QObject *parent)
    : QObject(parent)
{
}
