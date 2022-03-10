/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             lvwujun<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#include "usershareservice.h"
#include "usershare_defines.h"
#include "private/usersharehelper.h"

#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/framework.h>

#include <QFile>
#include <QFutureWatcher>

DSC_BEGIN_NAMESPACE

#define dispatcher dpfInstance.eventDispatcher()

namespace EventType {
const int kRemoveShare = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}

UserShareService::UserShareService(QObject *parent)
    : dpf::PluginService(parent), dpf::AutoServiceRegister<UserShareService>()
{
    initConnect();
    initEventHandlers();
}

UserShareService::~UserShareService()
{
}

void UserShareService::initConnect()
{
    connect(UserShareHelperInstance, &UserShareHelper::shareAdded, this, &UserShareService::shareAdded);
    connect(UserShareHelperInstance, &UserShareHelper::shareRemoved, this, &UserShareService::shareRemoved);
    connect(UserShareHelperInstance, &UserShareHelper::shareCountChanged, this, &UserShareService::shareCountChanged);
    connect(UserShareHelperInstance, &UserShareHelper::shareRemoveFailed, this, &UserShareService::shareRemoveFailed);
}

void UserShareService::initEventHandlers()
{
    dispatcher.subscribe(EventType::kRemoveShare, UserShareHelperInstance, &UserShareHelper::removeShareByPath);
}

UserShareService *UserShareService::service()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(name()))
            abort();
    });

    return ctx.service<UserShareService>(name());
}

void UserShareService::startSambaService(StartSambaFinished onFinished)
{
    UserShareHelperInstance->startSambaServiceAsync(onFinished);
}

bool UserShareService::isSambaServiceRunning()
{
    return UserShareHelperInstance->isSambaServiceRunning();
}

void UserShareService::setSharePassword(const QString &userName, const QString &passwd)
{
    UserShareHelperInstance->setSambaPasswd(userName, passwd);
}

bool UserShareService::addShare(const ShareInfo &share)
{
    return UserShareHelperInstance->share(share);
}

void UserShareService::removeShare(const QString &path)
{
    UserShareHelperInstance->removeShareByPath(path);
}

bool UserShareService::isSharedPath(const QString &path)
{
    return UserShareHelperInstance->isShared(path);
}

ShareInfoList UserShareService::shareInfos()
{
    return UserShareHelperInstance->shareInfos();
}

ShareInfo UserShareService::getInfoByPath(const QString &path)
{
    return UserShareHelperInstance->getShareInfoByPath(path);
}

ShareInfo UserShareService::getInfoByName(const QString &shareName)
{
    return UserShareHelperInstance->getShareInfoByShareName(shareName);
}

QString UserShareService::getShareNameByPath(const QString &path)
{
    return UserShareHelperInstance->getShareNameByPath(path);
}

QString UserShareService::getCurrentUserName()
{
    return UserShareHelperInstance->getCurrentUserName();
}

uint UserShareService::getUidByShareName(const QString &name)
{
    return UserShareHelperInstance->getUidByShareName(name);
}

void ShareInfo::setShareName(const QString &value)
{
    shareName = value;
}

void ShareInfo::setPath(const QString &value)
{
    path = value;
}

void ShareInfo::setComment(const QString &value)
{
    comment = value;
    if (comment.isEmpty())
        comment = "\"\"";
}

void ShareInfo::setUserShareAcl(const QString &value)
{
    userShareAcl = value;
    if (userShareAcl.isEmpty())
        userShareAcl = "\"\"";
    writable = !userShareAcl.contains("Everyone:R");
}

void ShareInfo::setGuestEnable(const QString &value)
{
    guestEnable = value;
}

void ShareInfo::setWritable(bool value)
{
    writable = value;
    userShareAcl = writable ? "Everyone:f" : "Everyone:r";
}

void ShareInfo::setAnonymous(bool value)
{
    anonymous = value;
    guestEnable = anonymous ? "guest_ok=y" : "guest_ok=n";
}

ShareInfo::ShareInfo(const QString &name, const QString &path, const QString &comment, bool writable, bool anonymous)
{
    setShareName(name);
    setPath(path);
    setComment(comment);
    setWritable(writable);
    setAnonymous(anonymous);
}

bool ShareInfo::isValid() const
{
    return !shareName.isEmpty() && QFile(path).exists();
}

QDebug operator<<(QDebug dbg, const ShareInfo &obj)
{
    dbg.nospace() << "{";
    dbg.nospace() << "shareName:" << obj.getShareName() << ",";
    dbg.nospace() << "path:" << obj.getPath() << ",";
    dbg.nospace() << "comment:" << obj.getComment() << ",";
    dbg.nospace() << "usershare_acl:" << obj.getUserShareAcl() << ",";
    dbg.nospace() << "guest_ok:" << obj.getGuestEnable() << ",";
    dbg.nospace() << "}";
    return dbg;
}
DSC_END_NAMESPACE
