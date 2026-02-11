// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "watermasksystem.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DSysInfo>

#include <QFileInfo>
#include <QImageReader>
#include <QLabel>

using namespace ddplugin_canvas;
using namespace dfmbase;
DCORE_USE_NAMESPACE

static constexpr char kConfName[] = "org.deepin.dde.file-manager.desktop.sys-watermask";
static constexpr char kDefaults[] = "defaults";

WatermaskSystem::WatermaskSystem(QWidget *parent) : QObject(parent)
{
    DeepinLicenseHelper::instance()->init();

    // 授权状态改变
    connect(DeepinLicenseHelper::instance(), &DeepinLicenseHelper::postLicenseState,
            this, &WatermaskSystem::stateChanged);

    logoLabel = new QLabel(parent);
    logoLabel->lower();
    logoLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    textLabel = new QLabel(parent);
    textLabel->lower();
    textLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

bool WatermaskSystem::isEnable()
{
    QFileInfo info("/usr/share/deepin/dde-desktop-watermask");
    return info.isReadable();
}

bool WatermaskSystem::usingCn()
{
    static const QSet<QString> cn = {"zh_CN", "zh_TW", "zh_HK", "ug_CN", "bo_CN"};
    return cn.contains(QLocale::system().name().simplified());
}

bool WatermaskSystem::showLicenseState()
{
    DSysInfo::DeepinType deepinType = DSysInfo::deepinType();
    DSysInfo::UosEdition uosEdition = DSysInfo::uosEditionType();
    fmInfo() << "deepinType" << deepinType << "uosEditionType" << uosEdition;

    bool ret = (DSysInfo::DeepinType::DeepinProfessional == deepinType
                || DSysInfo::DeepinType::DeepinPersonal == deepinType
                || DSysInfo::DeepinType::DeepinServer == deepinType);

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 4, 7, 0))
    // 教育版
    ret = ret || (DSysInfo::UosEdition::UosEducation == uosEdition ||
                  DSysInfo::UosEdition::UosMilitary == uosEdition);
    fmInfo() << "check uos Edition" << ret;
#endif

    fmDebug() << "License state should be shown:" << ret;
    return ret;
}

void WatermaskSystem::getEditonResource(const QString &root, QString *logo, QString *text)
{
    if (root.isEmpty() || (logo == nullptr && text == nullptr)) {
        fmWarning() << "Invalid parameters for getEditonResource, root:" << root;
        return;
    }

    fmDebug() << "Getting edition resources for root:" << root;

    QString lang = QLocale::system().name().simplified();
    const QString cn = "zh_CN";
    QString tmpLogo;
    QString tmpText;
    getResource(root, lang, &tmpLogo , &tmpText);

    // find cn
    if (lang != cn && usingCn())
        getResource(root, cn, tmpLogo.isEmpty() ? &tmpLogo : nullptr, tmpText.isEmpty() ? &tmpText : nullptr);

    // resource without lang in root
    getResource(root, QString(), tmpLogo.isEmpty() ? &tmpLogo : nullptr, tmpText.isEmpty() ? &tmpText : nullptr);

    // cd to defaluts
    if (root != kDefaults) {
        //! do not get default text
        // find defaluts with lang
        getResource(kDefaults, lang, tmpLogo.isEmpty() ? &tmpLogo : nullptr, nullptr);

        // find defaluts cn
        if (lang != cn && usingCn()) {
            getResource(kDefaults, cn, tmpLogo.isEmpty() ? &tmpLogo : nullptr,  nullptr);
        }

        // find defaluts
        getResource(kDefaults, QString(), tmpLogo.isEmpty() ? &tmpLogo : nullptr, nullptr);
    }

    if (logo)
        *logo = tmpLogo;

    if (text)
        *text = tmpText;

    fmDebug() << "Edition resources found - logo:" << tmpLogo << "text:" << tmpText;
}

QPixmap WatermaskSystem::maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio)
{
    if (uri.isEmpty()) {
        fmWarning() << "Empty URI provided for mask pixmap";
        return {};
    }

    fmDebug() << "Loading mask pixmap from:" << uri << "size:" << size << "pixelRatio:" << pixelRatio;

    QImageReader maskIimageReader(uri);
    const QSize &maskSize = size * pixelRatio;

    maskIimageReader.setScaledSize(maskSize);

    QPixmap maskPixmap = QPixmap::fromImage(maskIimageReader.read());
    maskPixmap.setDevicePixelRatio(pixelRatio);

    fmDebug() << "Successfully loaded mask pixmap from:" << uri;
    return maskPixmap;
}

void WatermaskSystem::getResource(const QString &root, const QString &lang, QString *logo, QString *text)
{
    if (root.isEmpty() || (logo == nullptr && text == nullptr)) {
        fmWarning() << "Invalid parameters for getResource, root:" << root;
        return;
    }

    QString path = QString("/usr/share/deepin/dde-desktop-watermask/") + root;
    fmDebug() << "Getting resource from path:" << path << "language:" << lang;

    QString tmpLogo;
    QString tmpText;

    findResource(path, lang, logo ? &tmpLogo : nullptr, text ? &tmpText : nullptr);

    if (logo)
        *logo = tmpLogo;

    if (text)
        *text = tmpText;
}

void WatermaskSystem::refresh()
{
    loadConfig();

    fmInfo() << "request state..";
    DeepinLicenseHelper::instance()->delayGetState();
}

void WatermaskSystem::stackUnder(QWidget *w)
{
    if (w) {
        logoLabel->stackUnder(w);
        textLabel->stackUnder(w);
    }
}

void WatermaskSystem::updatePosition()
{
    fmDebug() << "Updating watermask position";

    {
        int right = DConfigManager::instance()->value(kConfName, "logoRight", 160).toInt();
        int bottom = DConfigManager::instance()->value(kConfName, "logoBottom", 98).toInt();

        QSize pSize = parentWidget()->size();
        int x = pSize.width() - right - logoLabel->width();
        int y = pSize.height() - bottom - logoLabel->height();
        logoLabel->move(x, y);

        fmDebug() << "Logo position updated to:" << QPoint(x, y) << "parent size:" << pSize;
    }

    QPoint org = logoLabel->geometry().topLeft();

    {
        int w = DConfigManager::instance()->value(kConfName, "textWidth", 100).toInt();
        int h = DConfigManager::instance()->value(kConfName, "textHeight", 30).toInt();
        textLabel->setFixedSize(w, h);

        int offsetX = DConfigManager::instance()->value(kConfName, "textXPos", logoLabel->width()).toInt();
        int offsetY = DConfigManager::instance()->value(kConfName, "textYPos", 0).toInt();

        int x = org.x() + offsetX;
        int y = org.y() + offsetY;
        textLabel->move(x, y);

        fmDebug() << "Text position updated to:" << QPoint(x, y) << "size:" << QSize(w, h);
    }

    emit showedOn(org);
}

void WatermaskSystem::findResource(const QString &dirPath, const QString &lang, QString *logo, QString *text)
{
    if (dirPath.isEmpty() || (logo == nullptr && text == nullptr)) {
        fmWarning() << "Invalid parameters for findResource, dirPath:" << dirPath;
        return;
    }

    fmDebug() << "Finding resources in directory:" << dirPath << "language:" << lang;

    if (logo) {
        QString path = lang.isEmpty() ? QString("logo.svg") : QString("logo_%0.svg").arg(lang);
        QFileInfo file(dirPath + "/" + path);
        if (file.isReadable()) {
            *logo = file.absoluteFilePath();
            fmDebug() << "Found logo resource:" << *logo;
        } else {
            fmDebug() << "Logo resource not found:" << file.absoluteFilePath();
        }
    }

    if (text) {
        QString path = lang.isEmpty() ? QString("label.svg") : QString("label_%0.svg").arg(lang);
        QFileInfo file(dirPath + "/" + path);
        if (file.isReadable()) {
            *text = file.absoluteFilePath();
            fmDebug() << "Found text resource:" << *text;
        } else {
            fmDebug() << "Text resource not found:" << file.absoluteFilePath();
        }
    }
}

void WatermaskSystem::stateChanged(int state, int prop)
{
    bool showSate = showLicenseState();
    fmInfo() << "License state changed - state:" << state << "property:" << prop << "showState:" << showSate << "locale:" << QLocale::system().name().simplified();
    static QMap<int, QString> docs = {
        {DeepinLicenseHelper::LicenseProperty::Secretssecurity, QString("secretssecurity")},
        {DeepinLicenseHelper::LicenseProperty::Government, QString("government")},
        {DeepinLicenseHelper::LicenseProperty::Enterprise, QString("enterprise")},
        {DeepinLicenseHelper::LicenseProperty::Office, QString("office")},
        {DeepinLicenseHelper::LicenseProperty::BusinessSystem, QString("businesssystem")},
        {DeepinLicenseHelper::LicenseProperty::Equipment, QString("equipment")},
    };

    textLabel->setText("");
    textLabel->setPixmap(QPixmap());
    logoLabel->setPixmap(QPixmap());

    if (state == DeepinLicenseHelper::Authorized) {
        const QString doc = docs.value(prop, QString(kDefaults));
        fmInfo() << "System is authorized, using document type:" << doc;

        // find editon
        QString logo;
        QString text;
        getEditonResource(doc, &logo, showSate ? &text : nullptr);

        logoLabel->setPixmap(maskPixmap(logo, logoLabel->size(), logoLabel->devicePixelRatioF()));
        if (showSate)
            textLabel->setPixmap(maskPixmap(text, textLabel->size(), textLabel->devicePixelRatioF()));
    } else {
        fmInfo() << "System is not authorized, state:" << state;

        QString logo;
        getEditonResource(QString(kDefaults), &logo, nullptr);
        logoLabel->setPixmap(maskPixmap(logo, logoLabel->size(), logoLabel->devicePixelRatioF()));

        if (showSate) {
            switch (state) {
            case DeepinLicenseHelper::Unauthorized:
            case DeepinLicenseHelper::AuthorizedLapse:
            case DeepinLicenseHelper::TrialExpired: {
                textLabel->setText(tr("Not authorized"));
                textLabel->setObjectName(tr("Not authorized"));
                fmInfo() << "Set unauthorized text for state:" << state;
            }
                break;
            case DeepinLicenseHelper::TrialAuthorized:{
                textLabel->setText(tr("In trial period"));
                textLabel->setObjectName(tr("In trial period"));
                fmInfo() << "Set trial period text";
            }
                break;
            default:
                fmWarning() << "Unknown license state:" << state;
            }
        }
    }

    logoLabel->setVisible(true);
    textLabel->setVisible(showSate);

    fmInfo() << "Watermask visibility updated - logo: true, text:" << showSate;
    emit showedOn(logoLabel->geometry().topLeft());
}

void WatermaskSystem::loadConfig()
{
    // logo geometry
    {
        int w = DConfigManager::instance()->value(kConfName, "logoWidth", 114).toInt();
        int h = DConfigManager::instance()->value(kConfName, "logoHeight", 30).toInt();
        logoLabel->setFixedSize(w, h);

        int right = DConfigManager::instance()->value(kConfName, "logoRight", 160).toInt();
        int bottom = DConfigManager::instance()->value(kConfName, "logoBottom", 98).toInt();

        QSize pSize = parentWidget()->size();
        int x = pSize.width() - right - w;
        int y = pSize.height() - bottom - h;
        logoLabel->move(x, y);
    }

    // text geometry
    {
        QPoint org = logoLabel->geometry().topLeft();

        int w = DConfigManager::instance()->value(kConfName, "textWidth", 100).toInt();
        int h = DConfigManager::instance()->value(kConfName, "textHeight", 30).toInt();
        textLabel->setFixedSize(w, h);

        int offsetX = DConfigManager::instance()->value(kConfName, "textXPos", logoLabel->width()).toInt();
        int offsetY = DConfigManager::instance()->value(kConfName, "textYPos", 0).toInt();

        int x = org.x() + offsetX;
        int y = org.y() + offsetY;
        textLabel->move(x, y);

        const QString defColor = "#F5F5F5F5";
        QString colorStr = DConfigManager::instance()->value(kConfName, "textColor", defColor).toString();
        QColor color(colorStr.isEmpty() ? defColor : colorStr);
        auto pa = textLabel->palette();
        pa.setColor(textLabel->foregroundRole(), color); // text color
        textLabel->setPalette(pa);

        auto font = textLabel->font();
        int fontSize = DConfigManager::instance()->value(kConfName, "textFontSize", 11).toInt();
        font.setPixelSize(fontSize > 0 ? fontSize : 11);
        textLabel->setFont(font);

        int defAlign = Qt::AlignBottom | Qt::AlignLeft;
        int align = DConfigManager::instance()->value(kConfName, "textAlign", defAlign).toInt();
        textLabel->setAlignment(static_cast<Qt::Alignment>(align < 1 ? defAlign : align));
    }
}
