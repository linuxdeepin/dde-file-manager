/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "sharecontroler.h"
#include "models/sharefileinfo.h"
#include "dfileinfo.h"
#include "dabstractfilewatcher.h"
#include "usershare/shareinfo.h"
#include "usershare/usersharemanager.h"
#include "singleton.h"
#include "app/define.h"
#include "dfileservices.h"
#include "private/dabstractfilewatcher_p.h"

class ShareFileWatcherPrivate;
class ShareFileWatcher : public DAbstractFileWatcher
{
public:
    explicit ShareFileWatcher(QObject *parent = 0);

private slots:
    void onUserShareAdded(const QString &filePath);
    void onUserShareDeleted(const QString &filePath);

private:
    Q_DECLARE_PRIVATE(ShareFileWatcher)
};

class ShareFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    ShareFileWatcherPrivate(ShareFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() Q_DECL_OVERRIDE;
    bool stop() Q_DECL_OVERRIDE;

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

    return q->disconnect(userShareManager, 0, q, 0);
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

const DAbstractFileInfoPointer ShareControler::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new ShareFileInfo(event->url()));
}

const QList<DAbstractFileInfoPointer> ShareControler::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    Q_UNUSED(event)

    QList<DAbstractFileInfoPointer> infolist;

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
        return 0;

    return new ShareFileWatcher();
}

bool ShareControler::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    return DFileService::instance()->unShareFolder(event->sender(), realUrl(event->url()));
}

DUrl ShareControler::realUrl(const DUrl &shareUrl)
{
    DUrl ret = shareUrl;
    ret.setScheme(FILE_SCHEME);
    return ret;
}
