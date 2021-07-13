//fix: 设置光盘容量属性
#include "interfaces/dfmstandardpaths.h"

#include "dfmopticalmediawidget.h"
#include "disomaster.h"
#include "shutil/fileutils.h"
#include "dialogs/burnoptdialog.h"
#include "dialogs/dialogmanager.h"
#include "dfilestatisticsjob.h"
#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "singleton.h"
#include "models/dfmrootfileinfo.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

#include <QLabel>
#include <DPushButton>

#include <QHBoxLayout>
#include <QSvgWidget>
//fix: 根据光盘选择文件状态实时更新状态
#include <QTimer>
//fix: 设置光盘容量属性
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStandardPaths>
#include <QProcess>
#include <QFile>
#include <glob.h>

DWIDGET_USE_NAMESPACE

using namespace DISOMasterNS;

//fixed:CD display size error
QMap<QString, CdStatusInfo> DFMOpticalMediaWidget::g_mapCdStatusInfo;
//fix: 动态获取刻录选中文件的字节大小
qint64 DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;
qint64 DFMOpticalMediaWidget::g_selectBurnDirFileCount = 0;

class DFMOpticalMediaWidgetPrivate
{
public:
    explicit DFMOpticalMediaWidgetPrivate(DFMOpticalMediaWidget *q);
    ~DFMOpticalMediaWidgetPrivate();
    void setupUi();
    void setDeviceProperty(DeviceProperty dp);
    void setCurrentDevice(const QString &dev);
    void setCurrentFSByDev(const QString &dev);
    QString getCurrentDevice() const;
    QString getVolTag();

    //fix: 根据光盘选择文件状态实时更新状态
    QTimer *updateBurnStatusTimer = nullptr;

private:
    //fix: 根据光盘选择文件状态实时更新状态
    qint64 m_selectBurnFilesSize = 0;
    qint64 m_selectBurnDirCount = 0;

    QLabel *lb_mediatype = nullptr;
    QLabel *lb_available = nullptr;
    QLabel *lb_udsupport = nullptr;
    DPushButton *pb_burn = nullptr;
    QSvgWidget *icon_caution = nullptr;

    QHBoxLayout *layout = nullptr;
    DFMOpticalMediaWidget *q_ptr;
    QString curFS;
    QString curFSVersion;
    QString curdev;
    QString strMntPath;
    QString defaultDiscName;
    Q_DECLARE_PUBLIC(DFMOpticalMediaWidget)
};

DFMOpticalMediaWidget::DFMOpticalMediaWidget(QWidget *parent) :
    QWidget(parent),
    d_ptr(new DFMOpticalMediaWidgetPrivate(this))
{
    Q_D(DFMOpticalMediaWidget);
    d->setupUi();

    m_pStatisticWorker = new DFileStatisticsJob(this);
    //fix: 根据光盘选择文件状态实时更新状态
    DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;
    DFMOpticalMediaWidget::g_selectBurnDirFileCount = 0;

    connect(d->pb_burn, &DPushButton::clicked, this, [ = ] {
        if (m_pStatisticWorker->isRunning())
            return;

        QString volTag = d->getVolTag();
        CdStatusInfo &statusInfo = DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag];
        statusInfo.bReadyToBurn = true;

        QDir::Filters filter = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;

        DUrl urlOfStage = DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + qApp->organizationName()
                                              + "/" DISCBURN_STAGING "/" + d->getCurrentDevice().replace('/', '_') + "/");
        // 1、获取暂存区内文件列表信息，去除与当前光盘中有交集的部分（当前 isomaster 库不提供覆盖写入的选项，后或可优化）
        QDir dirMnt(d->strMntPath);
        if (!dirMnt.exists())
        {
            statusInfo.bReadyToBurn = false;
            qWarning() << "Mount points doesn't exist: " << d->strMntPath;
            return;
        }
        // 如果放入空盘是没有挂载点的，此时给QDir传入空的path将导致QDir获取到的是程序运行目录的Dir，之后的去重会产生不正常的结果
        QFileInfoList lstFilesOnDisc = d->strMntPath.isEmpty() ? QFileInfoList() : dirMnt.entryInfoList(filter);

        QDir dirStage(urlOfStage.path());
        if (!dirStage.exists())
        {
            statusInfo.bReadyToBurn = false;
            return;
        }
        QFileInfoList lstFilesInStage = dirStage.entryInfoList(filter);
        if (lstFilesInStage.count() == 0)
        {
            statusInfo.bReadyToBurn = false;
            dialogManager->showMessageDialog(DialogManager::msgWarn, tr("No files to burn"));
            return;
        }

        bool bDeletedValidFile = false; // 在点击进入光驱中文件夹时，因解决bug#27870时，在暂存区中手动创建了本不存在的目录
        // 如果光盘挂载根目录与暂存区根目录中有同名文件或文件夹，则移除暂存区中的相关文件或文件夹；
        for (QFileInfo fOn : lstFilesOnDisc)
        {
            for (QFileInfo fStage : lstFilesInStage) {
                if (fStage.fileName() != fOn.fileName())
                    continue;

                if (fStage.isFile() || fStage.isSymLink())
                    dirStage.remove(fStage.fileName());
                else {
                    if (!bDeletedValidFile)
                        bDeletedValidFile = hasFileInDir(fStage.absoluteFilePath()); // 这里判断是否有移除掉非文件夹类文件
                    QDir(fStage.absoluteFilePath()).removeRecursively();
                }
            }
        }

        lstFilesInStage = dirStage.entryInfoList(filter);
        if (lstFilesInStage.count() == 0)
        {
            QString errTitle(tr("No files to burn"));
            dialogManager->showMessageDialog(DialogManager::msgWarn, errTitle);
            statusInfo.bReadyToBurn = false;
            return;
        }

        // 2、启动worker线程计算缓存区文件大小
        m_pStatisticWorker->start({urlOfStage});
    });

    connect(m_pStatisticWorker, &DFileStatisticsJob::finished, this, [ = ] {

        QString volTag = d->getVolTag();
        CdStatusInfo &statusInfo = DFMOpticalMediaWidget::g_mapCdStatusInfo[volTag];

        DeviceProperty dp = ISOMaster->getDevicePropertyCached(d->getCurrentDevice());

        if (dp.avail == 0 || static_cast<quint64>(m_pStatisticWorker->totalSize()) > dp.avail) // 可用空间为0时也禁止刻录
        {
            //fix: 光盘容量小于刻录项目，对话框提示：目标磁盘剩余空间不足，无法进行刻录！
            qDebug() << d->m_selectBurnFilesSize / 1024 / 1024 << "MB" << dp.avail / 1024 / 1024 << "MB";
            dialogManager->showMessageDialog(DialogManager::msgWarn, tr("Unable to burn. Not enough free space on the target disk."));
            statusInfo.bReadyToBurn = false;
            return;
        }

        QScopedPointer<BurnOptDialog> bd(new BurnOptDialog(d->getCurrentDevice(), this));
        bd->setJobWindowId(static_cast<int>(this->window()->winId()));
        bd->setDefaultVolName(d->defaultDiscName);
        bd->setDiscAndFsInfo(dp.media, d->curFS, d->curFSVersion);
        bd->exec();
        statusInfo.bReadyToBurn = false;
    });
}

DFMOpticalMediaWidget::~DFMOpticalMediaWidget()
{
    Q_D(DFMOpticalMediaWidget);
//    d->updateBurnStatusTimer->stop();
    DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;
    DFMOpticalMediaWidget::g_selectBurnDirFileCount = 0;
    d->m_selectBurnFilesSize = 0;
    d->m_selectBurnDirCount = 0;
}

//fix: 设置光盘容量属性
// TODO(zhangs): 此函数的逻辑不应该在此类中，重构后需要移除，将此逻辑改为服务进程中，通过 dbus 接口调用
void DFMOpticalMediaWidget::setBurnCapacity(int status, QString strVolTag)
{
    QMap<QString, QVariant> capacityInfo;
    double burnTotalSize = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nTotal;
    double burnUsedSize = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nUsage;

    capacityInfo[BURN_CAPACITY_TOTAL_SIZE] = burnTotalSize;
    capacityInfo[BURN_CAPACITY_USED_SIZE] = burnUsedSize;
    capacityInfo[BURN_CAPACITY_STATUS] = status;

    DFMApplication::genericSetting()->setValue(BURN_CAPACITY_ATTRIBUTE, strVolTag, capacityInfo);
}

void DFMOpticalMediaWidget::updateDiscInfo(QString dev)
{
    Q_D(DFMOpticalMediaWidget);
    d->setCurrentFSByDev(dev);
    d->setCurrentDevice(dev);
    //fix: 设置光盘容量属性
    DFMOpticalMediaWidget::setBurnCapacity(BCSA_BurnCapacityStatusAddMount, d->getVolTag());
}

void DFMOpticalMediaWidget::setDiscMountPoint(const QString &strMntPath)
{
    Q_D(DFMOpticalMediaWidget);
    d->strMntPath = strMntPath;
}

void DFMOpticalMediaWidget::setDefaultDiscName(const QString &name)
{
    Q_D(DFMOpticalMediaWidget);
    d->defaultDiscName = name;
}

bool DFMOpticalMediaWidget::hasFileInDir(QDir dir)
{
    QFileInfoList lstFiles = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::System);
    foreach (QFileInfo f, lstFiles) {
        if (f.isFile())
            return true;
        return hasFileInDir(QDir(f.absoluteFilePath()));
    }
    return false;
}

QString DFMOpticalMediaWidget::getDiscMountPoint()
{
    Q_D(DFMOpticalMediaWidget);
    return d->strMntPath;
}

//fix: 根据光盘选择文件状态实时更新状态
void DFMOpticalMediaWidget::selectBurnFilesOptionUpdate()
{
    Q_D(DFMOpticalMediaWidget);
    d->m_selectBurnDirCount = DFMOpticalMediaWidget::g_selectBurnDirFileCount;
    d->m_selectBurnFilesSize = DFMOpticalMediaWidget::g_selectBurnFilesSize;
    if ((d->m_selectBurnFilesSize > 0) || (d->m_selectBurnDirCount > 0)) {
        d->pb_burn->setEnabled(true);
    } else {
        d->pb_burn->setEnabled(false);
    }
}

QString DFMOpticalMediaWidget::getVolTag(const DUrl &fileUrl)
{
    QString strVolTag = fileUrl.path().split("/", QString::SkipEmptyParts).count() >= 2
                        ? fileUrl.path().split("/", QString::SkipEmptyParts).at(1)
                        : "";
    return strVolTag;
}

bool DFMOpticalMediaWidget::hasVolProcessBusy()
{
    QMap<QString, CdStatusInfo>::iterator ite = g_mapCdStatusInfo.begin();

    for (; ite != g_mapCdStatusInfo.end(); ++ite) {
        if (ite.value().bProcessLocked)
            return true;
    }
    return false;
}

CdStatusInfo *DFMOpticalMediaWidget::getCdStatusInfo(const QString &dev)
{
    QMap<QString, CdStatusInfo>::iterator ite = g_mapCdStatusInfo.find(dev);
    if (ite != g_mapCdStatusInfo.end()) {
        return &(ite.value());
    }
    return nullptr;
}

DFMOpticalMediaWidgetPrivate::DFMOpticalMediaWidgetPrivate(DFMOpticalMediaWidget *q) :
    q_ptr(q)
{
}

DFMOpticalMediaWidgetPrivate::~DFMOpticalMediaWidgetPrivate()
{
}

void DFMOpticalMediaWidgetPrivate::setupUi()
{
    Q_Q(DFMOpticalMediaWidget);
    layout = new QHBoxLayout(q);
    q->setLayout(layout);
    layout->addWidget(lb_mediatype = new QLabel("<Media Type>"));
    layout->addWidget(lb_available = new QLabel("<Space Available>"));
    layout->addWidget(lb_udsupport = new QLabel(QObject::tr("It does not support burning %1 discs").arg(QString("U/D/F").remove("/"))));
    layout->addWidget(pb_burn = new DPushButton());
    layout->addWidget(icon_caution = new QSvgWidget());
    pb_burn->setText(QObject::tr("Burn"));
    lb_udsupport->setVisible(false);
    icon_caution->setVisible(false);


    pb_burn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lb_available->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lb_mediatype->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    lb_available->setAlignment(Qt::AlignCenter);
}

void DFMOpticalMediaWidgetPrivate::setDeviceProperty(DeviceProperty dp)
{
    const static QHash<MediaType, QString> rtypemap = {
        {MediaType::CD_ROM, "CD-ROM"},
        {MediaType::CD_R, "CD-R"},
        {MediaType::CD_RW, "CD-RW"},
        {MediaType::DVD_ROM, "DVD-ROM"},
        {MediaType::DVD_R, "DVD-R"},
        {MediaType::DVD_RW, "DVD-RW"},
        {MediaType::DVD_PLUS_R, "DVD+R"},
        {MediaType::DVD_PLUS_R_DL, "DVD+R/DL"},
        {MediaType::DVD_RAM, "DVD-RAM"},
        {MediaType::DVD_PLUS_RW, "DVD+RW"},
        {MediaType::BD_ROM, "BD-ROM"},
        {MediaType::BD_R, "BD-R"},
        {MediaType::BD_RE, "BD-RE"}
    };

    lb_available->setText(QObject::tr("Free Space %1").arg(FileUtils::formatSize(dp.avail)));
    lb_mediatype->setText(rtypemap[dp.media]);

    auto isSupportedUD = [this, &dp]{
        if (!DFMGlobal::isProfessional())
            return false;
        if (!BurnOptDialog::isSupportedUDVersion(curFSVersion))
            return false;
        if (BurnOptDialog::isSupportedUDMedium(dp.media))
            return true;
        return false;
    };
    if (curFS == QString("u/d/f").remove("/") && !isSupportedUD()) {
        if (DFMGlobal::isProfessional()) { // for other version, show normal unsupported writtings
            lb_udsupport->setText(DFMOpticalMediaWidget::tr("%1 burning is not supported").arg("U/D/F").remove("/"));
            icon_caution->setVisible(true);
            icon_caution->load(QString(":/dark/icons/caution.svg"));
            icon_caution->setFixedSize(14, 14);
            icon_caution->setToolTip(DFMOpticalMediaWidget::tr("1. It is not %1 disc;\n2. The version of this file system does not support adding files yet.")
                                     .arg("DVD+R, DVD-R, CD-R, CD-RW"));
        }
        lb_udsupport->setVisible(true);
        pb_burn->setEnabled(false);
    } else {
        lb_udsupport->setVisible(false);
        icon_caution->setVisible(false);
        pb_burn->setEnabled(true);
    }

    if (dp.avail == 0) {
        lb_udsupport->setVisible(false);
        icon_caution->setVisible(false);
    }
}

void DFMOpticalMediaWidgetPrivate::setCurrentDevice(const QString &dev)
{
    curdev = dev;
    DeviceProperty dp = ISOMaster->getDevicePropertyCached(dev);
    setDeviceProperty(dp);

    QString strKey = getVolTag();
    //fixed:CD display size error
    if (dp.avail != 0 || DFMOpticalMediaWidget::g_mapCdStatusInfo[strKey].nTotal == 0) {
        DFMOpticalMediaWidget::g_mapCdStatusInfo[strKey].nTotal = dp.data + dp.avail;
        DFMOpticalMediaWidget::g_mapCdStatusInfo[strKey].nUsage = dp.data;
    }
    if (dp.avail == 0) // fix while available is zero, the button is still enable
        pb_burn->setEnabled(false);

    QString tempMediaAddr = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString cachePath = tempMediaAddr + DISCBURN_CACHE_MID_PATH + strKey;
    DFMOpticalMediaWidget::g_mapCdStatusInfo[strKey].cachePath = cachePath;
    DFMOpticalMediaWidget::g_mapCdStatusInfo[strKey].bLoading = false;

    qDebug() << "get " << strKey << " catch path:" << cachePath;
}

void DFMOpticalMediaWidgetPrivate::setCurrentFSByDev(const QString &dev)
{
    DAbstractFileInfoPointer fi(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + dev.mid(QString("/dev/").length()) + "." SUFFIX_UDISKS)));
    curFS = fi->extraProperties()["fsType"].toString().toLower();
    curFSVersion = fi->extraProperties()["fsVersion"].toString().toLower();
}

QString DFMOpticalMediaWidgetPrivate::getCurrentDevice() const
{
    return curdev;
}

QString DFMOpticalMediaWidgetPrivate::getVolTag()
{
    QString strKey;
    QStringList lst = curdev.split("/", QString::SkipEmptyParts); // /dev/sr0 -> { dev, sr0 }
    if (lst.count() >= 2)
        strKey = lst[1]; // strKey =
    return strKey;
}
