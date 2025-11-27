// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "opticalmediawidget.h"

#include "utils/opticalhelper.h"
#include "utils/opticalsignalmanager.h"
#include "events/opticaleventcaller.h"

#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-burn/dburn_global.h>

#include <DSysInfo>
#include <QDir>

using namespace dfmplugin_optical;
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFM_BURN_USE_NS

using namespace GlobalServerDefines;

OpticalMediaWidget::OpticalMediaWidget(QWidget *parent)
    : QWidget(parent)
{
    initializeUi();
    initConnect();
}

bool OpticalMediaWidget::updateDiscInfo(const QUrl &url, bool retry)
{
    curUrl = url;
    QString dev { OpticalHelper::burnDestDevice(url) };
    devId = { DeviceUtils::getBlockDeviceId(dev) };
    auto &&map = DevProxyMng->queryBlockInfo(devId);
    QString &&mnt = qvariant_cast<QString>(map[DeviceProperty::kMountPoint]);
    isBlank = DeviceUtils::isBlankOpticalDisc(devId);
    curDev = qvariant_cast<QString>(map[DeviceProperty::kDevice]);

    if (curDev.isEmpty()) {
        fmWarning() << "Error url: " << url << "Cannot acquire dev";
        return false;
    }

    if (mnt.isEmpty() && !isBlank) {
        fmWarning() << "Mount point is empty for non-blank disc, handling error mount for URL:" << url;
        handleErrorMount();
        return false;
    }

    // Acquire blank disc info
    curAvial = qvariant_cast<qint64>(map[DeviceProperty::kSizeFree]);
    if (!retry && isBlank && curAvial == 0) {
        fmDebug() << "Blank disc with 0 available space, attempting to mount device:" << devId;
        DevMngIns->mountBlockDevAsync(devId, {}, [this, url](bool, const DFMMOUNT::OperationErrorInfo &, const QString &) {
            this->updateDiscInfo(url, true);
        });
    }

    disableNotify = false;
    curMnt = mnt;
    curFS = qvariant_cast<QString>(map[DeviceProperty::kFileSystem]);
    curFSVersion = qvariant_cast<QString>(map[DeviceProperty::kFsVersion]);
    curDiscName = qvariant_cast<QString>(map[DeviceProperty::kIdLabel]);

    auto type = static_cast<MediaType>(map[DeviceProperty::kOpticalMediaType].toInt());
    curMediaType = int(type);
    const static QMap<MediaType, QString> kDiscTypeMap = {
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
    curMediaTypeStr = kDiscTypeMap[type];

    updateUi();
    return true;
}

void OpticalMediaWidget::initializeUi()
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    // 创建控件
    lbMediatype = new QLabel("<Media Type>");
    lbAvailable = new QLabel("<Space Available>");
    lbUDFSupport = new QLabel(tr("It does not support burning UDF discs"));
    pbDump = new DPushButton();
    pbBurn = new DPushButton();
    iconCaution = new QSvgWidget();

    // 设置按钮文本
    pbDump->setText(tr("Save as Image File"));
    pbBurn->setText(tr("Burn"));

    // 设置控件初始可见性
    lbUDFSupport->setVisible(false);
    iconCaution->setVisible(false);

    // 设置按钮固定高度为30px
    pbDump->setFixedHeight(30);
    pbBurn->setFixedSize({ 86, 30 });

    // 设置标签字体大小为16px
    QFont font = lbMediatype->font();
    font.setPixelSize(16);
    lbMediatype->setFont(font);
    lbAvailable->setFont(font);
    lbUDFSupport->setFont(font);

    // 设置布局边距为6px
    layout->setContentsMargins(10, 6, 10, 6);

    // 添加控件到布局
    layout->addWidget(lbMediatype);
    layout->addWidget(lbAvailable);
    layout->addWidget(lbUDFSupport);
    layout->addWidget(pbDump);
    layout->addWidget(pbBurn);
    layout->addWidget(iconCaution);

    // 设置控件的大小策略
    pbBurn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lbAvailable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lbMediatype->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lbAvailable->setAlignment(Qt::AlignCenter);

    statisticWorker = new FileStatisticsJob(this);
    statisticWorker->setFileHints(FileStatisticsJob::FileHint::kNoFollowSymlink | FileStatisticsJob::FileHint::kDontSizeInfoPointer);
}

void OpticalMediaWidget::initConnect()
{
    connect(pbBurn, &QPushButton::clicked, this, &OpticalMediaWidget::onBurnButtonClicked);
    connect(pbDump, &QPushButton::clicked, this, &OpticalMediaWidget::onDumpButtonClicked);
    connect(statisticWorker, &FileStatisticsJob::finished, this, &OpticalMediaWidget::onStagingFileStatisticsFinished);
    connect(OpticalSignalManager::instance(), &OpticalSignalManager::discUnmounted, this, &OpticalMediaWidget::onDiscUnmounted);
}

void OpticalMediaWidget::updateUi()
{
    lbMediatype->setText(curMediaTypeStr);
    lbAvailable->setText(QObject::tr("Free Space %1").arg(FileUtils::formatSize(curAvial)));

    if (curFS.toLower() == "udf" && !isSupportedUDF()) {
        fmInfo() << "UDF filesystem detected but not supported - Type:" << curMediaTypeStr << "Version:" << curFSVersion;
        if (DSysInfo::deepinType() == DSysInfo::DeepinProfessional) {   // for other version, show normal unsupported writtings
            lbUDFSupport->setText(tr("%1 burning is not supported").arg("UDF"));
            iconCaution->setVisible(true);
            iconCaution->load(QString(":/dark/icons/caution.svg"));
            iconCaution->setFixedSize(14, 14);
            iconCaution->setToolTip(tr("1. It is not %1 disc;\n2. The version of this file system does not support adding files yet.")
                                            .arg("DVD+R, DVD-R, CD-R, CD-RW"));
        }
        lbUDFSupport->setVisible(true);
        pbBurn->setEnabled(false);
    } else {
        lbUDFSupport->setVisible(false);
        iconCaution->setVisible(false);
        pbBurn->setEnabled(true);
    }

    if (curAvial == 0) {
        fmInfo() << "No available space on disc, disabling burn functionality";
        lbUDFSupport->setVisible(false);
        iconCaution->setVisible(false);
        pbBurn->setEnabled(false);
    }

    if (isBlank) {
        fmInfo() << "Empty disc, disable dump iso";
        pbDump->setEnabled(false);
    } else {
        pbDump->setEnabled(true);
    }

    if (!OpticalHelper::isBurnEnabled()) {
        fmInfo() << "Burn functionality is disabled globally";
        pbBurn->setEnabled(false);
    }
}

void OpticalMediaWidget::handleErrorMount()
{
    auto winId { FMWindowsIns.findWindowId(this) };
    FileManagerWindow *window { FMWindowsIns.findWindowById(winId) };
    if (window) {
        QUrl jumpUrl { UrlRoute::rootUrl(Global::Scheme::kComputer) };
        window->cd(jumpUrl);
    }

    if (disableNotify) {
        fmDebug() << "Notification disabled, skipping error dialog";
        disableNotify = false;
        return;
    }

    DevMngIns->ejectBlockDevAsync(devId, {}, [](bool, const DFMMOUNT::OperationErrorInfo &) {
        DialogManagerInstance->showErrorDialog(tr("Mounting failed"), {});
    });
}

bool OpticalMediaWidget::isSupportedUDF()
{
    if (!(DSysInfo::deepinType() == DSysInfo::DeepinProfessional)) {
        fmDebug() << "UDF not supported - not professional edition";
        return false;
    }
    if (!OpticalHelper::isSupportedUDFMedium(curMediaType)) {
        fmDebug() << "UDF not supported - media type not compatible: " << curMediaType;
        return false;
    }
    if (!curFS.isEmpty() && !OpticalHelper::isSupportedUDFVersion(curFSVersion)) {
        fmDebug() << "UDF not supported - filesystem version not compatible: " << curFSVersion;
        return false;
    }

    return true;
}

void OpticalMediaWidget::onBurnButtonClicked()
{
    if (statisticWorker->isRunning()) {
        fmWarning() << "statisticWorker is running";
        return;
    }

    // not mount point
    QDir dirMnt(curMnt);
    if (!dirMnt.exists()) {
        fmWarning() << "Mount points doesn't exist: " << curMnt;
        return;
    }

    // not stage files foldder
    QUrl urlOfStage { OpticalHelper::localStagingFile(curDev) };
    QDir dirStage(urlOfStage.path());
    if (!dirStage.exists()) {
        fmWarning() << "Staging files not exist: " << dirStage;
        return;
    }

    // empty stage files folder
    QString errTitle(tr("No files to burn"));
    QDir::Filters filter { QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::NoSymLinks };
    QFileInfoList listFilesInStage = dirStage.entryInfoList(filter);
    if (listFilesInStage.count() == 0) {
        fmInfo() << "No files found in staging folder, showing warning dialog";
        DialogManagerInstance->showMessageDialog(DialogManager::kMsgWarn, errTitle);
        return;
    }

    // Set busy state while calculating file sizes
    setCursor(Qt::WaitCursor);
    pbBurn->setEnabled(false);

    statisticWorker->start({ urlOfStage });
}

void OpticalMediaWidget::onDumpButtonClicked()
{
    fmInfo() << "Dump button clicked, opening dump ISO dialog for device:" << devId;
    OpticalEventCaller::sendOpenDumpISODlg(devId);
}

void OpticalMediaWidget::onStagingFileStatisticsFinished()
{
    // Restore cursor and button state
    unsetCursor();

    auto &&map = DevProxyMng->queryBlockInfo(devId);
    qint64 avil { qvariant_cast<qint64>(map[DeviceProperty::kSizeFree]) };

    // Update available space and UI state
    curAvial = avil;
    updateUi();

    qint64 total { statisticWorker->totalSize() };
    if (avil == 0 || total > avil) {
        fmWarning() << "Insufficient space for burn operation - Available:" << avil << "Required:" << total;
        DialogManagerInstance->showMessageDialog(DialogManager::kMsgWarn, tr("Unable to burn. Not enough free space on the target disk."));
        return;
    }

    auto id = FMWindowsIns.findWindowId(this);
    auto window = FMWindowsIns.findWindowById(id);
    OpticalEventCaller::sendOpenBurnDlg(curDev, isSupportedUDF(), window);
}

void OpticalMediaWidget::onDiscUnmounted(const QUrl &url)
{
    if (UniversalUtils::urlEquals(curUrl, url)) {
        fmInfo() << "Current disc was unmounted, disabling notifications: " << url;
        disableNotify = true;
    } else {
        disableNotify = false;
    }
}
