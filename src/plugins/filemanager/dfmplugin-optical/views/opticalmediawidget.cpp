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
#include "opticalmediawidget.h"

#include "utils/opticalhelper.h"

#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-burn/dfmburn_global.h>

DPOPTICAL_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFM_BURN_USE_NS

using namespace GlobalServerDefines;

OpticalMediaWidget::OpticalMediaWidget(QWidget *parent)
    : QWidget(parent)
{
    initializeUi();
    initConnect();
}

void OpticalMediaWidget::updateDiscInfo(const QUrl &url)
{
    QString dev { OpticalHelper::burnDestDevice(url) };
    QString devId { OpticalHelper::deviceId(dev) };
    auto &&map = DeviceManagerInstance.invokeQueryBlockDeviceInfo(devId, true);
    QString &&mnt = qvariant_cast<QString>(map[DeviceProperty::kMountPoint]);
    bool blank { qvariant_cast<bool>(map[DeviceProperty::kOpticalBlank]) };

    curDev = qvariant_cast<QString>(map[DeviceProperty::kDevice]);
    if (curDev.isEmpty()) {
        qWarning() << "Error url: " << url << "Cannot acquire dev";
        return;
    }

    if (mnt.isEmpty() && !blank) {
        handleErrorMount();
        return;
    }
    curMnt = mnt;
    curFS = qvariant_cast<QString>(map[DeviceProperty::kFileSystem]);
    curFSVersion = qvariant_cast<QString>(map[DeviceProperty::kFsVersion]);
    curDiscName = qvariant_cast<QString>(map[DeviceProperty::kIdLabel]);
    curAvial = qvariant_cast<qint64>(map[DeviceProperty::kSizeFree]);
    curMediaWriteSpeeed = qvariant_cast<QStringList>(map[DeviceProperty::kOpticalWriteSpeed]);

    auto type = static_cast<MediaType>(map[DeviceProperty::kOpticalMediaType].toInt());
    const static QMap<MediaType, QString> rtypemap = {
        { MediaType::kCD_ROM, "CD-ROM" },
        { MediaType::kCD_R, "CD-R" },
        { MediaType::kCD_RW, "CD-RW" },
        { MediaType::kDVD_ROM, "DVD-ROM" },
        { MediaType::kDVD_R, "DVD-R" },
        { MediaType::kDVD_RW, "DVD-RW" },
        { MediaType::kDVD_PLUS_R, "DVD+R" },
        { MediaType::kDVD_PLUS_R_DL, "DVD+R/DL" },
        { MediaType::kDVD_RAM, "DVD-RAM" },
        { MediaType::kDVD_PLUS_RW, "DVD+RW" },
        { MediaType::kBD_ROM, "BD-ROM" },
        { MediaType::kBD_R, "BD-R" },
        { MediaType::kBD_RE, "BD-RE" }
    };
    curMediaType = rtypemap[type];

    updateUi();
}

void OpticalMediaWidget::initializeUi()
{
    layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->addWidget(lbMediatype = new QLabel("<Media Type>"));
    layout->addWidget(lbAvailable = new QLabel("<Space Available>"));
    layout->addWidget(lbUdsupport = new QLabel(tr("It does not support burning UDF discs")));
    layout->addWidget(pbBurn = new DPushButton());
    layout->addWidget(iconCaution = new QSvgWidget());
    pbBurn->setText(QObject::tr("Burn"));
    lbUdsupport->setVisible(false);
    iconCaution->setVisible(false);

    pbBurn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lbAvailable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lbMediatype->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lbAvailable->setAlignment(Qt::AlignCenter);
}

void OpticalMediaWidget::initConnect()
{
    // TODO(zhangs): impl me!
}

void OpticalMediaWidget::updateUi()
{
    lbMediatype->setText(curMediaType);
    lbAvailable->setText(QObject::tr("Free Space %1").arg(FileUtils::formatSize(curAvial)));
}

void OpticalMediaWidget::handleErrorMount()
{
    auto winId { OpticalHelper::winServIns()->findWindowId(this) };
    FileManagerWindow *window { OpticalHelper::winServIns()->findWindowById(winId) };
    if (window) {
        QUrl jumpUrl { UrlRoute::rootUrl(SchemeTypes::kComputer) };
        window->cd(jumpUrl);
    }
    DialogManagerInstance->showErrorDialog(tr("Mounting failed"), {});
}
