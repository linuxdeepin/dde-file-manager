/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "sharewatcher.h"
#include "private/sharewatcher_p.h"
#include "utils/shareutils.h"

#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

ShareWatcher::ShareWatcher(const QUrl &url, QObject *parent)
    : DFMBASE_NAMESPACE::AbstractFileWatcher(new ShareWatcherPrivate(url, this), parent)
{
}

ShareWatcher::~ShareWatcher()
{
}

void ShareWatcher::shareAdded(const QString &path)
{
    auto &&url = ShareUtils::makeShareUrl(path);
    auto info = InfoFactory::create<AbstractFileInfo>(url);
    if (info)
        info->refresh();   // make sure that the cache can be updated if share's name updated.
    Q_EMIT subfileCreated(url);
}

void ShareWatcher::shareRemoved(const QString &path)
{
    Q_EMIT fileDeleted(ShareUtils::makeShareUrl(path));
}

ShareWatcherPrivate::ShareWatcherPrivate(const QUrl &fileUrl, ShareWatcher *qq)
    : DFMBASE_NAMESPACE::AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool ShareWatcherPrivate::start()
{
    auto qp = qobject_cast<ShareWatcher *>(q);
    auto ret = true;
    ret &= dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareAdded", qp, &ShareWatcher::shareAdded);
    ret &= dpfSignalDispatcher->subscribe("dfmplugin_dirshare", "signal_Share_ShareRemoved", qp, &ShareWatcher::shareRemoved);
    return ret;
}

bool ShareWatcherPrivate::stop()
{
    auto qp = qobject_cast<ShareWatcher *>(q);
    auto ret = true;
    ret &= dpfSignalDispatcher->unsubscribe("dfmplugin_dirshare", "signal_Share_ShareAdded", qp, &ShareWatcher::shareAdded);
    ret &= dpfSignalDispatcher->unsubscribe("dfmplugin_dirshare", "signal_Share_ShareRemoved", qp, &ShareWatcher::shareRemoved);
    return ret;
}
