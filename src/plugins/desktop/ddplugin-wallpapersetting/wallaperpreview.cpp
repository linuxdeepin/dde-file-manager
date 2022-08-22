/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "wallaperpreview.h"

#include "desktoputils/widgetutil.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

DFMBASE_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;

WallaperPreview::WallaperPreview(QObject *parent)
    : QObject(parent)
{
    qInfo() << "create com.deepin.wm";
    wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm",
                          QDBusConnection::sessionBus(), this);
    wmInter->setTimeout(1000);
    qInfo() << "create com.deepin.wm end";
}

WallaperPreview::~WallaperPreview()
{

}

void WallaperPreview::init()
{
    pullImageSettings();
    buildWidgets();
    updateWallpaper();
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
        QString path = getBackgroundFromWm(sc->name());
        wallpapers.insert(sc->name(), path);
    }
}

void WallaperPreview::updateWallpaper()
{
    QMap<QString, QString> recorder; // 记录有效的壁纸
    for (auto screenName : previewWidgets.keys()) {
        QString userPath;
        if (!wallpapers.contains(screenName)) {
            userPath = getBackgroundFromWm(screenName);
        } else {
            userPath = wallpapers.value(screenName);
        }

        if (userPath.isEmpty())
            continue;

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
    qInfo() << "screen mode:" << mode << "screen count:" << screens.size();

    // 实际是单屏
    if ((DisplayMode::kShowonly == mode) || (DisplayMode::kDuplicate == mode) // 仅显示和复制
            || (screens.count() == 1)) {  // 单屏模式

        ScreenPointer primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
        if (primary == nullptr) {
            qCritical() << "get primary screen failed return";
            previewWidgets.clear();
            return;
        }

        PreviewWidgetPtr wid = previewWidgets.value(primary->name());
        previewWidgets.clear();
        if (!wid.isNull()) {
            if (wid->geometry() != primary->geometry())
                wid->setGeometry(primary->geometry());
        } else {
            wid = createWidget(primary);
        }

        previewWidgets.insert(primary->name(), wid);
    } else { //多屏
        for (auto screenName : previewWidgets.keys()) {
            // 删除实际不存在的数据
            if (!ddplugin_desktop_util::screenProxyScreen(screenName)) {
                qInfo() << "screen:" << screenName << "  invalid, delete it.";
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
                qInfo() << "screen:" << s->name() << " added, create it.";
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
        qDebug() << "screen geometry change:" << sp.get() << wid.get();
        if (wid.get() != nullptr) {
            if (wid->geometry() == sp->geometry()) {
                qDebug() << "background geometry is equal to screen geometry,and discard changes" << wid->geometry();
                continue;
            }

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
    qDebug() << "screen name" << sc->name() << "geometry" << sc->geometry();

    ddplugin_desktop_util::setPrviewWindow(wid.get());
    return wid;
}

QString WallaperPreview::getBackgroundFromWm(const QString &screen)
{
    QString ret;
    if (screen.isEmpty())
        return ret;

    int retry = 5;
    static const int timeOut = 200;
    int oldTimeOut = wmInter->timeout();
    wmInter->setTimeout(timeOut);

    while (retry--) {
        qInfo() << "Get background by wm GetCurrentWorkspaceBackgroundForMonitor and sc:" << screen;
        QDBusPendingReply<QString> reply = wmInter->GetCurrentWorkspaceBackgroundForMonitor(screen);
        reply.waitForFinished();

        if (reply.error().type() != QDBusError::NoError) {
            qWarning() << "Get background failed by wmDBus and times:" << (5-retry)
                       << reply.error().type() << reply.error().name() << reply.error().message();
        } else {
            ret = reply.argumentAt<0>();
            qInfo() << "Get background path succeed:" << ret << "screen" << screen << "   times:" << (5 - retry);
            break;
        }
    }
    wmInter->setTimeout(oldTimeOut);

    if (ret.isEmpty() || !QFile::exists(QUrl(ret).toLocalFile()))
        qCritical() << "get background fail path :" << ret << "screen" << screen;
    else
        qInfo() << "getBackgroundFromWm path :" << ret << "screen" << screen;
    return ret;
}
