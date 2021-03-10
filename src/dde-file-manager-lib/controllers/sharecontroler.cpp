/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "sharecontroler.h"
#include "models/sharefileinfo.h"
#include "dfileinfo.h"
#include "dabstractfilewatcher.h"
#include "usershare/shareinfo.h"
#include "usershare/usersharemanager.h"
#include "singleton.h"
#include "app/define.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "private/dabstractfilewatcher_p.h"

class ShareFileWatcherPrivate;
class ShareFileWatcher : public DAbstractFileWatcher
{
public:
    explicit ShareFileWatcher(QObject *parent = nullptr);

private slots:
    void onUserShareAdded(const QString &filePath);
    void onUserShareDeleted(const QString &filePath);

private:
    Q_DECLARE_PRIVATE(ShareFileWatcher)
};

class ShareFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit ShareFileWatcherPrivate(ShareFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override;
    bool stop() override;

    Q_DECLARE_PUBLIC(ShareFileWatcher)
};

ShareFileWatcher::ShareFileWatcher(QObject *parent)
    : DAbstractFileWatcher(*new ShareFileWatcherPrivate(this), DUrl::fromUserShareFile("/"), parent)
{

}

bool ShareFileWatcherPrivate::start()
{
    Q_Q(ShareFileWatcher);

    return q->connect(userShareManager, &UserShareManager::userShareAdded, q, &ShareFileWatcher::onUserShareAdded)
           && q->connect(userShareManager, &UserShareManager::userShareDeleted, q, &ShareFileWatcher::onUserShareDeleted);
}

bool ShareFileWatcherPrivate::stop()
{
    Q_Q(ShareFileWatcher);

    return q->disconnect(userShareManager, nullptr, q, nullptr);
}

void ShareFileWatcher::onUserShareAdded(const QString &filePath)
{
    emit subfileCreated(DUrl::fromUserShareFile(filePath));
}

void ShareFileWatcher::onUserShareDeleted(const QString &filePath)
{
    emit fileDeleted(DUrl::fromUserShareFile(filePath));
}

ShareControler::ShareControler(QObject *parent) :
    DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer ShareControler::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new ShareFileInfo(event->url()));
}

const QList<DAbstractFileInfoPointer> ShareControler::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    Q_UNUSED(event)

    QList<DAbstractFileInfoPointer> infolist;

    userShareManager->updateUserShareInfo(false);

    ShareInfoList sharelist = userShareManager->shareInfoList();
    foreach (ShareInfo shareInfo, sharelist) {
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, DUrl::fromUserShareFile(shareInfo.path()));

        if (fileInfo->exists())
            infolist << fileInfo;
    }

    return infolist;
}

DAbstractFileWatcher *ShareControler::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    if (event->url().path() != "/")
        return nullptr;

    return new ShareFileWatcher();
}

bool ShareControler::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    // 需要将共享URL转换为普通url
    const DUrl &fileUrl = realUrl(event->url());

    if (!fileUrl.isValid())
        return false;

    return DFileService::instance()->openFile(event->sender(), fileUrl);
}

bool ShareControler::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    // 需要将共享URL转换为普通url
    const DUrl &fileUrl = realUrl(event->url());

    if (!fileUrl.isValid())
        return false;

    return DFileService::instance()->setPermissions(event->sender(), fileUrl, event->permissions());
}

bool ShareControler::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
{
    return DFileService::instance()->shareFolder(event->sender(), realUrl(event->url()),
                                                 event->name(), event->isWritable(),
                                                 event->allowGuest());
}

bool ShareControler::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    return DFileService::instance()->unShareFolder(event->sender(), realUrl(event->url()));
}

bool ShareControler::addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const
{
    return DFileService::instance()->addToBookmark(event->sender(), realUrl(event->url()));
}

bool ShareControler::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    return DFileService::instance()->removeBookmark(nullptr, realUrl(event->url()));
}

DUrl ShareControler::realUrl(const DUrl &shareUrl)
{
    DUrl ret = shareUrl;
    ret.setScheme(FILE_SCHEME);
    return ret;
}
