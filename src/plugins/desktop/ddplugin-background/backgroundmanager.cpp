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
#ifdef COMPILE_ON_V23
    #include "backgrounddde.h"
#else
    #include "backgroundwm.h"
#endif
#include "backgroundmanager.h"
#include "backgroundmanager_p.h"
#include "backgrounddefault.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include "dfm-base/dfm_desktop_defines.h"

#include <QImageReader>

DFMBASE_USE_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

inline QString getScreenName(QWidget *win)
{
    return win->property(DesktopFrameProperty::kPropScreenName).toString();
}

static QMap<QString, QWidget *> rootMap()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
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
    : QObject(qq)
    , q(qq)
{
}

BackgroundManagerPrivate::~BackgroundManagerPrivate()
{
    backgroundWidgets.clear();
    backgroundPaths.clear();
}

bool BackgroundManagerPrivate::isEnableBackground()
{
    //return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
    // fix bug152473
    return enableBackground;
}

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent)
    , d(new BackgroundManagerPrivate(this))
{
    d->service =
    #ifdef COMPILE_ON_V23
        new BackgroundDDE(this);
    #else
        new BackgroundWM(this);
    #endif
}

BackgroundManager::~BackgroundManager()
{
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &BackgroundManager::onDetachWindows);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &BackgroundManager::onBackgroundBuild);
    CanvasCoreUnsubscribe(signal_DesktopFrame_GeometryChanged, &BackgroundManager::onGeometryChanged);
}

void BackgroundManager::init()
{
    onRestBackgroundManager();

    CanvasCoreSubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &BackgroundManager::onDetachWindows);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowBuilded, &BackgroundManager::onBackgroundBuild);
    CanvasCoreSubscribe(signal_DesktopFrame_GeometryChanged, &BackgroundManager::onGeometryChanged);
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
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    if (root.size() == 1) {
        QWidget *primary = root.first();
        if (primary == nullptr) {
            // get screen failed,clear all widget
            d->backgroundWidgets.clear();
            qCritical() << "get primary screen failed return.";
            return;
        }

        const QString &screeName = getScreenName(primary);
        if (screeName.isEmpty()) {
            qWarning() << "can not get screen name from root window";
            return;
        }

        BackgroundWidgetPointer bwp = d->backgroundWidgets.value(screeName);
        d->backgroundWidgets.clear();
        if (!bwp.isNull()) {
            QRect geometry = d->relativeGeometry(primary->geometry());   // scaled area
            if (bwp->geometry() != geometry)
                bwp->setGeometry(geometry);
            bwp->setParent(primary);
        } else {
            bwp = createBackgroundWidget(primary);
        }

        d->backgroundWidgets.insert(screeName, bwp);
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
                QRect geometry = d->relativeGeometry(win->geometry());   // scaled area
                if (bwp->geometry() != geometry)
                    bwp->setGeometry(geometry);
                bwp->setParent(win);
            } else {
                // add new widget
                qInfo() << "screen:" << screenName << "  added, create it.";
                bwp = createBackgroundWidget(win);
                d->backgroundWidgets.insert(screenName, bwp);
            }
        }

        // clean up invalid widget
        {
            auto winMap = rootMap();
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
        connect(d->service, &BackgroundService::backgroundChanged, this, &BackgroundManager::onBackgroundChanged);
        updateBackgroundPaths();
    }
    onBackgroundBuild();
}

void BackgroundManager::onBackgroundChanged()
{
    updateBackgroundPaths();
    resetBackgroundImage();
}

void BackgroundManager::onGeometryChanged()
{
    auto winMap = rootMap();
    for (auto itor = d->backgroundWidgets.begin(); itor != d->backgroundWidgets.end(); ++itor) {
        BackgroundWidgetPointer bw = itor.value();
        auto *win = winMap.value(itor.key());
        if (win == nullptr) {
            qCritical() << "can not get root " << itor.key() << getScreenName(win);
            continue;
        }

        if (bw.get() != nullptr) {
            QRect geometry = d->relativeGeometry(win->geometry());   // scaled area
            if (bw->geometry() == geometry) {
                qDebug() << "background geometry is equal to root widget geometry,and discard changes" << bw->geometry()
                         << win->geometry() << win->property(DesktopFrameProperty::kPropScreenName).toString()
                         << win->property(DesktopFrameProperty::kPropScreenGeometry).toRect() << win->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect()
                         << win->property(DesktopFrameProperty::kPropScreenAvailableGeometry);
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

void BackgroundManager::updateBackgroundPaths()
{
    d->backgroundPaths.clear();

    if (!d->isEnableBackground())
        return;

    auto winMap = rootMap();
    for (auto itor = d->backgroundWidgets.begin(); itor != d->backgroundWidgets.end(); ++itor) {
        auto *win = winMap.value(itor.key());
        if (win == nullptr) {
            qCritical() << "can not get root " << itor.key() << getScreenName(win);
            continue;
        }

        QString screenName = getScreenName(win);
        QString path = d->service->background(screenName);
        d->backgroundPaths.insert(screenName, path);
    }
}

/*!
 * \brief BackgroundManager::resetBackgroundImage
 */
void BackgroundManager::resetBackgroundImage()
{
    if (d->isEnableBackground()) {
        QMap<QString, QString> recorder;   // 记录有效的壁纸
        for (auto screenName : d->backgroundWidgets.keys()) {
            QString userPath;
            if (!d->backgroundPaths.contains(screenName)) {
                userPath = d->service->background(screenName);
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

BackgroundWidgetPointer BackgroundManager::createBackgroundWidget(QWidget *root)
{
    const QString screenName = getScreenName(root);
    BackgroundWidgetPointer bwp(new BackgroundDefault(screenName, root));
    bwp->setParent(root);
    bwp->setProperty(DesktopFrameProperty::kPropScreenName, getScreenName(root));
    bwp->setProperty(DesktopFrameProperty::kPropWidgetName, "background");
    bwp->setProperty(DesktopFrameProperty::kPropWidgetLevel, 5.0);

    QRect geometry = d->relativeGeometry(root->geometry());   // scaled area
    bwp->setGeometry(geometry);
    qDebug() << "screen name" << screenName << "geometry" << geometry << root->geometry() << bwp.get();

    return bwp;
}
