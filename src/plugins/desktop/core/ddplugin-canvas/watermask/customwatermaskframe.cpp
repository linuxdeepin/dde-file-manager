#include "customwatermaskframe.h"
#include "displayconfig.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DSysInfo>

#include <QHBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImageReader>
#include <QFont>

using namespace ddplugin_canvas;
using namespace dfmbase;
DCORE_USE_NAMESPACE

static constexpr char kConfName[] { "org.deepin.dde.file-manager.customwatermask" };
static constexpr char kIsMaskOpen[] { "isMaskOpen" };
static constexpr char kMaskLogoUri[] { "maskLogoUri" };
static constexpr char kMaskOffsetX[] { "maskOffsetX" };
static constexpr char kMaskOffsetY[] { "maskOffsetY" };
static constexpr char kMaskWidth[] { "maskWidth" };
static constexpr char kMaskHeight[] { "maskHeight" };

CustomWaterMaskFrame::CustomWaterMaskFrame(QWidget *parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    logoLabel = new QLabel(this);

    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &CustomWaterMaskFrame::onConfigChanged, Qt::DirectConnection);
}

CustomWaterMaskFrame::~CustomWaterMaskFrame()
{

}

void CustomWaterMaskFrame::loadConfig()
{
   maskOpen = DConfigManager::instance()->value(kConfName, kIsMaskOpen).toBool();
   maskLogoUri = DConfigManager::instance()->value(kConfName, kMaskLogoUri).toString();

   maskOffset.setX(DConfigManager::instance()->value(kConfName, kMaskOffsetX).toInt());
   maskOffset.setY(DConfigManager::instance()->value(kConfName, kMaskOffsetY).toInt());
   maskSize.setWidth(DConfigManager::instance()->value(kConfName, kMaskWidth).toInt());
   maskSize.setHeight(DConfigManager::instance()->value(kConfName, kMaskHeight).toInt());

    if (maskLogoUri.startsWith("~/")) {
        maskLogoUri.replace(0, 1, QDir::homePath());
    }

    return;
}

void CustomWaterMaskFrame::refresh()
{
    loadConfig();
    update();

    return;
}

void CustomWaterMaskFrame::setPosition()
{
    int x = systemMaskPosition.x() + maskOffset.x();
    int y = systemMaskPosition.y() - maskOffset.y() - maskSize.height();

    move(x, y);
    return;
}

void CustomWaterMaskFrame::onConfigChanged(const QString &cfg, const QString &key)
{
    if (cfg != QString(kConfName))
        return;

    refresh();
    return;
}

void CustomWaterMaskFrame::update()
{
    if (!maskOpen || !systemMaskEnable) {
        hide();
        return;
    }

    // remove old layout
    {
        auto mainLayout = this->layout();
        if (mainLayout) {
            delete mainLayout;
            mainLayout = nullptr;
        }
    }

    auto mainLayout = new QHBoxLayout();
    mainLayout->addWidget(logoLabel);

    logoLabel->setPixmap(maskPixmap(maskLogoUri, maskSize, logoLabel->devicePixelRatioF()));
    logoLabel->setFixedSize(maskSize.width(), maskSize.height());

    setLayout(mainLayout);
    setPosition();
    show();

    return;
}

void CustomWaterMaskFrame::onSystemMaskShow(bool showEnable, QPoint pos)
{
    systemMaskEnable = showEnable;
    systemMaskPosition = pos;

    update();
    return;
}

QPixmap CustomWaterMaskFrame::maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio)
{
    QImageReader maskIimageReader(uri);
    const QSize &maskSize = size * pixelRatio;
    const QSize &maskImageSize = maskIimageReader.size();

    if (uri.endsWith(".svg") || (maskImageSize.width() >= maskSize.width()
                                         || maskImageSize.height() >= maskSize.height())) {
        maskIimageReader.setScaledSize(maskSize);
    } else {
        maskIimageReader.setScaledSize(size);
    }

    QPixmap maskPixmap = QPixmap::fromImage(maskIimageReader.read());
    maskPixmap.setDevicePixelRatio(pixelRatio);
    return maskPixmap;
}
