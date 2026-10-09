// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarkcallback.h"
#include "events/bookmarkeventcaller.h"
#include "controller/bookmarkmanager.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-framework/event/event.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-io/dfile.h>

#include <DMenu>

#include <QApplication>
#include <QUrl>
#include <QDebug>

DPBOOKMARK_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void BookmarkCallBack::contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    DMenu *menu = new DMenu;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(menu), AcName::kAcSidebarBookmarkMenu);
#endif
    auto newWindowAct = menu->addAction(QObject::tr("Open in new window"), [url]() { BookMarkEventCaller::sendBookMarkOpenInNewWindow(url); });
    newWindowAct->setEnabled(bEnabled);

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        BookMarkEventCaller::sendBookMarkOpenInNewTab(windowId, url);
    });
    newTabAct->setEnabled(bEnabled && BookMarkEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();

    auto renameAct = menu->addAction(QObject::tr("Rename"), [url, windowId]() {
        QTimer::singleShot(200, [url, windowId] { dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_TriggerEdit", windowId, url); });
    });
    renameAct->setEnabled(bEnabled);

    menu->addAction(QObject::tr("Remove from quick access"),
                    [url]() { BookMarkManager::instance()->removeBookMark(url); });

    menu->addSeparator();

    auto propertyAct = menu->addAction(QObject::tr("Properties"),
                                       [url]() { BookMarkEventCaller::sendShowBookMarkPropertyDialog(url); });
    propertyAct->setEnabled(bEnabled);

    menu->exec(globalPos);
    delete menu;
}

void BookmarkCallBack::renameCallBack(quint64 windowId, const QUrl &url, const QString &name)
{
    Q_UNUSED(windowId);

    if (!BookMarkManager::instance()->bookMarkRename(url, name))
        return;

    QVariantMap map {
        { "Property_Key_DisplayName", name },
        { "Property_Key_Editable", true }
    };
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
}

void BookmarkCallBack::cdBookMarkUrlCallBack(quint64 windowId, const QUrl &url)
{
    QApplication::restoreOverrideCursor();

    const QMap<QUrl, BookmarkData> &bookmarkMap = BookMarkManager::instance()->getBookMarkDataMap();

    if (!bookmarkMap.contains(url)) {
        fmCritical() << "boormark:"
                     << "not find the book mark!";
        return;
    }

    if (NetworkUtils::instance()->checkFtpOrSmbBusy(url)) {
        DialogManager::instance()->showUnableToVistDir(url.path());
        return;
    }

    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (dfmio::DFile(url).exists()) {
        BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
        return;
    } else if (ProtocolUtils::isSMBFile(url) || ProtocolUtils::isFTPFile(url)) {
        auto srcUrl = DeviceUtils::parseNetSourceUrl(url);
        fmInfo() << "bookmark of net file:" << url << "got souce url:" << srcUrl;
        if (srcUrl.isValid()) {
            BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, srcUrl);
            return;
        }
    }
    if (DDialog::Accepted == BookMarkManager::instance()->showRemoveBookMarkDialog(windowId))
        BookMarkManager::instance()->removeBookMark(url);
}

void BookmarkCallBack::cdDefaultItemUrlCallBack(quint64 windowId, const QUrl &url)
{
    BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
}
