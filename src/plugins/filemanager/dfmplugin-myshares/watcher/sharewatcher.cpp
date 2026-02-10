// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharewatcher.h"
#include "private/sharewatcher_p.h"
#include "utils/shareutils.h"

#include <dfm-base/base/schemefactory.h>

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
    auto info = InfoFactory::create<FileInfo>(url);
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
