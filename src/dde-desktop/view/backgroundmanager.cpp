/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "backgroundmanager.h"
#include "screen/screenhelper.h"
#include "util/util.h"

#include <qpa/qplatformwindow.h>
#include <QImageReader>

BackgroundManager::BackgroundManager(bool preview, QObject *parent)
    : QObject(parent)
    , windowManagerHelper(DWindowManagerHelper::instance())
    , m_preview(preview)
{
    init();
    QDBusConnection::sessionBus().connect("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",  "NameOwnerChanged", this, SLOT(onWmDbusStarted(QString, QString, QString)));
}

BackgroundManager::~BackgroundManager()
{
    if (gsettings) {
        gsettings->deleteLater();
        gsettings = nullptr;
    }

    if (wmInter) {
        wmInter->deleteLater();
        wmInter = nullptr;
    }

    windowManagerHelper = nullptr;

    m_backgroundMap.clear();
    QDBusConnection::sessionBus().disconnect("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",  "NameOwnerChanged", this, SLOT(onWmDbusStarted(QString, QString, QString)));
}

void BackgroundManager::onRestBackgroundManager()
{
    //判断是否需要绘制背景，如果是窗口绘制背景则桌面isEnable返回false，即桌面不绘制背景
    if (m_preview || isEnabled()) {
        if (wmInter) {
            return;
        }

        wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this);
        gsettings = new DGioSettings("com.deepin.dde.appearance", "", this);

        if (!m_preview) {
            connect(wmInter, &WMInter::WorkspaceSwitched, this, [this] (int, int to) {
                currentWorkspaceIndex = to;
                pullImageSettings();
                onResetBackgroundImage();
            });

            connect(gsettings, &DGioSettings::valueChanged, this, [this] (const QString & key, const QVariant & value) {
                Q_UNUSED(value);
                if (key == "background-uris") {
                    pullImageSettings();
                    onResetBackgroundImage();
                }
            });
        }

        //屏幕改变
        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenChanged,
                this, &BackgroundManager::onBackgroundBuild);
        disconnect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenChanged,
                   this, &BackgroundManager::onSkipBackgroundBuild);

        //显示模式改变
        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigDisplayModeChanged,
                this, &BackgroundManager::onBackgroundBuild);
        disconnect(ScreenHelper::screenManager(), &AbstractScreenManager::sigDisplayModeChanged,
                   this, &BackgroundManager::onSkipBackgroundBuild);

        //屏幕大小改变
        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenGeometryChanged,
                this, &BackgroundManager::onScreenGeometryChanged);

        //可用区改变,背景不用处理可用区改变
//        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenAvailableGeometryChanged,
//                this, &BackgroundManager::onScreenGeometryChanged);

        //创建背景
        pullImageSettings();
        onBackgroundBuild();
    } else {

        // 清理数据
        if (gsettings) {
            gsettings->deleteLater();
            gsettings = nullptr;
        }

        if (wmInter) {
            wmInter->deleteLater();
            wmInter = nullptr;
        }

        currentWorkspaceIndex = 0;

        //屏幕改变
        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenChanged,
                this, &BackgroundManager::onSkipBackgroundBuild);
        disconnect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenChanged,
                   this, &BackgroundManager::onBackgroundBuild);

        //显示模式改变
        connect(ScreenHelper::screenManager(), &AbstractScreenManager::sigDisplayModeChanged,
                this, &BackgroundManager::onSkipBackgroundBuild);
        disconnect(ScreenHelper::screenManager(), &AbstractScreenManager::sigDisplayModeChanged,
                   this, &BackgroundManager::onBackgroundBuild);

        //没有背景所以不用关心
        disconnect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenGeometryChanged,
                   this, &BackgroundManager::onScreenGeometryChanged);
        disconnect(ScreenHelper::screenManager(), &AbstractScreenManager::sigScreenAvailableGeometryChanged,
                   this, &BackgroundManager::onScreenGeometryChanged);

        //销毁窗口
        m_backgroundMap.clear();

        //不创建背景
        onSkipBackgroundBuild();
    }
}

void BackgroundManager::onScreenGeometryChanged()
{
    bool changed = false;
    for (ScreenPointer sp : m_backgroundMap.keys()) {
        BackgroundWidgetPointer bw = m_backgroundMap.value(sp);
        qDebug() << "screen geometry change:" << sp.get() << bw.get();
        if (bw.get() != nullptr) {
            //bw->windowHandle()->handle()->setGeometry(sp->handleGeometry()); //不能设置，设置了widget的geometry会被乱改
            //fix bug32166 bug32205
            if (bw->geometry() == sp->geometry()) {
                qDebug() << "background geometry is equal to screen geometry,and discard changes" << bw->geometry();
                continue;
            }
            qInfo() << "background geometry change from" << bw->geometry() << "to" << sp->geometry()
                    << "screen name" << sp->name();
            bw->setGeometry(sp->geometry());
            changed = true;
        }
    }

    //背景处理
    if (changed)
        onResetBackgroundImage();
}

void BackgroundManager::init()
{
    if (!m_preview) {
        connect(windowManagerHelper, &DWindowManagerHelper::windowManagerChanged,
                this, &BackgroundManager::onRestBackgroundManager);
        connect(windowManagerHelper, &DWindowManagerHelper::hasCompositeChanged,
                this, &BackgroundManager::onRestBackgroundManager);
    }

    onRestBackgroundManager();
}

void BackgroundManager::pullImageSettings()
{
    m_backgroundImagePath.clear();
    for (ScreenPointer sc : ScreenMrg->logicScreens()) {
        QString path = getBackgroundFromWm(sc->name());
        m_backgroundImagePath.insert(sc->name(), path);
    }
}

QString BackgroundManager::getBackgroundFromWm(const QString &screen)
{
    QString ret;
    if (!screen.isEmpty() && wmInter) {

        // 1.从窗管获取壁纸
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

        if (ret.isEmpty() || !QFile::exists(QUrl(ret).toLocalFile())) {

            // 2.从配置文件解析壁纸
            ret = getBackgroundFromWmConfig(screen);
            if (ret.isEmpty() || !QFile::exists(QUrl(ret).toLocalFile())) {

                // 3.使用默认壁纸
                ret = getDefaultBackground();
                qCritical() << "get background fail path :" << ret << "screen" << screen
                            << "use default:" << ret;
            } else {
                qCritical() << "get background fail path :" << ret << "screen" << screen
                            << "use wm config file:" << ret;
            }
        } else {
            qInfo() << "getBackgroundFromWm GetCurrentWorkspaceBackgroundForMonitor path :" << ret << "screen" << screen;
        }
    } else {
        qInfo() << "Get background path terminated screen:" << screen << wmInter;
    }

    return ret;
}

QString BackgroundManager::getBackgroundFromWmConfig(const QString &screen)
{
    QString imagePath;

    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QFile wmFile(homePath + "/.config/deepinwmrc");
    if (wmFile.open(QIODevice::ReadOnly | QIODevice::Text)) {

        // 根据工作区和屏幕名称查找对应的壁纸
        while (!wmFile.atEnd()) {
            QString line = wmFile.readLine();
            int index = line.indexOf("@");
            int indexEQ = line.indexOf("=");
            if (index <= 0 || indexEQ <= index+1) {
                continue;
            }

            int workspaceIndex = line.left(index).toInt();
            QString screenName = line.mid(index+1, indexEQ-index-1);
            if (workspaceIndex != currentWorkspaceIndex || screenName != screen) {
                continue;
            }

            imagePath = line.mid(indexEQ+1).trimmed();
            break;
        }

        wmFile.close();
    }

    return imagePath;
}

QString BackgroundManager::getDefaultBackground() const
{
    //获取默认壁纸
    QString defaultPath;
    if (gsettings) {
        for (const QString &path : gsettings->value("background-uris").toStringList()){
            if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
                continue;
            }
            else {
                defaultPath = path;
                qInfo() << "default background path:" << path;
                break;
            }
        }
    }
    // 设置默认壁纸
    if (defaultPath.isEmpty()) {
        defaultPath = QString("file:///usr/share/backgrounds/default_background.jpg");
    }
    return defaultPath;
}

BackgroundWidgetPointer BackgroundManager::createBackgroundWidget(ScreenPointer screen)
{
    BackgroundWidgetPointer bwp(new BackgroundWidget);
    bwp->setAccessableInfo(screen->name());
    bwp->setProperty("isPreview", m_preview);
    bwp->setProperty("myScreen", screen->name()); // assert screen->name is unique
    //bwp->createWinId();   //不创建，4k下有bug
    //bwp->windowHandle()->handle()->setGeometry(screen->handleGeometry()); //不能设置，设置了widget的geometry会被乱改//分辨率原始大小
    bwp->setGeometry(screen->geometry()); //经过缩放的区域
    qDebug() << "screen name" << screen->name() << "geometry" << screen->geometry() << bwp.get();

    if (m_preview) {
        DesktopUtil::set_prview_window(bwp.data());
    } else {
        DesktopUtil::set_desktop_window(bwp.data());
    }

    return bwp;
}

bool BackgroundManager::isEnabled() const
{
    // 只支持kwin，或未开启混成的桌面环境
//    return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
    return m_backgroundEnable;
}

void BackgroundManager::setVisible(bool visible)
{
    m_visible = visible;
    for (BackgroundWidgetPointer w : m_backgroundMap.values()) {
        w->setVisible(visible);
    }
}

bool BackgroundManager::isVisible() const
{
    return m_visible;
}

BackgroundWidgetPointer BackgroundManager::backgroundWidget(ScreenPointer sp) const
{
    return m_backgroundMap.value(sp);
}

void BackgroundManager::setBackgroundImage(const QString &screen, const QString &path)
{
    if (screen.isEmpty() || path.isEmpty())
        return;

    m_backgroundImagePath[screen] = path;
    onResetBackgroundImage();
}


void BackgroundManager::onBackgroundBuild()
{
    //屏幕模式判断
    AbstractScreenManager::DisplayMode mode = ScreenMrg->lastChangedMode();
    qInfo() << "screen mode" << mode << "screen count" << ScreenMrg->screens().size();

    //实际是单屏
    if ((AbstractScreenManager::Showonly == mode) || (AbstractScreenManager::Duplicate == mode) //仅显示和复制
            || (ScreenMrg->screens().count() == 1)) {  //单屏模式

        ScreenPointer primary = ScreenMrg->primaryScreen();
        if (primary == nullptr) {
            qCritical() << "get primary screen failed return";
            //清空并通知view重建
            m_backgroundMap.clear();
            emit sigBackgroundBuilded(mode);
            return;
        }

        BackgroundWidgetPointer bwp = m_backgroundMap.value(primary);
        m_backgroundMap.clear();
        if (!bwp.isNull()) {
            if (bwp->geometry() != primary->geometry())
                bwp->setGeometry(primary->geometry());
        } else {
            bwp = createBackgroundWidget(primary);
        }

        m_backgroundMap.insert(primary, bwp);

        //设置壁纸
        onResetBackgroundImage();

        if (m_visible)
            bwp->show();
        else
            qWarning() << "Disable show the background widget, of screen:" << primary->name() << primary->geometry();
    } else { //多屏
        auto screes = ScreenMrg->logicScreens();
        for (auto sp : m_backgroundMap.keys()) {
            if (!screes.contains(sp)) {
                auto rmd = m_backgroundMap.take(sp);
                qInfo() << "screen:" << rmd->property("myScreen") << "is invalid, delete it.";
            }
        }
        for (ScreenPointer sc : screes) {
            BackgroundWidgetPointer bwp = m_backgroundMap.value(sc);
            if (!bwp.isNull()) {
                if (bwp->geometry() != sc->geometry())
                    bwp->setGeometry(sc->geometry());
            } else {
                qInfo() << "screen:" << sc->name() << "  added, create it.";
                bwp = createBackgroundWidget(sc);
                m_backgroundMap.insert(sc, bwp);
            }

            if (m_visible)
                bwp->show();
            else
                qWarning() << "Disable show the background widget, of screen:" << sc->name() << sc->geometry();
        }

        onResetBackgroundImage();
    }

    //通知view重建
    emit sigBackgroundBuilded(mode);
}

void BackgroundManager::onSkipBackgroundBuild()
{
    //通知view重建
    emit sigBackgroundBuilded(ScreenMrg->lastChangedMode());
}

void BackgroundManager::onResetBackgroundImage()
{
    auto getPix = [](const QString & path, const QPixmap & defalutPixmap)->QPixmap{
        if (path.isEmpty())
            return defalutPixmap;

        QString currentWallpaper = path.startsWith("file:") ? QUrl(path).toLocalFile() : path;
        QPixmap backgroundPixmap(currentWallpaper);
        // fix whiteboard shows when a jpeg file with filename xxx.png
        // content formart not epual to extension
        if (backgroundPixmap.isNull()) {
            QImageReader reader(currentWallpaper);
            reader.setDecideFormatFromContent(true);
            backgroundPixmap = QPixmap::fromImage(reader.read());
        }
        return backgroundPixmap.isNull() ? defalutPixmap : backgroundPixmap;
    };

    QPixmap defaultImage;

    QMap<QString, QString> recorder; //记录有效的壁纸
    for (ScreenPointer sp : m_backgroundMap.keys()) {
        QString userPath;
        if (!m_backgroundImagePath.contains(sp->name())) {
            userPath = getBackgroundFromWm(sp->name());
        } else {
            userPath = m_backgroundImagePath.value(sp->name());
        }

        if (!userPath.isEmpty())
            recorder.insert(sp->name(), userPath);

        QPixmap backgroundPixmap = getPix(userPath, defaultImage);
        if (backgroundPixmap.isNull()) {
            qCritical() << "screen " << sp->name() << "backfround path" << userPath
                        << "can not read!";
            continue;
        }

        BackgroundWidgetPointer bw = m_backgroundMap.value(sp);
        QSize trueSize = sp->handleGeometry().size(); //使用屏幕缩放前的分辨率
        auto pix = backgroundPixmap.scaled(trueSize,
                                           Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation);

        if (pix.width() > trueSize.width() || pix.height() > trueSize.height()) {
            pix = pix.copy(QRect(static_cast<int>((pix.width() - trueSize.width()) / 2.0),
                                 static_cast<int>((pix.height() - trueSize.height()) / 2.0),
                                 trueSize.width(),
                                 trueSize.height()));
        }

        qDebug() << sp->name() << "background path" << userPath << "truesize" << trueSize << "devicePixelRatio"
                 << bw->devicePixelRatioF() << pix << "widget" << bw.get();
        pix.setDevicePixelRatio(bw->devicePixelRatioF());
        bw->setPixmap(pix);
    }

    //更新壁纸
    m_backgroundImagePath = recorder;
}

void BackgroundManager::onWmDbusStarted(QString name, QString oldOwner, QString newOwner)
{
    Q_UNUSED(oldOwner)
    Q_UNUSED(newOwner)
    //窗管服务注销也会进入该函数，因此需要判断服务是否已注册
    if (name == QString("com.deepin.wm") && QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.wm")) {
        qInfo() << "dbus server com.deepin.wm started.";
        pullImageSettings();
        onResetBackgroundImage();

        QDBusConnection::sessionBus().disconnect("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",  "NameOwnerChanged", this, SLOT(onWmDbusStarted(QString, QString, QString)));
    }
}
