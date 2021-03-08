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
