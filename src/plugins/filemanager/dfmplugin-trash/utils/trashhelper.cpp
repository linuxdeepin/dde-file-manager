// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashhelper.h"
#include "trashfilewatcher.h"
#include "events/trasheventcaller.h"
#include "views/emptyTrashWidget.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <dfm-io/dfmio_utils.h>

#include <DHorizontalLine>

#include <QFileInfo>
#include <QFile>
#include <QMenu>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <unistd.h>

using namespace dfmplugin_trash;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

TrashHelper *TrashHelper::instance()
{
    static TrashHelper instance;
    return &instance;
}

QUrl TrashHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath("/");
    url.setHost("");
    return url;
}

quint64 TrashHelper::windowId(QWidget *sender)
{
    return FMWindowsIns.findWindowId(sender);
}

void TrashHelper::contenxtMenuHandle(const quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QMenu *menu = new QMenu;
    menu->addAction(QObject::tr("Open in new window"), [url]() {
        TrashEventCaller::sendOpenWindow(url);
    });

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        TrashEventCaller::sendOpenTab(windowId, url);
    });

    newTabAct->setDisabled(!TrashEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();

    auto emptyTrashAct = menu->addAction(QObject::tr("Empty Trash"), [windowId, url]() {
        TrashEventCaller::sendEmptyTrash(windowId, {});
    });
    emptyTrashAct->setDisabled(FileUtils::trashIsEmpty());

    menu->addSeparator();

    menu->addAction(QObject::tr("Properties"), [url]() {
        TrashEventCaller::sendTrashPropertyDialog(url);
    });
    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_trash", "signal_ReportLog_MenuData", act->text(), urls);
    }
    delete menu;
}

QFrame *TrashHelper::createEmptyTrashTopWidget()
{
    EmptyTrashWidget *emptyTrashWidget = new EmptyTrashWidget;
    QObject::connect(emptyTrashWidget, &EmptyTrashWidget::emptyTrash, TrashHelper::instance(), [emptyTrashWidget] {
        auto windId = TrashHelper::instance()->windowId(emptyTrashWidget);
        fmInfo() << "Trash: Empty trash triggered from widget for window:" << windId;
        TrashHelper::emptyTrash(windId);
    });
    return emptyTrashWidget;
}

bool TrashHelper::showTopWidget(QWidget *w, const QUrl &url)
{
    Q_UNUSED(w)

    return false;
}

QUrl TrashHelper::transToTrashFile(const QString &filePath)
{
    QUrl url;
    url.setScheme(TrashHelper::scheme());
    url.setPath(filePath);
    return url;
}

QUrl TrashHelper::trashFileToTargetUrl(const QUrl &url)
{
    auto fileInfo = InfoFactory::create<FileInfo>(url);
    if (fileInfo)
        return fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);

    return url;
}

bool TrashHelper::isTrashFile(const QUrl &url)
{
    if (url.scheme() == TrashHelper::scheme())
        return true;
    if (url.path().startsWith(StandardPaths::location(StandardPaths::kTrashLocalFilesPath)))
        return true;

    const QString &rule = QString("/.Trash-%1/(files|info)/").arg(getuid());
    QRegularExpression reg(rule);
    QRegularExpressionMatch matcher = reg.match(url.toString());
    return matcher.hasMatch();
}

bool TrashHelper::isTrashRootFile(const QUrl &url)
{
    if (UniversalUtils::urlEquals(url, TrashHelper::rootUrl()))
        return true;
    if (url.path().endsWith(StandardPaths::location(StandardPaths::kTrashLocalFilesPath)))
        return true;

    const QString &rule = QString("/.Trash-%1/(files|info)$").arg(getuid());
    QRegularExpression reg(rule);
    QRegularExpressionMatch matcher = reg.match(url.toString());
    return matcher.hasMatch();
}

void TrashHelper::emptyTrash(const quint64 windowId)
{
    dpfSlotChannel->push("dfmplugin_trashcore", "slot_TrashCore_EmptyTrash", windowId);
}

TrashHelper::ExpandFieldMap TrashHelper::propetyExtensionFunc(const QUrl &url)
{
    const auto &info = InfoFactory::create<FileInfo>(url);

    ExpandFieldMap map;
    {
        // source path
        BasicExpand expand;
        const QString &sourcePath = info->urlOf(UrlInfoType::kOriginalUrl).path();
        expand.insert("kFileModifiedTime", qMakePair(QObject::tr("Source path"), sourcePath));
        map["kFieldInsert"] = expand;
    }
    {
        // trans trash path
        BasicExpand expand;
        const QString &targetPath = info->urlOf(UrlInfoType::kRedirectedFileUrl).path();
        expand.insert("kFilePosition", qMakePair(QObject::tr("Location"), targetPath));
        map["kFieldReplace"] = expand;
    }

    return map;
}

TrashHelper::ExpandFieldMap TrashHelper::detailExtensionFunc(const QUrl &url)
{
    const auto &info = InfoFactory::create<FileInfo>(url);

    ExpandFieldMap map;
    {
        // source path
        BasicExpand expand;
        const QString &sourcePath = info->urlOf(UrlInfoType::kOriginalUrl).path();
        expand.insert("kFileChangeTIme", qMakePair(QObject::tr("Source path"), sourcePath));
        map["kFieldInsert"] = expand;
    }

    return map;
}

JobHandlePointer TrashHelper::restoreFromTrashHandle(const quint64 windowId, const QList<QUrl> urls, const AbstractJobHandler::JobFlags flags)
{
    dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrash,
                                 windowId,
                                 urls,
                                 QUrl(),
                                 flags, nullptr);
    return {};
}

bool TrashHelper::checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    if (urls.isEmpty())
        return false;
    if (!urlTo.isValid())
        return false;
    if (!action)
        return false;

    const bool fromIsTrash = isTrashFile(urls.first());
    const bool toIsTrash = isTrashFile(urlTo);
    const bool toIsTrashRoot = isTrashRootFile(urlTo);

    if (fromIsTrash && toIsTrash) {
        *action = Qt::IgnoreAction;
        return true;
    } else if (toIsTrash && !toIsTrashRoot) {
        *action = Qt::IgnoreAction;
        return true;
    } else if (fromIsTrash || toIsTrash) {
        *action = Qt::MoveAction;
        return true;
    }
    return false;
}

bool TrashHelper::checkCanMove(const QUrl &url)
{
    if (url.scheme() != scheme())
        return false;
    if (!FileUtils::isTrashRootFile(UrlRoute::urlParent(url)))
        return false;

    return true;
}

bool TrashHelper::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (UniversalUtils::urlEquals(url, rootUrl())) {
        *iconName = SystemPathUtil::instance()->systemPathIconName("Trash");
        if (!iconName->isEmpty())
            return true;
    }
    return false;
}

bool TrashHelper::customColumnRole(const QUrl &rootUrl, QList<Global::ItemRoles> *roleList)
{
    if (rootUrl.scheme() == scheme()) {
        roleList->append(kItemFileDisplayNameRole);
        roleList->append(kItemFileOriginalPath);
        roleList->append(kItemFileDeletionDate);
        roleList->append(kItemFileSizeRole);
        roleList->append(kItemFileMimeTypeRole);

        return true;
    }

    return false;
}

bool TrashHelper::customRoleDisplayName(const QUrl &url, const Global::ItemRoles role, QString *displayName)
{
    if (url.scheme() != scheme())
        return false;

    if (role == kItemFileOriginalPath) {
        displayName->append(tr("Source Path"));
        return true;
    }

    if (role == kItemFileDeletionDate) {
        displayName->append(tr("Time deleted"));
        return true;
    }

    return false;
}

void TrashHelper::onTrashStateChanged()
{
    bool actuallyEmpty = FileUtils::trashIsEmpty();
    trashState = actuallyEmpty ? TrashState::Empty : TrashState::NotEmpty;

    // When trash becomes non-empty, update UI
    const QList<quint64> &windowIds = FMWindowsIns.windowIdList();
    for (const quint64 winId : windowIds) {
        auto window = FMWindowsIns.findWindowById(winId);
        if (window) {
            const QUrl &url = window->currentUrl();
            if (url.scheme() == scheme()) {
                // !isTrashEmpty was used in original, so when not empty, pass true
                bool showNotEmpty = (trashState == TrashState::NotEmpty);
                TrashEventCaller::sendShowEmptyTrash(winId, showNotEmpty);
            }
        }
    }
    fmDebug() << "Trash: State changed to non-empty, UI updated";
}

void TrashHelper::onTrashEmptyState()
{
    // Force refresh the actual state
    bool actuallyEmpty = FileUtils::trashIsEmpty();
    trashState = actuallyEmpty ? TrashState::Empty : TrashState::NotEmpty;

    if (trashState != TrashState::Empty) {
        fmDebug() << "Trash: Trash is not empty, no action needed";
        return;
    }

    const QList<quint64> &windowIds = FMWindowsIns.windowIdList();
    for (const quint64 winId : windowIds) {
        auto window = FMWindowsIns.findWindowById(winId);
        if (window) {
            const QUrl &url = window->currentUrl();
            if (url.scheme() == scheme())
                TrashEventCaller::sendShowEmptyTrash(winId, false);   // false means empty
        }
    }
    fmDebug() << "Trash: State updated to empty, UI refreshed";
}

void TrashHelper::trashNotEmpty()
{
    emit trashNotEmptyState();
}

void TrashHelper::handleWindowUrlChanged(quint64 winId, const QUrl &url)
{
    // url切换时，更新回收站顶部控件显示状态，主要针对标签切换
    if (url.scheme() == scheme() && trashState != TrashState::Unknown)
        TrashEventCaller::sendShowEmptyTrash(winId, trashState == TrashState::NotEmpty);
}

void TrashHelper::onTrashNotEmptyState()
{
    trashState = TrashState::NotEmpty;
    const QList<quint64> &windowIds = FMWindowsIns.windowIdList();
    for (const quint64 winId : windowIds) {
        auto window = FMWindowsIns.findWindowById(winId);
        if (window) {
            const QUrl &url = window->currentUrl();
            if (url.scheme() == scheme())
                TrashEventCaller::sendShowEmptyTrash(winId, true);   // true means not empty
        }
    }
    fmDebug() << "Trash: State explicitly set to non-empty";
}

TrashHelper::TrashHelper(QObject *parent)
    : QObject(parent)
{
    // Remove blocking FileUtils::trashIsEmpty() call from constructor
    // State will be lazily initialized when needed
    initEvent();
}

void TrashHelper::initEvent()
{
    bool resutl = dpfSignalDispatcher->subscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged", this, &TrashHelper::onTrashStateChanged);
    if (!resutl)
        fmWarning() << "subscribe signal_TrashCore_TrashStateChanged from dfmplugin_trashcore is failed.";
    connect(this, &TrashHelper::trashNotEmptyState, this, &TrashHelper::onTrashNotEmptyState, Qt::QueuedConnection);
}
