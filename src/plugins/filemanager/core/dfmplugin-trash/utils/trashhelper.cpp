/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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

#include "trashhelper.h"
#include "trashfileinfo.h"
#include "trashfilewatcher.h"
#include "events/trasheventcaller.h"
#include "views/emptyTrashWidget.h"
#include "dfm_event_defines.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/dpf.h>

#include <dfm-io/dfmio_utils.h>

#include <DHorizontalLine>
#include <DApplicationHelper>

#include <QFileInfo>
#include <QFile>
#include <QMenu>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

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
        QUrl localUrl = TrashHelper::toLocalFile(url);
        TrashEventCaller::sendEmptyTrash(windowId, { localUrl });
    });
    emptyTrashAct->setDisabled(TrashHelper::isEmpty());

    menu->addSeparator();

    menu->addAction(QObject::tr("Properties"), [url]() {
        TrashEventCaller::sendTrashPropertyDialog(url);
    });
    menu->exec(globalPos);
    delete menu;
}

QFrame *TrashHelper::createEmptyTrashTopWidget()
{
    EmptyTrashWidget *emptyTrashWidget = new EmptyTrashWidget;
    QObject::connect(emptyTrashWidget, &EmptyTrashWidget::emptyTrash,

                     TrashHelper::instance(), [emptyTrashWidget] {
                         auto windId = TrashHelper::instance()->windowId(emptyTrashWidget);
                         TrashHelper::emptyTrash(windId);
                     });
    return emptyTrashWidget;
}

bool TrashHelper::showTopWidget(QWidget *w, const QUrl &url)
{
    Q_UNUSED(w)

    auto rootUrl = TrashHelper::fromTrashFile("/");
    if (UniversalUtils::urlEquals(url, rootUrl) && !TrashHelper::isEmpty()) {
        return true;
    } else {
        return false;
    }
}

QUrl TrashHelper::fromTrashFile(const QString &filePath)
{
    QUrl url;

    url.setScheme(TrashHelper::scheme());
    url.setPath(filePath);

    return url;
}

QUrl TrashHelper::fromLocalFile(const QUrl &url)
{
    if (url.scheme() == Global::Scheme::kFile && url.path().startsWith(StandardPaths::location(StandardPaths::kTrashFilesPath))) {
        return TrashHelper::fromTrashFile(url.path().remove(StandardPaths::location(StandardPaths::kTrashFilesPath)));
    }
    return url;
}

QUrl TrashHelper::toLocalFile(const QUrl &url)
{
    return QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashFilesPath) + url.path());
}

bool TrashHelper::isEmpty()
{
    QDir dir(StandardPaths::location(StandardPaths::kTrashFilesPath));

    if (!dir.exists())
        return true;

    dir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

    QDirIterator iterator(dir);

    return !iterator.hasNext();
}

void TrashHelper::emptyTrash(const quint64 windowId)
{
    dpfSlotChannel->push("dfmplugin_trashcore", "slot_TrashCore_EmptyTrash", windowId);
}

TrashHelper::ExpandFieldMap TrashHelper::propetyExtensionFunc(const QUrl &url)
{
    const auto &info = InfoFactory::create<AbstractFileInfo>(url);

    ExpandFieldMap map;
    {
        // source path
        BasicExpand expand;
        const QString &sourcePath = info->redirectedFileUrl().path();
        expand.insert("kFileModifiedTime", qMakePair(QObject::tr("Source path"), sourcePath));
        map["kFieldInsert"] = expand;
    }
    {
        // trans trash path
        BasicExpand expand;
        expand.insert("kFilePosition", qMakePair(QObject::tr("Location"), TrashHelper::toLocalFile(url).path()));
        map["kFieldReplace"] = expand;
    }

    return map;
}

JobHandlePointer TrashHelper::restoreFromTrashHandle(const quint64 windowId, const QList<QUrl> urls, const AbstractJobHandler::JobFlags flags)
{
    QList<QUrl> urlsLocal;
    for (const auto &url : urls) {
        if (url.scheme() == TrashHelper::scheme())
            urlsLocal.append(TrashHelper::toLocalFile(url));
    }

    dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrash,
                                 windowId,
                                 urlsLocal,
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

    QUrl urlFrom = urls.first();
    QUrl urlToTemp = urlTo;
    // restore url to trash
    urlFrom = fromLocalFile(urlFrom);
    urlToTemp = fromLocalFile(urlTo);

    if (urlFrom.scheme() == Global::Scheme::kTrash && urlToTemp.scheme() == Global::Scheme::kTrash) {
        *action = Qt::IgnoreAction;
        return true;
    } else if (urlToTemp.scheme() == Global::Scheme::kTrash && !UniversalUtils::urlEquals(urlToTemp, TrashHelper::rootUrl())) {
        *action = Qt::IgnoreAction;
        return true;
    } else if (urlFrom.scheme() == Global::Scheme::kTrash || urlToTemp.scheme() == Global::Scheme::kTrash) {
        *action = Qt::MoveAction;
        return true;
    }
    return false;
}

bool TrashHelper::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (url == rootUrl()) {
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

bool TrashHelper::customRoleData(const QUrl &rootUrl, const QUrl &url, const Global::ItemRoles role, QVariant *data)
{
    Q_UNUSED(rootUrl)

    if (url.scheme() != scheme())
        return false;

    if (role == kItemFileOriginalPath) {
        QSharedPointer<TrashFileInfo> info = InfoFactory::create<TrashFileInfo>(url);
        if (info) {
            data->setValue(info->redirectedFileUrl().path());
            return true;
        }
    }

    if (role == kItemFileDeletionDate) {
        QSharedPointer<TrashFileInfo> info = InfoFactory::create<TrashFileInfo>(url);
        if (info) {
            data->setValue(info->fileTime(QFileDevice::FileAccessTime).toString(FileUtils::dateTimeFormat()));
            return true;
        }
    }

    return false;
}

bool TrashHelper::urlsToLocal(const QList<QUrl> &origins, QList<QUrl> *urls)
{
    if (!urls)
        return false;
    for (const QUrl &url : origins) {
        if (url.scheme() != TrashHelper::scheme())
            return false;
        (*urls).push_back(toLocalFile(url));
    }
    return true;
}

void TrashHelper::onTrashStateChanged()
{
    if (isEmpty() == isTrashEmpty)
        return;

    isTrashEmpty = isEmpty();

    const QList<quint64> &windowIds = FMWindowsIns.windowIdList();
    for (const quint64 winId : windowIds) {
        auto window = FMWindowsIns.findWindowById(winId);
        if (window) {
            const QUrl &url = window->currentUrl();
            if (url.scheme() == scheme())
                TrashEventCaller::sendShowEmptyTrash(winId, !isTrashEmpty);
        }
    }
}

TrashHelper::TrashHelper(QObject *parent)
    : QObject(parent),
      isTrashEmpty(isEmpty())
{
    initEvent();
}

void TrashHelper::initEvent()
{
    bool resutl = dpfSignalDispatcher->subscribe("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged", this, &TrashHelper::onTrashStateChanged);
    if (!resutl)
        qWarning() << "subscribe signal_TrashCore_TrashStateChanged from dfmplugin_trashcore is failed.";
}
