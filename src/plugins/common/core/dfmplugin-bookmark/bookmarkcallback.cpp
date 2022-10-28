#include "bookmarkcallback.h"
#include "events/bookmarkeventcaller.h"
#include "controller/bookmarkmanager.h"

#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/dfm_global_defines.h"

#include <QApplication>
#include <QUrl>
#include <QFileInfo>
#include <QMenu>
#include <QDebug>

DPBOOKMARK_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void BookmarkCallBack::contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    QMenu *menu = new QMenu;
    auto newWindowAct = menu->addAction(QObject::tr("Open in new window"), [url]() { BookMarkEventCaller::sendBookMarkOpenInNewWindow(url); });
    newWindowAct->setEnabled(bEnabled);

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        BookMarkEventCaller::sendBookMarkOpenInNewTab(windowId, url);
    });
    newTabAct->setEnabled(bEnabled && BookMarkEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();

    auto renameAct = menu->addAction(QObject::tr("Rename"), [url, windowId]() {
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_TriggerEdit", windowId, url);
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
        qCritical() << "boormark:"
                    << "not find the book mark!";
        return;
    }

    if (bookmarkMap[url].deviceUrl.startsWith(Global::Scheme::kSmb)
        || bookmarkMap[url].deviceUrl.startsWith(Global::Scheme::kFtp)
        || bookmarkMap[url].deviceUrl.startsWith(Global::Scheme::kSFtp)) {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url, false);
        if (info && info->exists()) {
            if (info->isDir())
                BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
        } else {
            DeviceManager::instance()->mountNetworkDeviceAsync(bookmarkMap[url].deviceUrl, [windowId, url](bool ok, DFMMOUNT::DeviceError err, const QString &mntPath) {
                if (!ok && err != dfmmount::DeviceError::kGIOErrorAlreadyMounted) {
                    DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kMount, err);
                } else {
                    BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
                }
            });
        }
        return;
    }

    QFileInfo info(url.path());
    if (info.exists() && info.isDir()) {
        BookMarkEventCaller::sendOpenBookMarkInWindow(windowId, url);
    } else {
        if (DDialog::Accepted == BookMarkManager::instance()->showRemoveBookMarkDialog(windowId))
            BookMarkManager::instance()->removeBookMark(url);
    }
}

void BookmarkCallBack::cdDefaultItemUrlCallBack(quint64 windowId, const QUrl &url)
{
    BookMarkEventCaller::sendDefaultItemActived(windowId, url);
}

void bookMarkActionClickedCallBack(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected)
{
    Q_UNUSED(isNormal);
    Q_UNUSED(currentUrl);
    Q_UNUSED(focusFile);

    if (selected.size() != 1)
        return;

    QUrl url = selected.at(0);
    if (!QFileInfo(url.path()).isDir())
        return;

    if (BookMarkManager::instance()->getBookMarkDataMap().contains(url))
        BookMarkManager::instance()->removeBookMark(url);
    else
        BookMarkManager::instance()->addBookMark(selected);
}
