// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallaperpreview.h"

#include "desktoputils/widgetutil.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"

DFMBASE_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;

WallaperPreview::WallaperPreview(QObject *parent)
    : QObject(parent)
{

    fmDebug() << "create org.deepin.dde.Appearance1";
    inter = new BackgroudInter("org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1",
                          QDBusConnection::sessionBus(), this);
    inter->setTimeout(1000);
    fmDebug() << "create org.deepin.dde.Appearance1 end";

}

WallaperPreview::~WallaperPreview()
{

}

void WallaperPreview::init()
{
    fmInfo() << "Starting WallaperPreview initialization";
    pullImageSettings();
    buildWidgets();
    updateWallpaper();
    fmInfo() << "WallaperPreview initialization completed";
}

void WallaperPreview::setVisible(bool v)
{
    visible = v;
    for (PreviewWidgetPtr wid : previewWidgets.values())
        wid->setVisible(v);
}

void WallaperPreview::pullImageSettings()
{
    wallpapers.clear();
    for (ScreenPointer sc : ddplugin_desktop_util::screenProxyLogicScreens()) {
        QString path = getBackground(sc->name());
        wallpapers.insert(sc->name(), path);
    }
}

void WallaperPreview::updateWallpaper()
{
    QMap<QString, QString> recorder; // 记录有效的壁纸
    for (auto screenName : previewWidgets.keys()) {
        QString userPath;
        if (!wallpapers.contains(screenName)) {
            userPath = getBackground(screenName);
        } else {
            userPath = wallpapers.value(screenName);
        }

        if (userPath.isEmpty()) {
            fmWarning() << "Empty wallpaper path for screen:" << screenName;
            continue;
        }

        recorder.insert(screenName, userPath);

        PreviewWidgetPtr wid = previewWidgets.value(screenName);
        wid->setDisplay(userPath);
    }

    wallpapers = recorder;
}

void WallaperPreview::setWallpaper(const QString &screen, const QString &image)
{
    if (!screen.isEmpty() && !image.isEmpty())
        wallpapers.insert(screen, image);
    updateWallpaper();
}

void WallaperPreview::buildWidgets()
{
    DisplayMode mode = ddplugin_desktop_util::screenProxyLastChangedMode();
    auto screens = ddplugin_desktop_util::screenProxyLogicScreens();
    fmDebug() << "screen mode:" << mode << "screen count:" << screens.size();

    // 实际是单屏
    if ((DisplayMode::kShowonly == mode) || (DisplayMode::kDuplicate == mode) // 仅显示和复制
            || (screens.count() == 1)) {  // 单屏模式

        fmInfo() << "Building widgets for single screen mode, display mode:" << static_cast<int>(mode);

        ScreenPointer primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
        if (primary == nullptr) {
            fmCritical() << "get primary screen failed return";
            previewWidgets.clear();
            return;
        }

        fmDebug() << "Primary screen found:" << primary->name() << "geometry:" << primary->geometry();

        PreviewWidgetPtr wid = previewWidgets.value(primary->name());
        previewWidgets.clear();
        if (!wid.isNull()) {
            if (wid->geometry() != primary->geometry()) {
                fmDebug() << "Updating existing widget geometry from" << wid->geometry() << "to" << primary->geometry();
                wid->setGeometry(primary->geometry());
            }
        } else {
            wid = createWidget(primary);
        }

        previewWidgets.insert(primary->name(), wid);
    } else { //多屏
        for (auto screenName : previewWidgets.keys()) {
            // 删除实际不存在的数据
            if (!ddplugin_desktop_util::screenProxyScreen(screenName)) {
                fmDebug() << "screen:" << screenName << "  invalid, delete it.";
                previewWidgets.remove(screenName);
            }
        }

        for (ScreenPointer s : screens) {
            PreviewWidgetPtr wid = previewWidgets.value(s->name());
            if (!wid.isNull()) {
                if (wid->geometry() != s->geometry())
                    wid->setGeometry(s->geometry());
            } else {
                // 添加缺少的数据
                fmDebug() << "screen:" << s->name() << " added, create it.";
                wid = createWidget(s);
                previewWidgets.insert(s->name(), wid);
            }
        }
    }
}

void WallaperPreview::updateGeometry()
{
    for (ScreenPointer sp : ddplugin_desktop_util::screenProxyScreens()) {
        PreviewWidgetPtr wid = previewWidgets.value(sp->name());
        fmDebug() << "screen geometry change:" << sp.get() << wid.get();
        if (wid.get() != nullptr) {
            if (wid->geometry() == sp->geometry()) {
                fmDebug() << "background geometry is equal to screen geometry,and discard changes" << wid->geometry();
                continue;
            }

            fmDebug() << "Updating geometry for screen:" << sp->name()
                      << "from" << wid->geometry() << "to" << sp->geometry();
            wid->setGeometry(sp->geometry());
            // 大小变化后，直接更新背景显示
            wid->updateDisplay();
        }
    }
}

PreviewWidgetPtr WallaperPreview::createWidget(ScreenPointer sc)
{
    PreviewWidgetPtr wid(new BackgroundPreview(sc->name()));
    wid->setProperty("isPreview", true);
    wid->setProperty("myScreen", sc->name()); // assert screen->name is unique
    wid->setGeometry(sc->geometry());
    fmDebug() << "screen name" << sc->name() << "geometry" << sc->geometry();

    ddplugin_desktop_util::setPrviewWindow(wid.get());
    return wid;
}

QString WallaperPreview::getBackground(const QString &screen)
{
    QString ret;
    if (screen.isEmpty()) {
        fmWarning() << "Cannot get background: empty screen name provided";
        return ret;
    }

    fmDebug() << "Getting background for screen:" << screen;

    int retry = 5;
    static const int timeOut = 200;
    int oldTimeOut = inter->timeout();
    inter->setTimeout(timeOut);

    while (retry--) {
        fmDebug() << "Get background by GetCurrentWorkspaceBackgroundForMonitor and sc:" << screen;
        QDBusPendingReply<QString> reply = inter->GetCurrentWorkspaceBackgroundForMonitor(screen);
        reply.waitForFinished();

        if (reply.error().type() != QDBusError::NoError) {
            fmWarning() << "Get background failed by DBus and times:" << (5-retry)
                       << reply.error().type() << reply.error().name() << reply.error().message();
        } else {
            ret = reply.argumentAt<0>();
            fmDebug() << "Get background path succeed:" << ret << "screen" << screen << "   times:" << (5 - retry);
            break;
        }
    }
    inter->setTimeout(oldTimeOut);

    if (ret.isEmpty() || !QFile::exists(QUrl(ret).toLocalFile()))
        fmCritical() << "get background fail path :" << ret << "screen" << screen;
    else
        fmDebug() << "getBackground path :" << ret << "screen" << screen;
    return ret;
}

