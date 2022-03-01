/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "burneventreceiver.h"
#include "dialogs/burnoptdialog.h"
#include "utils/burnhelper.h"
#include "utils/burnjob.h"

#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <DDialog>
#include <QtConcurrent>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

BurnEventReceiver::BurnEventReceiver(QObject *parent)
    : QObject(parent)
{
}

BurnEventReceiver *BurnEventReceiver::instance()
{
    static BurnEventReceiver receiver;
    return &receiver;
}

void BurnEventReceiver::handleShowBurnDlg(const QString &dev, bool isSupportedUDF, QWidget *parent)
{
    QString devId { DeviceManager::blockDeviceId(dev) };
    auto &&map = DeviceManagerInstance.invokeQueryBlockDeviceInfo(devId, true);

    QString defaultDiscName { qvariant_cast<QString>(map[DeviceProperty::kIdLabel]) };
    QStringList speed { qvariant_cast<QStringList>(map[DeviceProperty::kOpticalWriteSpeed]) };
    QString fileSystem { qvariant_cast<QString>(map[DeviceProperty::kFileSystem]) };
    bool disableISOOpts { isSupportedUDF };

    if (fileSystem.isEmpty()) {
        isSupportedUDF = true;
        disableISOOpts = false;
    }

    QScopedPointer<BurnOptDialog> dlg(new BurnOptDialog(dev, parent));
    dlg->setDefaultVolName(defaultDiscName);
    dlg->setUDFSupported(isSupportedUDF, disableISOOpts);
    dlg->setWriteSpeedInfo(speed);
    dlg->exec();
}

void BurnEventReceiver::handleErase(const QString &dev)
{
    DWIDGET_USE_NAMESPACE

    if (BurnHelper::showOpticalBlankConfirmationDialog() == DDialog::Accepted) {
        JobHandlePointer jobHandler { new AbstractJobHandler };
        DialogManagerInstance->addTask(jobHandler);
        QtConcurrent::run([=] {
            BurnJob job;
            job.doOpticalDiskBlank(dev, jobHandler);
        });
    }
}
