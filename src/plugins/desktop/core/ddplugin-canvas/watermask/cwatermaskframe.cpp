#include "cwatermaskframe.h"
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

static constexpr char kConfName[] { "org.deepin.dde.file-manager.desktop" };
static constexpr char kIsMaskOpen[] { "isMaskOpen" };
static constexpr char kMaskLogoUri[] { "maskLogoUri" };
static constexpr char kMaskOffsetX[] { "maskOffsetX" };
static constexpr char kMaskOffsetY[] { "maskOffsetY" };
static constexpr char kMaskWidth[] { "maskWidth" };
static constexpr char kMaskHeight[] { "maskHeight" };

CWaterMaskFrame::CWaterMaskFrame(QWidget *parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    logoLabel = new QLabel(this);
    //getSystemMaskPosision();

    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &CWaterMaskFrame::onConfigChanged, Qt::DirectConnection);
}

CWaterMaskFrame::~CWaterMaskFrame()
{

}

void CWaterMaskFrame::loadConfig()
{
   //QStringList keys = DConfigManager::instance()->keys(kConfName);

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

void CWaterMaskFrame::refresh()
{
    loadConfig();

    update();

    return;
}

void CWaterMaskFrame::setPosition()
{
    if (auto wid = parentWidget()) {
        int x = systemMaskPosition.x() + maskOffset.x();
        int y = systemMaskPosition.y() + maskOffset.y();

        move(x, y);
    }
}

void CWaterMaskFrame::onConfigChanged(const QString &cfg, const QString &key)
{
    if (cfg != QString(kConfName))
        return;

    refresh();
    return;
}

void CWaterMaskFrame::update()
{
    if (!systemMaskEnable) {
        hide();
        return;
    }

    if (!maskOpen) {
        hide();
        return;
    }

    logoLabel->setPixmap(maskPixmap(maskLogoUri, maskSize, logoLabel->devicePixelRatioF()));
    logoLabel->setFixedSize(maskSize.width(), maskSize.height());

    setPosition();
    show();
}

void CWaterMaskFrame::onSystemMaskShow(bool showEnable, QPoint pos, int height)
{
    systemMaskEnable = showEnable;
    systemMaskPosition = pos;

    //systemMaskHeight = height;
    DConfigManager::instance()->setValue(kConfName, kMaskOffsetY, height);

    update();
}

QPixmap CWaterMaskFrame::maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio)
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

//void CWaterMaskFrame::getSystemMaskPosision()
//{
//    QString systemMaskLogoUri = "/usr/share/deepin/dde-desktop-watermask.json";
//    QFile file(systemMaskLogoUri);
//    if (!file.open(QFile::ReadOnly)) {
//        return ;
//    }

//    QJsonParseError error;
//    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
//    if (error.error == QJsonParseError::NoError) {
//        auto jsonObject = QJsonObject::fromVariantMap(doc.toVariant().toMap());
//        if (jsonObject.contains("xRightBottom"))
//            systemMaskPosition.setX(jsonObject.value("xRightBottom").toInt());
//        if (jsonObject.contains("yRightBottom"))
//            systemMaskPosition.setY(jsonObject.value("yRightBottom").toInt());
//        if (jsonObject.contains("maskWidth"))
//            systemMaskSize.setWidth(jsonObject.value("maskWidth").toInt());
//        if (jsonObject.contains("maskHeight"))
//            systemMaskSize.setHeight(jsonObject.value("maskHeight").toInt());
//    }

//    return;
//}
//#include <QPainter>
//void CWaterMaskFrame::paintEvent(QPaintEvent *)
//{
//    QPainter pa(this);
//    pa.fillRect(0,0,width(),height(), Qt::green);
//}
