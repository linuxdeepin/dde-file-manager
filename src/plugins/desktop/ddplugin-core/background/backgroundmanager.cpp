/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "backgroundmanager_p.h"
#include "backgrounddefault.h"
#include "backgroundservice.h"
#include "utils/desktoputils.h"
#include "screenservice.h"

#include <QGSettings>

#include <QImageReader>

DFMBASE_USE_NAMESPACE
DGUI_USE_NAMESPACE

DSB_D_BEGIN_NAMESPACE

BackgroundManagerPrivate::BackgroundManagerPrivate(BackgroundManager *qq)
    : QObject (qq)
    , q(qq)
    , windowManagerHelper(DWindowManagerHelper::instance())
{

}

BackgroundManagerPrivate::~BackgroundManagerPrivate()
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

    backgroundWidgets.clear();
    backgroundPaths.clear();
}

QString BackgroundManagerPrivate::getBackgroundFormWm(const QString &screen)
{
    QString path;

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
            path = reply.argumentAt<0>();
            qInfo() << "Get background path succeed:" << path << "screen" << screen << "   times:" << (5 - retry);
            break;
        }
    }

    wmInter->setTimeout(oldTimeOut);

    return  path;
}

QString BackgroundManagerPrivate::getBackgroundFormConfig(const QString &screen)
{
    QString path;

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

            path = line.mid(indexEQ+1).trimmed();
            break;
        }

        wmFile.close();
    }

    return path;
}

QString BackgroundManagerPrivate::getDefaultBackground() const
{
    QString defaultPath;
    if (gsettings) {
        for (const QString &path : gsettings->get("background-uris").toStringList()) {
            if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
                continue;
            } else {
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

bool BackgroundManagerPrivate::isEnableBackground()
{
    return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
}

BackgroundManager::BackgroundManager(QObject *parent)
    : d(new BackgroundManagerPrivate(this))
{
    init();

    QDBusConnection::sessionBus().connect("org.freedesktop.DBus"
                                          , "/org/freedesktop/DBus"
                                          , "org.freedesktop.DBus"
                                          ,  "NameOwnerChanged"
                                          , this
                                          , SLOT(onWmDbusStarted(QString, QString, QString)));
}

BackgroundManager::~BackgroundManager()
{
    QDBusConnection::sessionBus().disconnect("org.freedesktop.DBus"
                                             , "/org/freedesktop/DBus"
                                             , "org.freedesktop.DBus"
                                             ,  "NameOwnerChanged"
                                             , this,
                                             SLOT(onWmDbusStarted(QString, QString, QString)));
}

void BackgroundManager::init()
{
    connect(d->windowManagerHelper, &DWindowManagerHelper::windowManagerChanged,
            this, &BackgroundManager::onRestBackgroundManager);
    connect(d->windowManagerHelper, &DWindowManagerHelper::hasCompositeChanged,
            this, &BackgroundManager::onRestBackgroundManager);

    onRestBackgroundManager();
}

QMap<QString, BackgroundWidgetPointer> BackgroundManager::allBackgroundWidgets()
{
    return d->backgroundWidgets;
}

BackgroundWidgetPointer BackgroundManager::backgroundWidget(const QString &screen)
{
    return d->backgroundWidgets.value(screen);
}

QMap<QString, QString> BackgroundManager::allBackgroundPath()
{
    return d->backgroundPaths;
}

QString BackgroundManager::backgroundPath(const QString &screen)
{
    return d->backgroundPaths.value(screen);
}

void BackgroundManager::onBackgroundBuild()
{
    if (!d->screenService) {
        qWarning() << "no screen service,give up build background.";
        return;
    }

    DisplayMode mode = d->screenService->lastChangedMode();
    qInfo() << "screen mode:" << mode << "screen count:" << d->screenService->screens().size();

    // 实际是单屏
    if ((DisplayMode::Showonly == mode) || (DisplayMode::Duplicate == mode) // 仅显示和复制
            || (d->screenService->screens().count() == 1)) {  // 单屏模式

        ScreenPointer primary = d->screenService->primaryScreen();
        if (primary == nullptr) {
            qCritical() << "get primary screen failed return";
            //清空并通知view重建
            d->backgroundWidgets.clear();
            emit sigBackgroundBuilded(mode);
            return;
        }

        BackgroundWidgetPointer bwp = d->backgroundWidgets.value(primary->name());
        d->backgroundWidgets.clear();
        if (!bwp.isNull()) {
            if (bwp->geometry() != primary->geometry())
                bwp->setGeometry(primary->geometry());
        } else {
            bwp = createBackgroundWidget(primary);
        }

        d->backgroundWidgets.insert(primary->name(), bwp);

        // 设置壁纸
        resetBackgroundImage();

        bwp->show();

    } else { //多屏
        auto screes = d->screenService->logicScreens();

        for (auto screenName : d->backgroundWidgets.keys()) {

            // 删除实际不存在的数据
            if (!d->screenService->screen(screenName)) {
                qInfo() << "screen:" << screenName << "  invalid, delete it.";
                d->backgroundWidgets.remove(screenName);
            }
        }

        for (ScreenPointer s : screes) {
            BackgroundWidgetPointer bwp = d->backgroundWidgets.value(s->name());
            if (!bwp.isNull()) {
                if (bwp->geometry() != s->geometry())
                    bwp->setGeometry(s->geometry());
            } else {
                // 添加缺少的数据
                qInfo() << "screen:" << s->name() << "  added, create it.";
                bwp = createBackgroundWidget(s);
                d->backgroundWidgets.insert(s->name(), bwp);
            }

            bwp->show();
        }

        resetBackgroundImage();
    }

    emit sigBackgroundBuilded(mode);
}

void BackgroundManager::onRestBackgroundManager()
{
    auto &ctx = dpfInstance.serviceContext();
    d->screenService = ctx.service<ScreenService>(ScreenService::name());

    Q_ASSERT_X(d->screenService, "Error", "get screen service failed.");

    connect(d->screenService, &ScreenService::destroyed, this, [this](){
        d->screenService = nullptr;
        qWarning() << "screen service destroyed.";
    }, Qt::UniqueConnection);

    // 屏幕改变
    connect(d->screenService, &ScreenService::screenChanged, this, &BackgroundManager::onBackgroundBuild, Qt::UniqueConnection);

    // 显示模式改变
    connect(d->screenService, &ScreenService::displayModeChanged, this, &BackgroundManager::onBackgroundBuild, Qt::UniqueConnection);

    // 屏幕大小改变
    connect(d->screenService, &ScreenService::screenGeometryChanged, this, &BackgroundManager::onScreenGeometryChanged, Qt::UniqueConnection);

    if (d->isEnableBackground()) {
        // 绘制背景图片

        if (!d->wmInter) {
            d->wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm", QDBusConnection::sessionBus(), this);
            connect(d->wmInter, &WMInter::WorkspaceSwitched, this, &BackgroundManager::onWorkspaceSwitched);
        }

        if (!d->gsettings) {
            d->gsettings = new QGSettings("com.deepin.dde.appearance", "", this);
            connect(d->gsettings, &QGSettings::changed, this, &BackgroundManager::onAppearanceCalueChanged);
        }

        updateBackgroundPaths();

    }

    onBackgroundBuild();
}

void BackgroundManager::onScreenGeometryChanged()
{
    if (!d->screenService) {
        qWarning() << "no screen service,give up build background.";
        return;
    }
    for (ScreenPointer sp : d->screenService->screens()) {
        BackgroundWidgetPointer bw = d->backgroundWidgets.value(sp->name());
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

            // 大小变化后，直接更新背景显示
            bw->updateDisplay();
        }
    }
}

void BackgroundManager::onWmDbusStarted(QString name, QString oldOwner, QString newOwner)
{
    Q_UNUSED(oldOwner)
    Q_UNUSED(newOwner)
    // 窗管服务注销也会进入该函数，因此需要判断服务是否已注册
    if (name == QString("com.deepin.wm") && QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.wm")) {
        qInfo() << "dbus server com.deepin.wm started.";
        updateBackgroundPaths();
        resetBackgroundImage();
        QDBusConnection::sessionBus().disconnect("org.freedesktop.DBus"
                                                 , "/org/freedesktop/DBus"
                                                 , "org.freedesktop.DBus"
                                                 ,  "NameOwnerChanged"
                                                 , this
                                                 , SLOT(onWmDbusStarted(QString, QString, QString)));
    }
}

void BackgroundManager::onWorkspaceSwitched(int from, int to)
{
    Q_UNUSED(from);

    d->currentWorkspaceIndex = to;
    updateBackgroundPaths();
    resetBackgroundImage();
}

void BackgroundManager::onAppearanceCalueChanged(const QString &key)
{
    if (QStringLiteral("background-uris") == key) {
        updateBackgroundPaths();
        resetBackgroundImage();
    }
}

void BackgroundManager::updateBackgroundPaths()
{
    d->backgroundPaths.clear();

    if (!d->isEnableBackground())
        return;

    for (ScreenPointer sc : d->screenService->logicScreens()) {
        QString path = getBackground(sc->name());
        d->backgroundPaths.insert(sc->name(), path);
    }
}

/*!
 * \brief BackgroundManager::resetBackgroundImage
 */
void BackgroundManager::resetBackgroundImage()
{
    if (d->isEnableBackground()) {

        QPixmap defaultImage;

        QMap<QString, QString> recorder; // 记录有效的壁纸
        for (auto screenName : d->backgroundWidgets.keys()) {
            QString userPath;
            if (!d->backgroundPaths.contains(screenName)) {
                userPath = getBackground(screenName);
            } else {
                userPath = d->backgroundPaths.value(screenName);
            }

            if (userPath.isEmpty())
                continue;

            recorder.insert(screenName, userPath);

            BackgroundWidgetPointer bw = d->backgroundWidgets.value(screenName);

            // 设置原始图片文件，由显示的类根据显示规则进行绘制
            bw->setDisplay(userPath);
        }

        // 更新壁纸
        d->backgroundPaths = recorder;
    } else {
        for (auto w : d->backgroundWidgets.values()) {
            if (w->mode() != AbstractBackground::ModeCustom) {
                w->setMode(AbstractBackground::ModeCustom);
                w->update();
            }
        }
    }
}

QString BackgroundManager::getBackground(const QString &screen)
{
    QString path;
    if (!screen.isEmpty() && d->wmInter) {

        // 1.从窗管获取壁纸
        path = d->getBackgroundFormWm(screen);
        qInfo() << "getBackgroundFromWm GetCurrentWorkspaceBackgroundForMonitor path :" << path << "screen" << screen;

        if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
            // 2.从配置文件解析壁纸
            path = d->getBackgroundFormConfig(screen);
            qInfo() << "getBackgroundFormConfig path :" << path << "screen" << screen;

            if (path.isEmpty() || !QFile::exists(QUrl(path).toLocalFile())) {
                // 3.使用默认壁纸
                path = d->getDefaultBackground();
                qInfo() << "getDefaultBackground path :" << path << "screen" << screen;
            }
        }
    } else {
        qInfo() << "Get background path terminated screen:" << screen << d->wmInter;
    }

    return path;
}

BackgroundWidgetPointer BackgroundManager::createBackgroundWidget(ScreenPointer sp)
{
    BackgroundWidgetPointer bwp(new BackgroundDefault(sp->name()));

    bwp->setGeometry(sp->geometry()); // 经过缩放的区域
    qDebug() << "screen name" << sp->name() << "geometry" << sp->geometry() << bwp.get();

    setDesktopWindow(bwp.get());

    return bwp;
}

DSB_D_END_NAMESPACE
