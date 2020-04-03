#include "dfmopticalmediawidget.h"
#include "disomaster.h"
#include "shutil/fileutils.h"
#include "dialogs/burnoptdialog.h"

#include <QLabel>
#include <DPushButton>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

using namespace DISOMasterNS;

//fixed:CD display size error
quint64 DFMOpticalMediaWidget::g_totalSize = 0;
quint64 DFMOpticalMediaWidget::g_usedSize = 0;

class DFMOpticalMediaWidgetPrivate
{
public:
    DFMOpticalMediaWidgetPrivate(DFMOpticalMediaWidget *q);
    ~DFMOpticalMediaWidgetPrivate();
    void setupUi();
    void setDeviceProperty(DeviceProperty dp);
    void setCurrentDevice(const QString &dev);
    QString getCurrentDevice() const;
private:
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

    connect(d->pb_burn, &DPushButton::clicked, this, [=] {
            QScopedPointer<BurnOptDialog> bd(new BurnOptDialog(d->getCurrentDevice(), this));
            bd->setJobWindowId(this->window()->winId());
            bd->exec();
        }
    );
}

DFMOpticalMediaWidget::~DFMOpticalMediaWidget()
{
}

void DFMOpticalMediaWidget::updateDiscInfo(QString dev)
{
    Q_D(DFMOpticalMediaWidget);
    d->setCurrentDevice(dev);
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
    pb_burn->setEnabled(dp.avail > 0);
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
