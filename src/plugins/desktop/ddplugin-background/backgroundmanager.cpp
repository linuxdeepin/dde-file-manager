/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include <QGSettings>

#include <QImageReader>

DFMBASE_USE_NAMESPACE
DGUI_USE_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE
DSB_D_USE_NAMESPACE

static QString getScreenName(QWidget *win)
{
    return win->property(kPropScreenName).toString();
}

static QMap<QString, QWidget *> rootMap(FrameService *srv)
{
    QList<QWidget *> root = srv->rootWindows();
    QMap<QString, QWidget *> ret;
    for (QWidget *win : root) {
        QString name = getScreenName(win);
        if (name.isEmpty())
            continue;
        ret.insert(name, win);
    }

    return ret;
}

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
    : QObject(parent)
    , d(new BackgroundManagerPrivate(this))
{
    QDBusConnection::sessionBus().connect("org.freedesktop.DBus"
                                          , "/org/freedesktop/DBus"
                                          , "org.freedesktop.DBus"
                                          ,  "NameOwnerChanged"
                                          , this
                                          , SLOT(onWmDbusStarted(QString, QString, QString)));
}

BackgroundManager::~BackgroundManager()
{
}

void BackgroundManager::init()
{
    connect(d->windowManagerHelper, &DWindowManagerHelper::windowManagerChanged,
            this, &BackgroundManager::onRestBackgroundManager);
    connect(d->windowManagerHelper, &DWindowManagerHelper::hasCompositeChanged,
            this, &BackgroundManager::onRestBackgroundManager);

    onRestBackgroundManager();

    auto &ctx = dpfInstance.serviceContext();
    d->frameService = ctx.service<FrameService>(FrameService::name());
    if (!d->frameService) {
        qWarning() << "get frame service failed.";
        return;
    }

    connect(d->frameService, &FrameService::windowBuilded, this, &BackgroundManager::onBackgroundBuild, Qt::DirectConnection);
    connect(d->frameService, &FrameService::windowAboutToBeBuilded, this, &BackgroundManager::onDetachWindows, Qt::DirectConnection);
    connect(d->frameService, &FrameService::geometryChanged, this, &BackgroundManager::onGeometryChanged, Qt::DirectConnection);
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

void BackgroundManager::setBackgroundPath(const QString &screen, const QString &path)
{
    if (screen.isEmpty() || path.isEmpty())
        return;

    d->backgroundPaths.insert(screen, path);
    resetBackgroundImage();
}

void BackgroundManager::onBackgroundBuild()
{
    if (!d->frameService) {
        qWarning() << "no screen service,give up build background.";
        return;
    }

    QList<QWidget *> root = d->frameService->rootWindows();
    if (root.size() == 1) {
        QWidget *primary = root.first();
        if (primary == nullptr) {
            // get screen failed,clear all widget
            d->backgroundWidgets.clear();
            qCritical() << "get primary screen failed return.";
            return;
        }

        const QString screeName = getScreenName(primary);
        if (screeName.isEmpty()) {
            qWarning() << "can not get screen name from root window";
            return;
        }

        BackgroundWidgetPointer bwp = d->backgroundWidgets.value(screeName);
        d->backgroundWidgets.clear();
        if (!bwp.isNull()) {
            QRect geometry = d->relativeGeometry(primary->geometry()); // scaled area
            if (bwp->geometry() != geometry)
                bwp->setGeometry(geometry);
            bwp->setParent(primary);
            bwp->lower();
        } else {
            bwp = createBackgroundWidget(primary);
        }

        d->backgroundWidgets.insert(screeName, bwp);
        bwp->show();
    } else {
        // check whether to add
        for (QWidget *win : root) {

            const QString screenName = getScreenName(win);
            if (screenName.isEmpty()) {
                qWarning() << "can not get screen name from root window";
                continue;
            }

            BackgroundWidgetPointer bwp = d->backgroundWidgets.value(screenName);
            if (!bwp.isNull()) {
                // update widget
                QRect geometry = d->relativeGeometry(win->geometry()); // scaled area
                if (bwp->geometry() != geometry)
                    bwp->setGeometry(geometry);
                bwp->setParent(win);
                bwp->lower();
            } else {
                // add new widget
                qInfo() << "screen:" << screenName << "  added, create it.";
                bwp = createBackgroundWidget(win);
                d->backgroundWidgets.insert(screenName, bwp);
            }
            bwp->show();
        }

        // clean up invalid widget
        {
            auto winMap = rootMap(d->frameService);
            for (const QString &sp : d->backgroundWidgets.keys()) {
                if (!winMap.contains(sp)) {
                    d->backgroundWidgets.take(sp);
                    qInfo() << "remove screen:" << sp;
                }
            }
        }
    }

    resetBackgroundImage();
}

void BackgroundManager::onDetachWindows()
{
    for (const BackgroundWidgetPointer &bwp : d->backgroundWidgets.values())
        bwp->setParent(nullptr);
}

void BackgroundManager::onRestBackgroundManager()
{
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

    if (d->frameService)
        onBackgroundBuild();
}

void BackgroundManager::onGeometryChanged()
{
    if (!d->frameService) {
        qWarning() << "no screen service,give up build background.";
        return;
    }

    auto winMap = rootMap(d->frameService);
    for (auto itor = d->backgroundWidgets.begin(); itor != d->backgroundWidgets.end(); ++itor) {
        BackgroundWidgetPointer bw = itor.value();
        auto *win = winMap.value(itor.key());
        if (win == nullptr) {
            qCritical() << "can not get root " << itor.key() << getScreenName(win);
            continue;
        }

        if (bw.get() != nullptr) {
            QRect geometry = d->relativeGeometry(win->geometry()); // scaled area
            if (bw->geometry() == geometry) {
                qDebug() << "background geometry is equal to root widget geometry,and discard changes" << bw->geometry()
                         << win->geometry() << win->property(kPropScreenName).toString()
                         << win->property(kPropScreenGeometry).toRect() << win->property(kPropScreenHandleGeometry).toRect()
                         << win->property(kPropScreenAvailableGeometry);
                continue;
            }
            qInfo() << "background geometry change from" << bw->geometry() << "to" << geometry
                    << "screen name" << getScreenName(win) << "screen geometry" << win->geometry();
            bw->setGeometry(geometry);

            // a size change,just redraw
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
    if (QStringLiteral("backgroundUris") == key) {
        updateBackgroundPaths();
        resetBackgroundImage();
    }
}

void BackgroundManager::updateBackgroundPaths()
{
    if (!d->frameService)
        return;

    d->backgroundPaths.clear();

    if (!d->isEnableBackground())
        return;

    auto winMap = rootMap(d->frameService);
    for (auto itor = d->backgroundWidgets.begin(); itor != d->backgroundWidgets.end(); ++itor) {
        auto *win = winMap.value(itor.key());
        if (win == nullptr) {
            qCritical() << "can not get root " << itor.key() << getScreenName(win);
            continue;
        }

        QString screenName = getScreenName(win);
        QString path = getBackground(screenName);
        d->backgroundPaths.insert(screenName, path);
    }
}

/*!
 * \brief BackgroundManager::resetBackgroundImage
 */
void BackgroundManager::resetBackgroundImage()
{
    if (d->isEnableBackground()) {
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

BackgroundWidgetPointer BackgroundManager::createBackgroundWidget(QWidget *root)
{
    const QString screenName = getScreenName(root);
    BackgroundWidgetPointer bwp(new BackgroundDefault(screenName, root));
    bwp->setParent(root);
    bwp->setProperty(kPropScreenName, getScreenName(root));
    bwp->setProperty(kPropWidgetName, "background");
    bwp->setProperty(kPropWidgetLevel, 5.0);
    bwp->lower();

    QRect geometry = d->relativeGeometry(root->geometry()); // scaled area
    bwp->setGeometry(geometry);
    qDebug() << "screen name" << screenName << "geometry" << geometry << root->geometry() << bwp.get();

    return bwp;
}

