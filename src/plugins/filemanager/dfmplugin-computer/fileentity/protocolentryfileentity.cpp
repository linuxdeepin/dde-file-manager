/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "dfm_global_defines.h"
#include "protocolentryfileentity.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"

#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/dfm_global_defines.h"

#include <QMenu>

DPCOMPUTER_USE_NAMESPACE

using namespace GlobalServerDefines;

/*!
 * \class ProtocolEntryFileEntity
 * \brief class that present protocol devices
 */
ProtocolEntryFileEntity::ProtocolEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    if (!url.path().endsWith(SuffixInfo::kProtocol)) {
        qWarning() << "wrong suffix in" << __FUNCTION__ << "url";
        abort();
    }

    refresh();
}

QString ProtocolEntryFileEntity::displayName() const
{
    return datas.value(DeviceProperty::kDisplayName).toString();
}

QIcon ProtocolEntryFileEntity::icon() const
{
    auto icons = datas.value(DeviceProperty::kDeviceIcon).toStringList();
    for (const auto &icon : icons) {
        auto iconObj = QIcon::fromTheme(icon);
        if (iconObj.isNull())
            continue;
        return iconObj;
    }
    return {};
}

bool ProtocolEntryFileEntity::exists() const
{
    return !datas.value(DeviceProperty::kMountPoint).toString().isEmpty();
}

bool ProtocolEntryFileEntity::showProgress() const
{
    return true;
}

bool ProtocolEntryFileEntity::showTotalSize() const
{
    return true;
}

bool ProtocolEntryFileEntity::showUsageSize() const
{
    return true;
}

void ProtocolEntryFileEntity::onOpen()
{
}

DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder ProtocolEntryFileEntity::order() const
{
    const QString &id = datas.value(DeviceProperty::kId).toString();

    if (id.startsWith(DFMBASE_NAMESPACE::Global::kFtp)
        || id.startsWith(DFMBASE_NAMESPACE::Global::kSFtp))
        return DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder::kOrderFtp;

    if (id.startsWith(DFMBASE_NAMESPACE::Global::kSmb))
        return DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder::kOrderSmb;

    if (id.startsWith(DFMBASE_NAMESPACE::Global::kMtp))
        return DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder::kOrderMTP;

    if (id.startsWith(DFMBASE_NAMESPACE::Global::kGPhoto2))
        return DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder::kOrderGPhoto2;

    return DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder::kOrderFiles;
}

qint64 ProtocolEntryFileEntity::sizeTotal() const
{
    return datas.value(DeviceProperty::kSizeTotal).toLongLong();
}

qint64 ProtocolEntryFileEntity::sizeUsage() const
{
    return datas.value(DeviceProperty::kSizeUsed).toLongLong();
}

void ProtocolEntryFileEntity::refresh()
{
    auto encodecId = entryUrl.path().remove("." + QString(SuffixInfo::kProtocol)).toUtf8();
    auto id = QString(QByteArray::fromBase64(encodecId));

    datas = DFMBASE_NAMESPACE::UniversalUtils::convertFromQMap(DevProxyMng->queryProtocolInfo(id));
}

QUrl ProtocolEntryFileEntity::targetUrl() const
{
    auto mpt = datas.value(DeviceProperty::kMountPoint).toString();
    QUrl target;
    if (mpt.isEmpty())
        return target;
    target.setScheme(DFMBASE_NAMESPACE::Global::kFile);
    target.setPath(mpt);
    return target;
}

QMenu *ProtocolEntryFileEntity::createMenu()
{
    QMenu *menu = new QMenu();

    menu->addAction(ContextMenuActionTrs::trOpenInNewWin());
    menu->addAction(ContextMenuActionTrs::trOpenInNewTab());
    menu->addSeparator();

    if (targetUrl().isValid()) {
        menu->addAction(ContextMenuActionTrs::trUnmount());

        QString origId = datas[DeviceProperty::kId].toString();
        if (origId.startsWith(DFMBASE_NAMESPACE::Global::kSmb) || origId.startsWith(DFMBASE_NAMESPACE::Global::kFtp) || origId.startsWith(DFMBASE_NAMESPACE::Global::kSFtp) || origId.startsWith(DFMBASE_NAMESPACE::Global::kDav))
            menu->addAction(ContextMenuActionTrs::trLogoutAndClearSavedPasswd());
    }

    menu->addAction(ContextMenuActionTrs::trProperties());
    return menu;
}
