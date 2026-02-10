// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "watermaskframe.h"
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
#include <QFont>

using namespace ddplugin_canvas;
DCORE_USE_NAMESPACE

static const char *CfgDefault = "default";
static const char *CfgGovEn = "gov-en";
static const char *CfgGovCn = "gov-cn";
static const char *CfgEntEn = "ent-en";
static const char *CfgEntCn = "ent-cn";
static const char *CfgSecEn = "sec-en";
static const char *CfgSecCn = "sec-cn";


WaterMaskFrame::WaterMaskFrame(const QString &fileName, QWidget *parent)
    : QFrame(parent)
    , configFile(fileName)
{
    fmInfo() << "Initializing WaterMaskFrame with config file:" << fileName;
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    DeepinLicenseHelper::instance()->init();
    // 授权状态改变
    connect(DeepinLicenseHelper::instance(), &DeepinLicenseHelper::postLicenseState,
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

    fmDebug() << "Requesting license state update";
    DeepinLicenseHelper::instance()->delayGetState();
}

void WaterMaskFrame::updatePosition()
{
    if (auto wid = parentWidget()) {
        int x = wid->width() - curRightBottom.x() - curMaskSize.width();
        int y = wid->height() - curRightBottom.y() - curMaskSize.height();
        move(x, y);

        fmDebug() << "Water mask position updated to:" << QPoint(x, y);
        emit showMask(QPoint(x, y));
    }
}

void WaterMaskFrame::stateChanged(int state, int prop)
{
    if (!maskAlwaysOn) {
        fmDebug() << "Water mask disabled - hiding frame";
        hide();
        return;
    }

    bool showSate = showLicenseState();
    bool cn = usingCn();
    fmInfo() << "Processing license state - show:" << showSate << "locale_cn:" << cn;
    curState = state;
    curProperty = static_cast<DeepinLicenseHelper::LicenseProperty>(prop);

    // 已授权, 判断版本
    if (state == DeepinLicenseHelper::Authorized && showSate) {
        if (prop == DeepinLicenseHelper::LicenseProperty::Secretssecurity) {
            auto cfg = configInfos.value(cn ? CfgSecCn : CfgSecEn);
            if (cfg.valid) {
                fmInfo() << "Applying secrets security configuration - locale_cn:" << cn;
                update(cfg, false);
                return;
            } else {
                fmWarning() << "Invalid secrets security config - locale_cn:" << cn;
            }
        } else if (prop == DeepinLicenseHelper::LicenseProperty::Government) {   //gov
            auto cfg = configInfos.value(cn ? CfgGovCn : CfgGovEn);
            if (cfg.valid) {
                fmInfo() << "Applying government configuration - locale_cn:" << cn;
                update(cfg, false);
                return;
            } else {
                fmWarning() << "Invalid government config - locale_cn:" << cn;
            }
        } else if (prop == DeepinLicenseHelper::LicenseProperty::Enterprise) {   // com
            auto cfg = configInfos.value(cn ? CfgEntCn : CfgEntEn);
            if (cfg.valid) {
                fmInfo() << "Applying enterprise configuration - locale_cn:" << cn;
                update(cfg, false);
                return;
            } else {
                fmWarning() << "Invalid enterprise config - locale_cn:" << cn;
            }
        }
    }

   auto cfg = configInfos.value(CfgDefault);
   update(cfg, true);

   if (!showSate) {
       textLabel->setText("");
       fmDebug() << "License state display disabled";
       return;
   }

    switch (state) {
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
        fmWarning() << "Unknown license state:" << state;
    }
}

QMap<QString, WaterMaskFrame::ConfigInfo> WaterMaskFrame::parseJson(QJsonObject *configs)
{
    QMap<QString, WaterMaskFrame::ConfigInfo> ret;

//    if (configs->contains("isMaskAlwaysOn"))
//        maskAlwaysOn = configs->value("isMaskAlwaysOn").toBool(true);
    //! WaterMaskFrame is created if isMaskAlwaysOn is true.
    maskAlwaysOn = true;

    {
        ConfigInfo cfg = defaultCfg(configs);
        ret.insert(CfgDefault, cfg);
    }

    {
        ConfigInfo cfgGovCn = govCfg(configs, true);
        ret.insert(CfgGovCn, cfgGovCn);
    }

    {
        ConfigInfo cfgGovEn = govCfg(configs, false);
        ret.insert(CfgGovEn, cfgGovEn);
    }

    {
        ConfigInfo cfgComCn = entCfg(configs, true);
        ret.insert(CfgEntCn, cfgComCn);
    }

    {
        ConfigInfo cfgComEn = entCfg(configs, false);
        ret.insert(CfgEntEn, cfgComEn);
    }

    {
        ConfigInfo cfgSecCn = secCfg(configs, true);
        ret.insert(CfgSecCn, cfgSecCn);
    }

    {
        ConfigInfo cfgSecEn = secCfg(configs, false);
        ret.insert(CfgSecEn, cfgSecEn);
    }

    return ret;
}

bool WaterMaskFrame::showLicenseState()
{
    DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
    DSysInfo::UosEdition uosEdition = DSysInfo::uosEditionType();
    fmDebug() << "System info - deepinType:" << static_cast<int>(deepinType) << "uosEdition:" << static_cast<int>(uosEdition);

    bool ret = (DSysInfo::DeepinType::DeepinProfessional == deepinType
                || DSysInfo::DeepinType::DeepinPersonal == deepinType
                || DSysInfo::DeepinType::DeepinServer == deepinType);

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 4, 7, 0))
    // 教育版
    ret = ret || (DSysInfo::UosEdition::UosEducation == uosEdition ||
                  DSysInfo::UosEdition::UosMilitary == uosEdition);
    fmDebug() << "Extended edition check result:" << ret;
#endif

    fmDebug() << "License state display enabled:" << ret;
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

bool WaterMaskFrame::usingCn()
{
    static const QSet<QString> cn = {"zh_CN", "zh_TW", "zh_HK", "ug_CN", "bo_CN"};
    return cn.contains(QLocale::system().name().simplified());
}

void WaterMaskFrame::loadConfig()
{
    fmDebug() << "Loading water mask configuration from:" << configFile;
    QFile file(configFile);
    if (!file.open(QFile::ReadOnly)) {
        fmWarning() << "Water mask config file doesn't exist:" << configFile;
        return ;
    }
    configInfos.clear();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error == QJsonParseError::NoError) {
        auto jsonObject = QJsonObject::fromVariantMap(doc.toVariant().toMap());
        configInfos = parseJson(&jsonObject);
        fmInfo() << "Water mask configuration loaded successfully";
    } else {
        // defalut
        configInfos.insert(CfgDefault, ConfigInfo());
        fmCritical() << "Invalid water mask config file" << configFile << "- error:" << error.errorString();
    }
}

void WaterMaskFrame::update(const ConfigInfo &configs, bool normal)
{
    fmDebug() << "Updating water mask display - normal mode:" << normal;

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
    if (configs.maskLogoUri.length() != 0) {
        addWidget(mainLayout, logoLabel, "left");
        fmDebug() << "Logo added - URI:" << configs.maskLogoUri << "size:" << QSize(configs.maskLogoWidth, configs.maskLogoHeight);
    }

    // space
    mainLayout->addSpacing(configs.maskLogoTextSpacing);

    // add text at right
    if (normal)
        addWidget(mainLayout, textLabel, "right");

    textLabel->clear();
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

    curMaskSize = QSize(configs.maskWidth, configs.maskHeight);
    curRightBottom = QPoint(configs.xRightBottom, configs.yRightBottom);
    updatePosition();

    if (maskAlwaysOn) {
        show();
        fmDebug() << "Water mask frame displayed";
    }
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

WaterMaskFrame::ConfigInfo WaterMaskFrame::defaultCfg(QJsonObject *configs)
{
    fmDebug() << "Loading default configuration";
    ConfigInfo cfg;
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
        fmDebug() << "Default logo URI:" << maskLogoUri;
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
    cfg.valid = true;
    return cfg;
}

WaterMaskFrame::ConfigInfo WaterMaskFrame::govCfg(QJsonObject *configs, bool cn)
{
    fmDebug() << "Loading government configuration - locale_cn:" << cn;
    ConfigInfo cfg;
    {
        const QString urlKey = cn ? "maskLogoGovernmentCnUri" : "maskLogoGovernmentEnUri";
        QString maskLogoUri;
        if (configs->contains(urlKey))
            maskLogoUri = configs->value(urlKey).toString();

        if (maskLogoUri.startsWith("~/"))
            maskLogoUri.replace(0, 1, QDir::homePath());

        if (maskLogoUri.isEmpty()) {
            fmWarning() << "Cannot get government logo - locale_cn:" << cn;
            cfg.valid = false;
            return cfg;
        } else {
            cfg.maskLogoUri = maskLogoUri;
            fmDebug() << "Government logo URI:" << maskLogoUri;
        }
    }

    cfg.maskLogoTextSpacing = 0;

    if (configs->contains("maskLogoWidth"))
        cfg.maskLogoWidth = configs->value("maskLogoWidth").toInt();

    if (configs->contains("maskLogoHeight"))
        cfg.maskLogoHeight = configs->value("maskLogoHeight").toInt();

//    if (configs->contains("maskLogoTextSpacing"))
//        cfg.maskLogoTextSpacing = configs->value("maskLogoTextSpacing").toInt();

    if (configs->contains("maskHeight"))
        cfg.maskHeight = configs->value("maskHeight").toInt();

    if (configs->contains("xRightBottom"))
        cfg.xRightBottom = configs->value("xRightBottom").toInt();

    if (configs->contains("yRightBottom"))
        cfg.yRightBottom = configs->value("yRightBottom").toInt();

    cfg.maskWidth = cfg.maskLogoWidth + cfg.maskTextWidth;
    cfg.valid = true;
    return cfg;
}

WaterMaskFrame::ConfigInfo WaterMaskFrame::entCfg(QJsonObject *configs, bool cn)
{
    fmDebug() << "Loading enterprise configuration - locale_cn:" << cn;
    ConfigInfo cfg;
    {
        const QString urlKey = cn ? "maskLogoEnterpriseCnUri" : "maskLogoEnterpriseEnUri";
        QString maskLogoUri;
        if (configs->contains(urlKey))
            maskLogoUri = configs->value(urlKey).toString();

        if (maskLogoUri.startsWith("~/"))
            maskLogoUri.replace(0, 1, QDir::homePath());

        if (maskLogoUri.isEmpty()) {
            fmWarning() << "Cannot get enterprise logo - locale_cn:" << cn;
            cfg.valid = false;
            return cfg;
        } else {
            cfg.maskLogoUri = maskLogoUri;
            fmDebug() << "Enterprise logo URI:" << maskLogoUri;
        }
    }

    cfg.maskLogoTextSpacing = 0;

    if (configs->contains("maskLogoWidth"))
        cfg.maskLogoWidth = configs->value("maskLogoWidth").toInt();

    if (configs->contains("maskLogoHeight"))
        cfg.maskLogoHeight = configs->value("maskLogoHeight").toInt();

//    if (configs->contains("maskLogoTextSpacing"))
//        cfg.maskLogoTextSpacing = configs->value("maskLogoTextSpacing").toInt();

    if (configs->contains("maskHeight"))
        cfg.maskHeight = configs->value("maskHeight").toInt();

    if (configs->contains("xRightBottom"))
        cfg.xRightBottom = configs->value("xRightBottom").toInt();

    if (configs->contains("yRightBottom"))
        cfg.yRightBottom = configs->value("yRightBottom").toInt();

    cfg.maskWidth = cfg.maskLogoWidth + cfg.maskTextWidth;
    cfg.valid = true;
    return cfg;
}

WaterMaskFrame::ConfigInfo WaterMaskFrame::secCfg(QJsonObject *configs, bool cn)
{
    fmDebug() << "Loading secrets security configuration - locale_cn:" << cn;
    ConfigInfo cfg;
    {
        const QString urlKey = cn ? "maskLogoSecretesSecurityCnUri" : "maskLogoSecretesSecurityEnUri";
        QString maskLogoUri;
        if (configs->contains(urlKey))
            maskLogoUri = configs->value(urlKey).toString();

        if (maskLogoUri.startsWith("~/"))
            maskLogoUri.replace(0, 1, QDir::homePath());

        if (maskLogoUri.isEmpty()) {
            fmWarning() << "Cannot get secrets security logo - locale_cn:" << cn;
            cfg.valid = false;
            return cfg;
        } else {
            cfg.maskLogoUri = maskLogoUri;
            fmDebug() << "Secrets security logo URI:" << maskLogoUri;
        }
    }

    cfg.maskLogoTextSpacing = 0;

    if (configs->contains("maskLogoWidth"))
        cfg.maskLogoWidth = configs->value("maskLogoWidth").toInt();

    if (configs->contains("maskLogoHeight"))
        cfg.maskLogoHeight = configs->value("maskLogoHeight").toInt();

//    if (configs->contains("maskLogoTextSpacing"))
//        cfg.maskLogoTextSpacing = configs->value("maskLogoTextSpacing").toInt();

    if (configs->contains("maskHeight"))
        cfg.maskHeight = configs->value("maskHeight").toInt();

    if (configs->contains("xRightBottom"))
        cfg.xRightBottom = configs->value("xRightBottom").toInt();

    if (configs->contains("yRightBottom"))
        cfg.yRightBottom = configs->value("yRightBottom").toInt();

    cfg.maskWidth = cfg.maskLogoWidth + cfg.maskTextWidth;
    cfg.valid = true;
    return cfg;
}

QPixmap WaterMaskFrame::maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio)
{
    fmDebug() << "Creating mask pixmap - URI:" << uri << "size:" << size << "ratio:" << pixelRatio;
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
