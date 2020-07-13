//fix: 设置光盘容量属性
#include "interfaces/dfmstandardpaths.h"

#include "dfmopticalmediawidget.h"
#include "disomaster.h"
#include "shutil/fileutils.h"
#include "dialogs/burnoptdialog.h"
#include "dfilestatisticsjob.h"
#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "singleton.h"

#include <QLabel>
#include <DPushButton>

#include <QHBoxLayout>
//fix: 根据光盘选择文件状态实时更新状态
#include <QTimer>
//fix: 设置光盘容量属性
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStandardPaths>

#include <QFile>

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
    DFMOpticalMediaWidgetPrivate(DFMOpticalMediaWidget *q);
    ~DFMOpticalMediaWidgetPrivate();
    void setupUi();
    void setDeviceProperty(DeviceProperty dp);
    void setCurrentDevice(const QString &dev);
    QString getCurrentDevice() const;
    void updateBurnInfo();
    QString getVolTag();

    //fix: 根据光盘选择文件状态实时更新状态
    QTimer *updateBurnStatusTimer;

private:
    //fix: 根据光盘选择文件状态实时更新状态
    qint64 m_selectBurnFilesSize = 0;
    qint64 m_selectBurnDirCount = 0;

    QLabel *lb_mediatype;
    QLabel *lb_available;
    DPushButton *pb_burn;
    QHBoxLayout *layout;
    DFMOpticalMediaWidget *q_ptr;
    QString curdev;
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

//    d->updateBurnStatusTimer = new QTimer(this);
//    d->updateBurnStatusTimer->start(100);
//    connect(d->updateBurnStatusTimer, &QTimer::timeout, this, &DFMOpticalMediaWidget::selectBurnFilesOptionUpdate);
    connect(d->pb_burn, &DPushButton::clicked, this, [ = ] {
        DUrl url = DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + qApp->organizationName()
                                       + "/" DISCBURN_STAGING "/" + d->getCurrentDevice().replace('/','_') + "/");
        // 1、获取暂存区内文件列表信息，去除与当前光盘中有交集的部分（当前 isomaster 库不提供覆盖写入的选项，后或可优化）
        DFileView *pParent = dynamic_cast<DFileView *>(parent);
        if (!pParent)
            return;
        DUrlList lstCurr = pParent->model()->getNoTransparentUrls();

        QDir dir(url.path());
        if (!dir.exists())
            return;
        QFileInfoList lstFiles = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (lstFiles.count() == 0) {
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"), QSize(64, 64));
            dialog.setTitle(tr("No file to burn."));
            dialog.addButton(tr("OK"), true);
            dialog.exec();
            return;
        }

        // 如果光盘根目录与暂存区中文件有同名文件或文件夹，则移除暂存区中的相关文件或文件夹；
        for (QFileInfo f: lstFiles) {
            for (DUrl u: lstCurr) {
                if (f.fileName() == QFileInfo(u.path()).fileName()) {
                    if (f.isFile())
                        dir.remove(f.fileName());
                    else
                        QDir(f.absoluteFilePath()).removeRecursively();
                    continue;
                }
            }
        }
        lstFiles = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (lstFiles.count() == 0) {
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"), QSize(64, 64));
            dialog.setTitle(tr("No file to burn. Duplicated files will be ignore."));
            dialog.addButton(tr("OK"), true);
            dialog.exec();
            return;
        }

        // 2、启动worker线程计算缓存区文件大小
        if (!m_pStatisticWorker)
            return;
        QList<DUrl> urls;
        urls << url;
        m_pStatisticWorker->start(urls);
    });

    connect(m_pStatisticWorker, &DFileStatisticsJob::finished, this, [=] {
        DeviceProperty dp = ISOMaster->getDevicePropertyCached(d->getCurrentDevice());

        if (quint64(m_pStatisticWorker->totalSize()) > dp.avail) {
            //fix: 光盘容量小于刻录项目，对话框提示：目标磁盘剩余空间不足，无法进行刻录！
            //qDebug() << d->m_selectBurnFilesSize / 1024 / 1024 << "MB" << dp.avail / 1024 / 1024 << "MB";
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"), QSize(64, 64));
            dialog.setTitle(tr("Unable to burn. Not enough free space on the target disk."));
            dialog.addButton(tr("OK"), true);
            dialog.exec();
            return;
        }

        QScopedPointer<BurnOptDialog> bd(new BurnOptDialog(d->getCurrentDevice(), this));
        bd->setJobWindowId(this->window()->winId());
        bd->setDefaultVolName(dp.volid); // fix task 22858 默认填充当前盘符名称作为新的盘符名称
        bd->exec();
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
void DFMOpticalMediaWidget::setBurnCapacity(int status, QString strVolTag)
{
    QFile burnCapacityFile(QString("%1/dde-file-manager.json").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath)));
    if (!burnCapacityFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't open dde-file-manager.json burnCapacityFile!";
        return;
    }
    QByteArray burnCapacityData = burnCapacityFile.readAll();
    burnCapacityFile.close();

    double burnTotalSize = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nTotal;
    double burnUsedSize = DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].nUsage;

    QJsonObject rootObj;
    QJsonObject tagItem;
    QJsonObject burnItem;
    burnItem.insert("BurnCapacityTotalSize", burnTotalSize); //光盘容量总大小字节
    burnItem.insert("BurnCapacityUsedSize", burnUsedSize); //光盘容量已使用大小字节
    burnItem.insert("BurnCapacityStatus", status); //光盘容量状态：0,光驱弹出状态 1,光驱弹入处于添加未挂载状态 2,光驱弹入处于添加后并挂载的状态
    burnItem.insert("BurnCapacityExt", 0); //光盘容量扩展预留属性

    QJsonParseError parseJsonErr;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(burnCapacityData, &parseJsonErr));
    if(!(parseJsonErr.error == QJsonParseError::NoError)) {
        qDebug() << "decode json file error, create new json data！";
        //第一次如果没有这个属性需要创建
        tagItem[strVolTag] = burnItem;
    } else {
        rootObj = jsonDoc.object();
        if (rootObj.contains("BurnCapacityAttribute")) {
            tagItem = rootObj["BurnCapacityAttribute"].toObject();
        }
        tagItem[strVolTag] = burnItem;
    }
    rootObj["BurnCapacityAttribute"] = tagItem;
    jsonDoc.setObject(rootObj);
    QFile burnCapacityFile2(QString("%1/dde-file-manager.json").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath)));
    if (!burnCapacityFile2.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't open dde-file-manager.json burnCapacityFile2!";
        return;
    }
    burnCapacityFile2.write(jsonDoc.toJson());
    burnCapacityFile2.close();
}

void DFMOpticalMediaWidget::updateDiscInfo(QString dev)
{
    Q_D(DFMOpticalMediaWidget);
    d->setCurrentDevice(dev);

    //fix: 设置光盘容量属性
    DFMOpticalMediaWidget::setBurnCapacity(BCSA_BurnCapacityStatusAddMount, d->getVolTag());
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
    layout->addWidget(pb_burn = new DPushButton());
    pb_burn->setText(QObject::tr("Burn"));

    pb_burn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    lb_available->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lb_mediatype->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    lb_available->setAlignment(Qt::AlignCenter);
}

void DFMOpticalMediaWidgetPrivate::setDeviceProperty(DeviceProperty dp)
{
    const static QHash<MediaType, QString> rtypemap = {
        {MediaType::CD_ROM       , "CD-ROM"  },
        {MediaType::CD_R         , "CD-R"    },
        {MediaType::CD_RW        , "CD-RW"   },
        {MediaType::DVD_ROM      , "DVD-ROM" },
        {MediaType::DVD_R        , "DVD-R"   },
        {MediaType::DVD_RW       , "DVD-RW"  },
        {MediaType::DVD_PLUS_R   , "DVD+R"   },
        {MediaType::DVD_PLUS_R_DL, "DVD+R/DL"},
        {MediaType::DVD_RAM      , "DVD-RAM" },
        {MediaType::DVD_PLUS_RW  , "DVD+RW"  },
        {MediaType::BD_ROM       , "BD-ROM"  },
        {MediaType::BD_R         , "BD-R"    },
        {MediaType::BD_RE        , "BD-RE"   }
    };
    //fix: 没有选择文件时防止误操作,故默认禁止操作
    //pb_burn->setEnabled(dp.avail > 0);
//    pb_burn->setEnabled(false);
    lb_available->setText(QObject::tr("Free Space %1").arg(FileUtils::formatSize(dp.avail)));
    lb_mediatype->setText(rtypemap[dp.media]);
}

void DFMOpticalMediaWidgetPrivate::setCurrentDevice(const QString &dev)
{
    curdev = dev;
    DeviceProperty dp = ISOMaster->getDevicePropertyCached(dev);
    setDeviceProperty(dp);

    QString strKey = getVolTag();
    //fixed:CD display size error
    DFMOpticalMediaWidget::g_mapCdStatusInfo[strKey].nTotal = dp.data + dp.avail;
    DFMOpticalMediaWidget::g_mapCdStatusInfo[strKey].nUsage = dp.data;
    //qDebug() << dp.datablocks << "Sectors" << DFMOpticalMediaWidget::g_usedSize / 1024 / 1024 << "MB" << (DFMOpticalMediaWidget::g_totalSize - DFMOpticalMediaWidget::g_usedSize) / 1024 / 1024 << "MB";
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
        strKey = lst[1]; // strKey = sr0
    return strKey;
}
