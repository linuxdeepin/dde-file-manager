// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screensaversettingplugin.h"
#include "screensaver/screensaverwindow.h"
#include "screensaver/screensaverprovider.h"

#include "desktoputils/screensavercheck.h"

using namespace dfm_wallpapersetting;
using namespace DCC_NAMESPACE;

ScreensaverSettingPlugin::ScreensaverSettingPlugin(QObject *parent)
    : QObject(parent)
    , ModuleInterface()
{
    QTranslator *translator = new QTranslator(this);
    translator->load(QString("/usr/share/dcc-wallpapersetting-plugin/translations/dcc-wallpapersetting-plugin_%1.qm").arg(QLocale::system().name()));
    QCoreApplication::installTranslator(translator);
}

void ScreensaverSettingPlugin::preInitialize(bool sync, FrameProxyInterface::PushType)
{
    if (provider) {
        qCritical() << "Repeated initialization" << name();
        return;
    }

    provider = new ScreensaverProvider(this);
    provider->fecthData();

    addChildPageTrans();
    initSearchData();
}

void ScreensaverSettingPlugin::initialize()
{

}

const QString ScreensaverSettingPlugin::name() const
{
    return QStringLiteral("ScreensaverSetting");
}

const QString ScreensaverSettingPlugin::displayName() const
{
    return tr("Screensaver");
}

QIcon ScreensaverSettingPlugin::icon() const
{
    return QIcon::fromTheme("icon_screensaver");
}

QString ScreensaverSettingPlugin::translationPath() const
{
    return QString(":/translations/dcc-wallpapersetting-plugin_%1.ts");
}

QString ScreensaverSettingPlugin::path() const
{
    // ineser to personalization
    return PERSONALIZATION;
}

QString ScreensaverSettingPlugin::follow() const
{
    return "WallpaperSetting";
}

void ScreensaverSettingPlugin::addChildPageTrans() const
{
    if (!m_frameProxy)
        return;

    // add menu info for search
    m_frameProxy->addChildPageTrans(name(), displayName());
}

QStringList ScreensaverSettingPlugin::availPage() const
{
    QStringList ret;
    ret << name();
    return ret;
}

bool ScreensaverSettingPlugin::enabled() const
{
    return ddplugin_desktop_util::enableScreensaver();
}

void ScreensaverSettingPlugin::initSearchData()
{
    if (!m_frameProxy)
        return;

    // set what info can be searched
    const QString &module = m_frameProxy->moduleDisplayName(PERSONALIZATION);

    // wallpaper menu
    m_frameProxy->setWidgetVisible(module, displayName(), true);

    // wallpaper setting widget
    m_frameProxy->setDetailVisible(module, displayName(), tr("Custom Screensaver"), true);
    m_frameProxy->setDetailVisible(module, displayName(), tr("Idle Time"), true);

    m_frameProxy->updateSearchData(module);
}

void ScreensaverSettingPlugin::active()
{
    auto window = new ScreensaverWindow;
    window->initialize(provider);
    window->reset();
    m_frameProxy->pushWidget(this, window);

    window->setVisible(true);
    window->scrollToCurrent();
}
