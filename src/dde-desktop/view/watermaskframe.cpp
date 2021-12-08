/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#include "../config/config.h"
#include "presenter/deepinlicensehelper.h"

#include <DSysInfo>
#include <QFile>
#include <QDebug>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QUrl>
#include <QDir>
#include <QImageReader>

#include "accessibility/ac-desktop-define.h"

DCORE_USE_NAMESPACE

WaterMaskFrame::WaterMaskFrame(const QString &fileName, QWidget *parent) :
    QFrame(parent),
    m_configFile(fileName)
{
    AC_SET_OBJECT_NAME( this, AC_WATER_MASK_FRAME);
    AC_SET_ACCESSIBLE_NAME( this, AC_WATER_MASK_FRAME);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    DeepinLicenseHelper::instance()->init();
    // 授权状态改变
    connect(DeepinLicenseHelper::instance(), &DeepinLicenseHelper::licenseStateChanged,
            this, &WaterMaskFrame::updateAuthorizationState);

    // 获取到当前授权状态
    connect(DeepinLicenseHelper::instance(), &DeepinLicenseHelper::postLicenseState,
            this, &WaterMaskFrame::onChangeAuthorizationLabel);

    //todo
    m_logoLabel = new QLabel(this);
    m_textLabel = new QLabel(this);

    AC_SET_OBJECT_NAME( m_logoLabel, AC_WATER_MASK_LOGO_LABEL);
    AC_SET_ACCESSIBLE_NAME( m_logoLabel, AC_WATER_MASK_LOGO_LABEL);
    AC_SET_OBJECT_NAME( m_textLabel, AC_WATER_MASK_TEXT);
    AC_SET_ACCESSIBLE_NAME( m_textLabel, AC_WATER_MASK_TEXT);

    bool isConfigFileExist = checkConfigFile(m_configFile);
    if (isConfigFileExist) {
        loadConfig(m_configFile);
    } else {
        hide();
    }
}

WaterMaskFrame::~WaterMaskFrame()
{
    if(m_logoLabel){
        m_logoLabel->deleteLater();
        m_logoLabel = nullptr;
    }

    if(m_textLabel){
        m_textLabel->deleteLater();
        m_textLabel = nullptr;
    }

    if (m_mainLayout) {
        m_mainLayout->deleteLater();
        m_mainLayout = nullptr;
    }
}

bool WaterMaskFrame::checkConfigFile(const QString &fileName)
{
    if (QFile(fileName).exists()) {
        return true;
    } else {
        return false;
    }
}

void WaterMaskFrame::loadConfig(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "WaterMask config file doesn't exist!";
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error == QJsonParseError::NoError) {
        m_configs = QJsonObject::fromVariantMap(doc.toVariant().toMap());
        initUI();
    } else {
        qCritical() << error.errorString();
    }

}

void WaterMaskFrame::initUI()
{
    if (m_configs.contains("isMaskAlwaysOn")) {
        m_isMaskAlwaysOn =  m_configs.value("isMaskAlwaysOn").toBool();
    }

    bool useJosn = Config::instance()->getConfig(Config::groupGeneral,Config::keyWaterMask, true).toBool();
    QString maskLogoUri;
    if(useJosn){
        if (m_configs.contains("maskLogoUri")) {
            maskLogoUri = m_configs.value("maskLogoUri").toString();
        } else {
            maskLogoUri.clear();
        }
    }
    else {
        maskLogoUri = DSysInfo::distributionOrgLogo(DSysInfo::OrgType::Distribution, DSysInfo::LogoType::Transparent);
    }

    if (maskLogoUri.startsWith("~/")) {
        maskLogoUri.replace(0, 1, QDir::homePath());
    }

//    QString maskLogoLayoutAlign;
//    if (m_configs.contains("maskLogoLayoutAlign")) {
//        maskLogoLayoutAlign = m_configs.value("maskLogoLayoutAlign").toString();
//    } else {
//        maskLogoLayoutAlign = "left";
//    }

    int maskLogoWidth;
    if (m_configs.contains("maskLogoWidth")) {
        maskLogoWidth = m_configs.value("maskLogoWidth").toInt();
    } else {
        maskLogoWidth = 128;
    }

    int maskLogoHeight;
    if (m_configs.contains("maskLogoHeight")) {
        maskLogoHeight = m_configs.value("maskLogoHeight").toInt();
    } else {
        maskLogoHeight = 48;
    }

//    QString maskText;
//    if (m_configs.contains("maskText")) {
//        maskText = m_configs.value("maskText").toString();
//    } else {
//        maskText = "";
//    }

//    QString maskTextLayoutAlign;
//    if (m_configs.contains("maskTextLayoutAlign")) {
//        maskTextLayoutAlign = m_configs.value("maskTextLayoutAlign").toString();
//    } else {
//        maskTextLayoutAlign = "right";
//    }

//    QString maskTextColor;
//    if (m_configs.contains("maskTextColor")) {
//        maskTextColor = m_configs.value("maskTextColor").toString();
//    } else {
//        maskTextColor = "rgba(245,245,245,245.130)";
//    }

//    QString maskTextFontSize;
//    if (m_configs.contains("maskTextFontSize")) {
//        maskTextFontSize = m_configs.value("maskTextFontSize").toString();
//    } else {
//        maskTextFontSize = "12px";
//    }


//    int maskTextWidth;
//    if (m_configs.contains("maskTextWidth")) {
//        maskTextWidth = m_configs.value("maskTextWidth").toInt();
//    } else {
//        maskTextWidth = 100;
//    }

//    int maskTextHeight;
//    if (m_configs.contains("maskTextHeight")) {
//        maskTextHeight = m_configs.value("maskTextHeight").toInt();
//    } else {
//        maskTextHeight = 30;
//    }

//    QString maskTextAlign;
//    if (m_configs.contains("maskTextAlign")) {
//        maskTextAlign = m_configs.value("maskTextAlign").toString();
//    } else {
//        maskTextAlign = "left";
//    }

    int maskLogoTextSpacing;
    if (m_configs.contains("maskLogoTextSpacing")) {
        maskLogoTextSpacing = m_configs.value("maskLogoTextSpacing").toInt();
    } else {
        maskLogoTextSpacing = 0;
    }

//    if (m_configs.contains("maskWidth")) {
//        m_maskWidth = m_configs.value("maskWidth").toInt();
//    } else {
//        m_maskWidth = 228;
//    }

    if (m_configs.contains("maskHeight")) {
        m_maskHeight = m_configs.value("maskHeight").toInt();
    } else {
        m_maskHeight = 48;
    }

    if (m_configs.contains("xRightBottom")) {
        m_xRightBottom = m_configs.value("xRightBottom").toInt();
    } else {
        m_xRightBottom = 50;
    }

    if (m_configs.contains("yRightBottom")) {
        m_yRightBottom = m_configs.value("yRightBottom").toInt();
    } else {
        m_yRightBottom = 98;
    }

    QString maskLogoLayoutAlign = "left";
    QString maskTextLayoutAlign = "right";
    QString maskTextColor = "rgba(245,245,245,245.120)";
    QString maskTextFontSize = "12px";
    int maskTextWidth = 100;
    int maskTextHeight = 30;
    QString maskTextAlign = "left";
    m_maskWidth = maskLogoWidth + maskTextWidth;

    {
        QImageReader mask_image_reader(maskLogoUri);
        const QSize &mask_size = QSize(maskLogoWidth, maskLogoHeight) * m_logoLabel->devicePixelRatioF();
        const QSize &mask_image_size = mask_image_reader.size();

        if (maskLogoUri.endsWith(".svg") || (mask_image_size.width() >= mask_size.width()
                                             && mask_image_size.height() >= mask_size.height())) {
            mask_image_reader.setScaledSize(mask_size);
        } else {
            mask_image_reader.setScaledSize(QSize(maskLogoWidth, maskLogoHeight));
        }

        QPixmap mask_pixmap = QPixmap::fromImage(mask_image_reader.read());
        mask_pixmap.setDevicePixelRatio(m_logoLabel->devicePixelRatioF());

        m_logoLabel->setPixmap(mask_pixmap);
    }

    if (isNeedState()) {
        qInfo() << "delayGetState";
        DeepinLicenseHelper::instance()->delayGetState();
        qInfo() << "called delayGetState";
    } else {
        m_textLabel->setText("");
    }

    m_textLabel->setFixedSize(maskTextWidth, maskTextHeight);

    if (maskTextAlign == "left") {
        m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    } else if (maskTextAlign == "right") {
        m_textLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    } else if (maskTextAlign == "center") {
        m_textLabel->setAlignment(Qt::AlignCenter);
    }

    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->addStretch();

    if (maskLogoUri.length() != 0) {
        if (maskLogoLayoutAlign == "left") {
            m_mainLayout->addWidget(m_logoLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
        } else if (maskLogoLayoutAlign == "right") {
            m_mainLayout->addWidget(m_logoLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
        } else if (maskLogoLayoutAlign == "center") {
            m_mainLayout->addWidget(m_logoLabel, 0, Qt::AlignCenter);
        }
    }

    m_mainLayout->addSpacing(maskLogoTextSpacing);

    if (isNeedState()) {
        if (maskTextLayoutAlign == "left") {
            m_mainLayout->addWidget(m_textLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
        } else if (maskTextLayoutAlign == "right") {
            m_mainLayout->addWidget(m_textLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
        } else if (maskTextLayoutAlign == "center") {
            m_mainLayout->addWidget(m_textLabel, 0, Qt::AlignCenter);
        }
    }
    m_mainLayout->addStretch();
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    /*initUI会new一个QHBoxLayout，WaterMaskFrame构造时已经调用了一次，
     * 后续多次同一对象调用initUI会导致泄露，由于updateAuthorizationState会间接调用到initUI,
     * 因此需要删掉之前的布局之后再调用*/
    auto tempLayout = this->layout();
    if (tempLayout) {
        delete tempLayout;
        tempLayout = nullptr;
    }
    setLayout(m_mainLayout);
    setFixedSize(m_maskWidth, m_maskHeight);
    QString color(maskTextColor);
    QString fontsize(maskTextFontSize);
    QString style = QString("QLabel {color: %1; font-size: %2}").arg(color, fontsize);
    setStyleSheet(style);
}

bool WaterMaskFrame::isNeedState()
{
    DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
    DSysInfo::UosEdition uosEdition = DSysInfo::uosEditionType();
    qInfo() << "deepinType" << deepinType << "uosEditionType" << uosEdition;

    bool ret = (DSysInfo::DeepinType::DeepinProfessional == deepinType
                || DSysInfo::DeepinType::DeepinPersonal == deepinType
                || DSysInfo::DeepinType::DeepinServer == deepinType);

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 4, 7, 0))
    // 教育版不需要水印
    ret = ret || DSysInfo::UosEdition::UosEducation == uosEdition;
    qInfo() << "check uos Edition" << ret;
#endif

    return ret;
}

bool WaterMaskFrame::parseJson(QString key)
{
    if(key.isNull() || key.isEmpty() || (!m_configs.contains(key))){
        qWarning() << key <<"WaterMask load Config fail";
        return false;
    }
    else {
        m_configs = m_configs.value(key).toObject();
        return true;
    }
}

void WaterMaskFrame::updatePosition()
{
    int x = static_cast<QWidget *>(parent())->width() - m_xRightBottom - m_maskWidth;
    int y = static_cast<QWidget *>(parent())->height() - m_yRightBottom - m_maskHeight;
    move(x, y);
}

void WaterMaskFrame::updateAuthorizationState()
{
    qInfo() << "received com.deepin.license.Info::LicenseStateChange.";
    bool isConfigFileExist = checkConfigFile(m_configFile);
    qInfo() << "isConfigFileExist:" << isConfigFileExist << "   configFile:" << m_configFile;
    if (isConfigFileExist) {
        loadConfig(m_configFile);
    }
}

void WaterMaskFrame::onChangeAuthorizationLabel(int stateType)
{
    qInfo() << "reply ActiveState is" << stateType << this;

    switch (stateType) {
    case Unauthorized:
    case AuthorizedLapse:
    case TrialExpired: {
        m_textLabel->setText(tr("Not authorized"));
        m_textLabel->setObjectName(tr("Not authorized"));
        AC_SET_ACCESSIBLE_NAME(m_textLabel, AC_WATER_TEXT_LABEL_NO_AUTHORIZED);
    }
        break;
    case Authorized:
        //2020-07-06 需求变更，已授权不显示
        //m_textLabel->setText(tr("authorized"));
        m_textLabel->setText("");
        break;
    case TrialAuthorized:{
        m_textLabel->setText(tr("In trial period"));
        m_textLabel->setObjectName(tr("In trial period"));
        AC_SET_ACCESSIBLE_NAME(m_textLabel, AC_WATER_TEXT_LABEL_IN_TRIAL);
    }
        break;
    default:
        qWarning() << "unkown active state:" << stateType;
    }
}
