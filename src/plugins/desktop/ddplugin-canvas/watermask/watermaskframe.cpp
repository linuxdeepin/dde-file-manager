/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "watermaskframe.h"
#include "deepinlicensehelper.h"
#include "displayconfig.h"

#include <DSysInfo>

#include <QHBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImageReader>

using namespace ddplugin_canvas;
DCORE_USE_NAMESPACE

WaterMaskFrame::WaterMaskFrame(const QString &fileName, QWidget *parent)
    : QFrame(parent)
    , configFile(fileName)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    DeepinLicenseHelper::instance()->init();
    // 授权状态改变
    connect(DeepinLicenseHelper::instance(), &DeepinLicenseHelper::licenseStateChanged,
            this, &WaterMaskFrame::stateChanged);

    logoLabel = new QLabel(this);
    textLabel = new QLabel(this);
}

WaterMaskFrame::~WaterMaskFrame()
{

}

void WaterMaskFrame::refresh()
{
    loadConfig();
    update(currentConfig);
    updatePosition();
}

void WaterMaskFrame::updatePosition()
{
    if (auto wid = parentWidget()) {
        int x = wid->width() - currentConfig.xRightBottom - currentConfig.maskWidth;
        int y = wid->height() - currentConfig.yRightBottom - currentConfig.maskHeight;
        move(x, y);
    }
}

void WaterMaskFrame::stateChanged(int stateType)
{
    qInfo() << "reply ActiveState is" << stateType << this;
    if (!showLicenseState())
        return;

    switch (stateType) {
    case DeepinLicenseHelper::Unauthorized:
    case DeepinLicenseHelper::AuthorizedLapse:
    case DeepinLicenseHelper::TrialExpired: {
        textLabel->setText(tr("Not authorized"));
        textLabel->setObjectName(tr("Not authorized"));
    }
        break;
    case DeepinLicenseHelper::Authorized:
        textLabel->setText("");
        break;
    case DeepinLicenseHelper::TrialAuthorized:{
        textLabel->setText(tr("In trial period"));
        textLabel->setObjectName(tr("In trial period"));
    }
        break;
    default:
        qWarning() << "unkown active state:" << stateType;
    }
}

WaterMaskFrame::ConfigInfo WaterMaskFrame::parseJson(QJsonObject *configs)
{
    ConfigInfo cfg;
    if (!configs)
        return cfg;

    if (configs->contains("isMaskAlwaysOn"))
        cfg.maskAlwaysOn = configs->value("isMaskAlwaysOn").toBool(true);

    {
        QString maskLogoUri;
        if (DispalyIns->customWaterMask()) {
            if (configs->contains("maskLogoUri"))
                maskLogoUri = configs->value("maskLogoUri").toString();
        } else {
            maskLogoUri = DSysInfo::distributionOrgLogo(DSysInfo::OrgType::Distribution, DSysInfo::LogoType::Transparent);
        }

        if (maskLogoUri.startsWith("~/"))
            maskLogoUri.replace(0, 1, QDir::homePath());

        cfg.maskLogoUri = maskLogoUri;
    }

    if (configs->contains("maskLogoWidth"))
        cfg.maskLogoWidth = configs->value("maskLogoWidth").toInt();

    if (configs->contains("maskLogoHeight"))
        cfg.maskLogoHeight = configs->value("maskLogoHeight").toInt();

    if (configs->contains("maskLogoTextSpacing"))
        cfg.maskLogoTextSpacing = configs->value("maskLogoTextSpacing").toInt();

    if (configs->contains("maskHeight"))
        cfg.maskHeight = configs->value("maskHeight").toInt();

    if (configs->contains("xRightBottom"))
        cfg.xRightBottom = configs->value("xRightBottom").toInt();

    if (configs->contains("yRightBottom"))
        cfg.yRightBottom = configs->value("yRightBottom").toInt();

    cfg.maskWidth = cfg.maskLogoWidth + cfg.maskTextWidth;
    return cfg;
}

bool WaterMaskFrame::showLicenseState()
{
    DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
    DSysInfo::UosEdition uosEdition = DSysInfo::uosEditionType();
    qInfo() << "deepinType" << deepinType << "uosEditionType" << uosEdition;

    bool ret = (DSysInfo::DeepinType::DeepinProfessional == deepinType
                || DSysInfo::DeepinType::DeepinPersonal == deepinType
                || DSysInfo::DeepinType::DeepinServer == deepinType);

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 4, 7, 0))
    // 教育版
    ret = ret || DSysInfo::UosEdition::UosEducation == uosEdition;
    qInfo() << "check uos Edition" << ret;
#endif

    return ret;
}

void WaterMaskFrame::addWidget(QHBoxLayout *layout, QWidget *wid, const QString &align)
{
    if (align == "left") {
        layout->addWidget(wid, 0, Qt::AlignLeft | Qt::AlignVCenter);
    } else if (align == "right") {
        layout->addWidget(wid, 0, Qt::AlignRight | Qt::AlignVCenter);
    } else if (align == "center") {
        layout->addWidget(wid, 0, Qt::AlignCenter);
    }
}

void WaterMaskFrame::loadConfig()
{
    QFile file(configFile);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "WaterMask config file doesn't exist!";
        return ;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error == QJsonParseError::NoError) {
        auto jsonObject = QJsonObject::fromVariantMap(doc.toVariant().toMap());
        currentConfig = parseJson(&jsonObject);
    } else {
        // defalut
        currentConfig = ConfigInfo();
        qCritical() << "config file is invailid" << configFile << error.errorString();
    }
}

void WaterMaskFrame::update(const ConfigInfo &configs)
{
    if (!configs.maskAlwaysOn) {
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
    mainLayout->setSpacing(0);
    mainLayout->addStretch();

    // add logo at left
    logoLabel->setPixmap(maskPixmap(configs.maskLogoUri, QSize(configs.maskLogoWidth, configs.maskLogoHeight)
                                    , logoLabel->devicePixelRatioF()));
    if (configs.maskLogoUri.length() != 0)
        addWidget(mainLayout, logoLabel, "left");

    // space
    mainLayout->addSpacing(configs.maskLogoTextSpacing);

    // add text at right
    if (showLicenseState()) {
        qInfo() << "delayGetState";
        DeepinLicenseHelper::instance()->delayGetState();
        qInfo() << "called delayGetState";
        addWidget(mainLayout, textLabel, "right");
    } else {
        textLabel->clear();
    }
    textLabel->setFixedSize(configs.maskTextWidth, configs.maskTextHeight);
    setTextAlign("left"); // fix to left

    mainLayout->addStretch();
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->setParent(this);
    setLayout(mainLayout);
    setFixedSize(configs.maskWidth, configs.maskHeight);

    static const QString maskTextColor = "rgba(245,245,245,245.120)";
    static const QString maskTextFontSize = "12px";
    QString style = QString("QLabel {color: %1; font-size: %2}").arg(maskTextColor, maskTextFontSize);
    setStyleSheet(style);

    show();
}

void WaterMaskFrame::setTextAlign(const QString &maskTextAlign)
{
    if (maskTextAlign == "left") {
        textLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    } else if (maskTextAlign == "right") {
        textLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    } else if (maskTextAlign == "center") {
        textLabel->setAlignment(Qt::AlignCenter);
    }
}

QPixmap WaterMaskFrame::maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio)
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
