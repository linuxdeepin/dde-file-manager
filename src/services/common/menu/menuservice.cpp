/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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

#include "private/menuservice_p.h"

#include <dfm-framework/framework.h>

#include <QThread>
#include <QCoreApplication>

#include <mutex>

DSC_BEGIN_NAMESPACE

DFMBASE_USE_NAMESPACE

/*!
 * \brief MenuService::createMenu: Create a menu corresponding to the scene.
 * \param parent: The parent of the menu, used to delete the menu when the parent is deleted.
 * \param scene: The name of the scene where the menu needs to be created.
 * \param mode: Create a blank area or normal menu.
 * \param rootUrl: The root URL of the current scene
 * \param focusUrl: The URL of the focus file where the mouse is when the right-click menu is located.
 * \param selected: The list of file urls selected when the right-click menu is displayed.
 * \param flags: Right-click menu selection mode (eg: single file, single folder,
 *               multiple files, multiple folders, files and folders, etc.)
 * \param customData: some custom data when creating the menu (if needed).
 * \return Return menu
 */

MenuServicePrivate::MenuServicePrivate(MenuService *parent)
    : QObject(parent), q(parent)
{
    setObjectName("dfm_service_desktop::ScreenServicePrivate");
}

MenuServicePrivate::~MenuServicePrivate()
{
    auto tmp = creators;
    creators.clear();

    for (auto it = tmp.begin(); it != tmp.end(); ++it)
        delete it.value();
}

void MenuServicePrivate::createSubscene(AbstractSceneCreator *creator, AbstractMenuScene *parent) const
{
    if (!parent) {
        qDebug() << "target  SceneCreator not exist!!!";
        return;
    }

    if (!creator) {
        qDebug() << "target MenuScene not exist!!!";
        return;
    }

    for (const QString &child : creator->getChildren()) {
        if (auto sub = q->createScene(child))
            parent->addSubscene(sub);
    }
}

MenuService *MenuService::service()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (QThread::currentThread() != qApp->thread()) {
            qCritical() << name() << "invaild thread: " << QThread::currentThread()
                        << ". service must be created in main thread." << qApp->thread();
            abort();
        }

        if (!ctx.load(name()))
            abort();
    });

    return ctx.service<MenuService>(name());
}

bool MenuService::contains(const QString &name) const
{
    QReadLocker lk(&d->locker);
    return d->creators.contains(name);
}

bool MenuService::registerScene(const QString &name, AbstractSceneCreator *creator)
{
    QWriteLocker lk(&d->locker);
    if (d->creators.contains(name) || !creator || name.isEmpty())
        return false;

    d->creators.insert(name, creator);
    lk.unlock();

    emit sceneAdded(name);
    return true;
}

AbstractSceneCreator *MenuService::unregisterScene(const QString &name)
{
    QWriteLocker lk(&d->locker);
    auto scene = d->creators.take(name);
    lk.unlock();

    unBind(name);

    if (scene)
        emit sceneRemoved(name);

    return scene;
}

bool MenuService::bind(const QString &name, const QString &parent)
{
    QReadLocker lk(&d->locker);
    if (!d->creators.contains(name) || !d->creators.contains(parent))
        return false;

    auto creator = d->creators.value(parent);

    if (!creator)
        return false;

    return creator->addChild(name);
}

void MenuService::unBind(const QString &name, const QString &parent)
{
    if (name.isEmpty())
        return;

    QReadLocker lk(&d->locker);
    if (parent.isEmpty()) {
        for (auto it = d->creators.begin(); it != d->creators.end(); ++it)
            it.value()->removeChild(name);
    } else {
        auto creator = d->creators.value(parent);
        if (creator)
            creator->removeChild(name);
    }
}

AbstractMenuScene *MenuService::createScene(const QString &name) const
{
    QReadLocker lk(&d->locker);

    AbstractMenuScene *top = nullptr;
    auto it = d->creators.find(name);
    if (it == d->creators.end())
        return top;

    auto creator = it.value();
    lk.unlock();

    if (creator)
        top = creator->create();

    if (!top)
        return top;

    d->createSubscene(creator, top);
    return top;
}

MenuService::MenuService(QObject *parent)
    : dpf::PluginService(parent), dpf::AutoServiceRegister<MenuService>(), d(new MenuServicePrivate(this))
{
}

MenuService::~MenuService()
{
}

DSC_END_NAMESPACE
