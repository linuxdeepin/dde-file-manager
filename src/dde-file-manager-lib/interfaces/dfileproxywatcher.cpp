// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfileproxywatcher.h"
#include "private/dabstractfilewatcher_p.h"
#include "controllers/vaultcontroller.h"

#include <QPointer>

class DFileProxyWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit DFileProxyWatcherPrivate(DFileProxyWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override;
    bool stop() override;

    QPointer<DAbstractFileWatcher> proxy;
    std::function<DUrl(const DUrl &)> urlConvertFun;

    Q_DECLARE_PUBLIC(DFileProxyWatcher)
};

bool DFileProxyWatcherPrivate::start()
{
    return proxy && proxy->startWatcher();
}

bool DFileProxyWatcherPrivate::stop()
{
    return proxy && proxy->stopWatcher();
}

DFileProxyWatcher::DFileProxyWatcher(const DUrl &url, DAbstractFileWatcher *proxy,
                                     std::function<DUrl(const DUrl &)> urlConvertFun,
                                     QObject *parent)
    : DAbstractFileWatcher(*new DFileProxyWatcherPrivate(this), url, parent)
{
    Q_ASSERT(proxy);
    Q_ASSERT(urlConvertFun);
    Q_ASSERT(!proxy->parent());

    proxy->moveToThread(thread());
    proxy->setParent(this);

    d_func()->proxy = proxy;
    d_func()->urlConvertFun = urlConvertFun;

    connect(proxy, &DAbstractFileWatcher::fileAttributeChanged, this, &DFileProxyWatcher::onFileAttributeChanged);
    connect(proxy, &DAbstractFileWatcher::fileDeleted, this, &DFileProxyWatcher::onFileDeleted);
    connect(proxy, &DAbstractFileWatcher::fileMoved, this, &DFileProxyWatcher::onFileMoved);
    connect(proxy, &DAbstractFileWatcher::subfileCreated, this, &DFileProxyWatcher::onSubfileCreated);
    connect(proxy, &DAbstractFileWatcher::fileModified, this, &DFileProxyWatcher::onFileModified);
}

void DFileProxyWatcher::onFileDeleted(const DUrl &url)
{
    Q_D(const DFileProxyWatcher);

    emit fileDeleted(d->urlConvertFun(url));
}

void DFileProxyWatcher::onFileAttributeChanged(const DUrl &url)
{
    Q_D(const DFileProxyWatcher);

    emit fileAttributeChanged(d->urlConvertFun(url));
}

void DFileProxyWatcher::onFileMoved(const DUrl &fromUrl, const DUrl &toUrl)
{
    Q_D(const DFileProxyWatcher);

    emit fileMoved(d->urlConvertFun(fromUrl), d->urlConvertFun(toUrl));
}

void DFileProxyWatcher::onSubfileCreated(const DUrl &url)
{
    Q_D(const DFileProxyWatcher);

    emit subfileCreated(d->urlConvertFun(url));
}

void DFileProxyWatcher::onFileModified(const DUrl &url)
{
    Q_D(const DFileProxyWatcher);

    emit fileModified(d->urlConvertFun(url));
}
