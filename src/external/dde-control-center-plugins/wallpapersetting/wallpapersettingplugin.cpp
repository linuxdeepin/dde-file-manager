// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpapersettingplugin.h"
#include "wallpaper/wallpaperprovider.h"

using namespace dfm_wallpapersetting;
using namespace DCC_NAMESPACE;

WallpaperSettingPlugin::WallpaperSettingPlugin(QObject *parent)
    : QObject(parent)
    , ModuleInterface()
{
    QTranslator *translator = new QTranslator(this);
    translator->load(QString("/usr/share/dcc-wallpapersetting-plugin/translations/dcc-wallpapersetting-plugin_%1.qm").arg(QLocale::system().name()));
    QCoreApplication::installTranslator(translator);
}

WallpaperSettingPlugin::~WallpaperSettingPlugin()
{

}

void WallpaperSettingPlugin::preInitialize(bool sync, FrameProxyInterface::PushType)
{
    if (provider) {
        qCritical() << "Repeated initialization" << name();
        return;
    }

    provider = new WallpaperProvider(this);
    provider->fecthData();
    addChildPageTrans();
    initSearchData();
    qInfo() << name() << "is preInitialized";
}

void WallpaperSettingPlugin::initialize()
{

}

const QString WallpaperSettingPlugin::name() const
{
    return QStringLiteral("WallpaperSetting");
}

const QString WallpaperSettingPlugin::displayName() const
{
    return tr("Wallpaper");
}

QIcon WallpaperSettingPlugin::icon() const
{
    return QIcon::fromTheme("icon_wallpaper");
}

QString WallpaperSettingPlugin::translationPath() const
{
    return QString(":/translations/dcc-wallpapersetting-plugin_%1.ts");
}

QString WallpaperSettingPlugin::path() const
{
    // ineser to personalization
    return PERSONALIZATION;
}

QString WallpaperSettingPlugin::follow() const
{
    return "Dock";
}

void WallpaperSettingPlugin::addChildPageTrans() const
{
    if (!m_frameProxy)
        return;

    // add menu info for search
    m_frameProxy->addChildPageTrans(name(), displayName());
}

QStringList WallpaperSettingPlugin::availPage() const
{
    QStringList ret;
    ret << name();
    return ret;
}

void WallpaperSettingPlugin::showPage(const QString &pageName)
{
}

void WallpaperSettingPlugin::initSearchData()
{
    if (!m_frameProxy)
        return;

    // set what info can be searched
    const QString &module = m_frameProxy->moduleDisplayName(PERSONALIZATION);

    // wallpaper menu
    m_frameProxy->setWidgetVisible(module, displayName(), true);

    // wallpaper setting widget
    m_frameProxy->setDetailVisible(module, displayName(), tr("Wallpaper Slideshow"), true);
    m_frameProxy->setDetailVisible(module, displayName(), tr("Picture"), true);
    m_frameProxy->setDetailVisible(module, displayName(), tr("Solid Color"), true);

    m_frameProxy->updateSearchData(module);
}

void WallpaperSettingPlugin::active()
{
    auto window = new WallpaperWindow;
    window->initialize(provider);
    window->reset();

    m_frameProxy->pushWidget(this, window);
    window->setVisible(true);

    window->scrollToCurrent();
}
