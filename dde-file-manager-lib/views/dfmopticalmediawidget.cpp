//fix: 设置光盘容量属性
#include "interfaces/dfmstandardpaths.h"

#include "dfmopticalmediawidget.h"
#include "disomaster.h"
#include "shutil/fileutils.h"
#include "dialogs/burnoptdialog.h"

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

#include <QFile>

DWIDGET_USE_NAMESPACE

using namespace DISOMasterNS;

//fixed:CD display size error
quint64 DFMOpticalMediaWidget::g_totalSize = 0;
quint64 DFMOpticalMediaWidget::g_usedSize = 0;
//fix: 动态获取刻录选中文件的字节大小
qint64 DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;

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

    //fix: 根据光盘选择文件状态实时更新状态
    QTimer *updateBurnStatusTimer;

private:
    //fix: 根据光盘选择文件状态实时更新状态
    qint64 m_selectBurnFilesSize = 0;

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

    //fix: 根据光盘选择文件状态实时更新状态
    DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;
    d->updateBurnStatusTimer = new QTimer(this);
    d->updateBurnStatusTimer->start(100);
    connect(d->updateBurnStatusTimer, &QTimer::timeout, this, &DFMOpticalMediaWidget::selectBurnFilesOptionUpdate);
    connect(d->pb_burn, &DPushButton::clicked, this, [=] {
            //fix: 光盘容量小于刻录项目，对话框提示：目标磁盘剩余空间不足，无法进行刻录！
            DeviceProperty dp = ISOMaster->getDevicePropertyCached(d->getCurrentDevice());
            //qDebug() << d->m_selectBurnFilesSize / 1024 / 1024 << "MB" << dp.avail / 1024 / 1024 << "MB";
            if (d->m_selectBurnFilesSize > dp.avail) {
                DDialog dialog(this);
                dialog.setIcon(QIcon::fromTheme("dialog-warning"), QSize(64, 64));
                dialog.setTitle(tr("Unable to burn. Not enough free space on the target disk."));
                dialog.addButton(tr("OK"), true);
                dialog.exec();
                return;
            }

            QScopedPointer<BurnOptDialog> bd(new BurnOptDialog(d->getCurrentDevice(), this));
            bd->setJobWindowId(this->window()->winId());
            bd->exec();
        }
    );
}

DFMOpticalMediaWidget::~DFMOpticalMediaWidget()
{
    Q_D(DFMOpticalMediaWidget);
    d->updateBurnStatusTimer->stop();
    DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;
    d->m_selectBurnFilesSize = 0;
}

//fix: 设置光盘容量属性
void DFMOpticalMediaWidget::setBurnCapacity(int status)
{
    QFile burnCapacityFile(QString("%1/dde-file-manager.json").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath)));
    if (!burnCapacityFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't open dde-file-manager.json burnCapacityFile!";
        return;
    }
    QByteArray burnCapacityData = burnCapacityFile.readAll();
    burnCapacityFile.close();

    QJsonParseError parseJsonErr;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(burnCapacityData,&parseJsonErr));
    if(!(parseJsonErr.error == QJsonParseError::NoError)) {
        qDebug() << "decode json file error！";
        return;
    }
    QJsonObject tempBurnObjs = jsonDoc.object();
//    qDebug() << "tempBurnObjs1==" << tempBurnObjs;
    if (tempBurnObjs.contains(QStringLiteral("BurnCapacityAttribute"))) {
        QJsonValue jsonBurnValueList = tempBurnObjs.value(QStringLiteral("BurnCapacityAttribute"));
        QJsonObject burnItem = jsonBurnValueList.toObject();
        qDebug() << "burnItem[BurnCapacityTotalSize]==" << burnItem["BurnCapacityTotalSize"].toDouble();
        qDebug() << "burnItem[BurnCapacityUsedSize]==" << burnItem["BurnCapacityUsedSize"].toDouble();
        qDebug() << "burnItem[BurnCapacityStatus]==" << burnItem["BurnCapacityStatus"].toInt();
        qDebug() << "burnItem[BurnCapacityExt]==" << burnItem["BurnCapacityExt"].toInt();
        double burnTotalSize = DFMOpticalMediaWidget::g_totalSize;
        double burnUsedSize = DFMOpticalMediaWidget::g_usedSize;
//        burnItem.insert("BurnCapacityTotalSize", burnTotalSize); //光盘容量总大小字节
//        burnItem.insert("BurnCapacityUsedSize", burnUsedSize); //光盘容量已使用大小字节
//        burnItem.insert("BurnCapacityStatus", BCSA_BurnCapacityStatusAddMount); //光盘容量状态：0,光驱弹出状态 1,光驱弹入处于添加未挂载状态 2,光驱弹入处于添加后并挂载的状态
//        burnItem.insert("BurnCapacityExt", 0); //光盘容量扩展预留属性
//        tempBurnObjs.insert("BurnCapacityAttribute", burnItem);
        burnItem["BurnCapacityTotalSize"] = burnTotalSize; //光盘容量总大小字节
        burnItem["BurnCapacityUsedSize"] = burnUsedSize; //光盘容量已使用大小字节
        burnItem["BurnCapacityStatus"] = status; //光盘容量状态：0,光驱弹出状态 1,光驱弹入处于添加未挂载状态 2,光驱弹入处于添加后并挂载的状态
        burnItem["BurnCapacityExt"] = 0; //光盘容量扩展预留属性
        tempBurnObjs["BurnCapacityAttribute"] = burnItem;
        jsonDoc.setObject(tempBurnObjs);
//        qDebug() << "tempBurnObjs2==" << tempBurnObjs;

        QFile burnCapacityFile1(QString("%1/dde-file-manager.json").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath)));
        if (!burnCapacityFile1.open(QIODevice::WriteOnly)) {
            qDebug() << "Couldn't open dde-file-manager.json burnCapacityFile1!";
            return;
        }
        burnCapacityFile1.write(jsonDoc.toJson());
        burnCapacityFile1.close();
    }
}

void DFMOpticalMediaWidget::updateDiscInfo(QString dev)
{
    Q_D(DFMOpticalMediaWidget);
    d->setCurrentDevice(dev);

    //fix: 设置光盘容量属性
    DFMOpticalMediaWidget::setBurnCapacity(BCSA_BurnCapacityStatusAddMount);
}

//fix: 根据光盘选择文件状态实时更新状态
void DFMOpticalMediaWidget::selectBurnFilesOptionUpdate()
{
    Q_D(DFMOpticalMediaWidget);
    d->m_selectBurnFilesSize = DFMOpticalMediaWidget::g_selectBurnFilesSize;
    if (d->m_selectBurnFilesSize > 0) {
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
    pb_burn->setEnabled(false);
    lb_available->setText(QObject::tr("Free Space %1").arg(FileUtils::formatSize(dp.avail)));
    lb_mediatype->setText(rtypemap[dp.media]);
}

void DFMOpticalMediaWidgetPrivate::setCurrentDevice(const QString &dev)
{
    curdev = dev;
    DeviceProperty dp = ISOMaster->getDevicePropertyCached(dev);
    setDeviceProperty(dp);

    //fixed:CD display size error
    DFMOpticalMediaWidget::g_usedSize = dp.data;
    DFMOpticalMediaWidget::g_totalSize = dp.data + dp.avail;
    //qDebug() << dp.datablocks << "Sectors" << DFMOpticalMediaWidget::g_usedSize / 1024 / 1024 << "MB" << (DFMOpticalMediaWidget::g_totalSize - DFMOpticalMediaWidget::g_usedSize) / 1024 / 1024 << "MB";
}

QString DFMOpticalMediaWidgetPrivate::getCurrentDevice() const
{
    return curdev;
}
