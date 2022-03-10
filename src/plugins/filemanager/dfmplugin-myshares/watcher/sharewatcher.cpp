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

#include "services/common/usershare/usershareservice.h"

DPMYSHARES_USE_NAMESPACE
DSC_USE_NAMESPACE

ShareWatcher::ShareWatcher(const QUrl &url, QObject *parent)
    : dfmbase::AbstractFileWatcher(new ShareWatcherPrivate(url, this), parent)
{
}

ShareWatcher::~ShareWatcher()
{
}

ShareWatcherPrivate::ShareWatcherPrivate(const QUrl &fileUrl, ShareWatcher *qq)
    : dfmbase::AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool ShareWatcherPrivate::start()
{
    return q->connect(UserShareService::service(), &UserShareService::shareAdded, q, [this](const QString &path) { Q_EMIT q->subfileCreated(ShareUtils::makeShareUrl(path)); })
            && q->connect(UserShareService::service(), &UserShareService::shareRemoved, q, [this](const QString &path) { Q_EMIT q->fileDeleted(ShareUtils::makeShareUrl(path)); });
}

bool ShareWatcherPrivate::stop()
{
    return q->disconnect(UserShareService::service(), nullptr, q, nullptr);
}
