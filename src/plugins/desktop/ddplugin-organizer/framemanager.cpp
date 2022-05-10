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
#include "framemanager.h"
#include "private/framemanager_p.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include "dfm-base/dfm_desktop_defines.h"

#include <QAbstractItemView>

DFMBASE_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

FrameManagerPrivate::FrameManagerPrivate(FrameManager *qq) : q(qq)
{

}

FrameManagerPrivate::~FrameManagerPrivate()
{
    delete organizer;
    // 退出自动整理
    // 刷新桌面
}

void FrameManagerPrivate::buildSurface()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    QMap<QString, QWidget *> rootMap;
    // create surface
    for (QWidget *win : root) {
        const QString screenName = win->property(DesktopFrameProperty::kPropScreenName).toString();
        if (screenName.isEmpty()) {
            qWarning() << "can not get screen name from root window";
            continue;
        }

        SurfacePointer surface = surfaceWidgets.value(screenName);
        if (surface.isNull()) {
            // add new widget
            qInfo() << "screen:" << screenName << "  added, create it.";
            surface = createSurface(win);
            surfaceWidgets.insert(screenName, surface);
        }

        layoutSurface(win, surface);
        rootMap.insert(screenName, win);
    }

    // clean up invalid widget
    {
        for (const QString &sp : surfaceWidgets.keys()) {
            if (!rootMap.contains(sp)) {
                surfaceWidgets.take(sp);
                qInfo() << "remove surface:" << sp;
            }
        }
    }
}

SurfacePointer FrameManagerPrivate::createSurface(QWidget *root)
{
    SurfacePointer surface = nullptr;
    if (Q_UNLIKELY(!root))
        return surface;

    surface.reset(new Surface());
    surface->setProperty(DesktopFrameProperty::kPropScreenName, root->property(DesktopFrameProperty::kPropScreenName).toString());
    surface->setProperty(DesktopFrameProperty::kPropWidgetName, "organizersurface");
    surface->setProperty(DesktopFrameProperty::kPropWidgetLevel, 11.0);
    return surface;
}

void FrameManagerPrivate::layoutSurface(QWidget *root, SurfacePointer surface, bool hidden)
{
    Q_ASSERT(surface);
    Q_ASSERT(root);

    auto view = dynamic_cast< QAbstractItemView *>(findView(root));
    // check hidden flags
    if (view && !hidden) {
        surface->setParent(view->viewport());
        surface->setGeometry(QRect(QPoint(0, 0), view->geometry().size()));
    } else {
        surface->setParent(root);
        surface->setGeometry(QRect(QPoint(0, 0), root->geometry().size()));
        surface->raise();
    }
}

QWidget *FrameManagerPrivate::findView(QWidget *root) const
{
    if (Q_UNLIKELY(!root))
        return nullptr;

    for (QObject *obj : root->children()) {
        if (QWidget *wid = dynamic_cast<QWidget *>(obj)) {
            QString type = wid->property(DesktopFrameProperty::kPropWidgetName).toString();
            if (type == "canvas") {
                return wid;
            }
        }
    }
    return nullptr;
}

FrameManager::FrameManager(QObject *parent)
    : QObject(parent)
    , d(new FrameManagerPrivate(this))
{

}

FrameManager::~FrameManager()
{
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &FrameManager::onDetachWindows);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &FrameManager::onBuild);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowShowed, &FrameManager::onWindowShowed);
    CanvasCoreUnsubscribe(signal_DesktopFrame_GeometryChanged, &FrameManager::onGeometryChanged);
    CanvasCoreUnsubscribe(signal_DesktopFrame_AvailableGeometryChanged, &FrameManager::onGeometryChanged);

    delete d;
}

bool FrameManager::init()
{
    CanvasCoreSubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &FrameManager::onDetachWindows);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowBuilded, &FrameManager::onBuild);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowShowed, &FrameManager::onWindowShowed);
    CanvasCoreSubscribe(signal_DesktopFrame_GeometryChanged, &FrameManager::onGeometryChanged);
    CanvasCoreSubscribe(signal_DesktopFrame_AvailableGeometryChanged, &FrameManager::onGeometryChanged);

    d->canvas = new CanvasInterface(this);
    if (!d->canvas->initialize()) {
        qWarning() << "fail to init CanvasInterface";
        return false;
    }

    d->model = new FileProxyModel(this);
    d->model->setModelShell(d->canvas->fileInfoModel());
    return true;
}

void FrameManager::layout()
{

}

void FrameManager::switchMode(int mode)
{
    if (d->organizer)
        delete d->organizer;

    d->organizer = OrganizerCreator::createOrganizer(static_cast<OrganizerCreator::Mode>(mode));
    Q_ASSERT(d->organizer);

    // 初始化创建集合窗口
    d->organizer->setCanvasShell(d->canvas->canvasModel());
    d->organizer->initialize(d->model);

    // 布局
}

void FrameManager::onBuild()
{
    // todo 是否开启

    d->buildSurface();

    // 读取配置文件获取模式
    // 创建模式
    switchMode(0);
}

void FrameManager::onWindowShowed()
{
    // todo hide the canvas if needed.
}

void FrameManager::onDetachWindows()
{
    for (const SurfacePointer &sur : d->surfaceWidgets.values())
        sur->setParent(nullptr);

    // 解绑集合窗口
}

void FrameManager::onGeometryChanged()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    for (auto win : root) {
        QString screen = win->property(DesktopFrameProperty::kPropScreenName).toString();
        auto surface = d->surfaceWidgets.value(screen);
        if (surface)
            d->layoutSurface(win, surface);
    }

    // 布局集合窗口
}

