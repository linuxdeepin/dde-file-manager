// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menu.h"
#include "utils/menuhelper.h"
#include "menuscene/clipboardmenuscene.h"
#include "menuscene/opendirmenuscene.h"
#include "menuscene/fileoperatormenuscene.h"
#include "menuscene/openwithmenuscene.h"
#include "menuscene/newcreatemenuscene.h"
#include "menuscene/sharemenuscene.h"
#include "menuscene/menuutils.h"
#include "menuscene/sendtomenuscene.h"
#include "menuscene/dconfighiddenmenuscene.h"
#include "menuscene/actioniconmenuscene.h"
#include "extendmenuscene/extendmenuscene.h"
#include "extendmenuscene/extendmenu/dcustomactionparser.h"
#include "oemmenuscene/oemmenuscene.h"
#include "oemmenuscene/oemmenu.h"
#include "oemmenuscene/extensionmonitor.h"
#include "templatemenuscene/templatemenuscene.h"
#include "templatemenuscene/templatemenu.h"

#include <QWidget>
#include <QApplication>
#include <QThread>

namespace dfmplugin_menu {
DFM_LOG_REGISTER_CATEGORY(DPMENU_NAMESPACE)

DFMBASE_USE_NAMESPACE

#define MenuHandlePublish(topic, args...) \
    dpfSignalDispatcher->publish(QT_STRINGIFY(DPMENU_NAMESPACE), QT_STRINGIFY2(topic), ##args)

#define MenuHandleSlot(topic, args...) \
    dpfSlotChannel->connect(QT_STRINGIFY(DPMENU_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define MenuHandleDisconnect(topic) \
    dpfSlotChannel->disconnect(QT_STRINGIFY(DPMENU_NAMESPACE), QT_STRINGIFY2(topic))

MenuHandle::MenuHandle(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
}

MenuHandle::~MenuHandle()
{
    auto tmp = creators;
    creators.clear();

    for (auto it = tmp.begin(); it != tmp.end(); ++it)
        delete it.value();

    MenuHandleDisconnect(slot_MenuScene_Contains);
    MenuHandleDisconnect(slot_MenuScene_RegisterScene);
    MenuHandleDisconnect(slot_MenuScene_UnregisterScene);
    MenuHandleDisconnect(slot_MenuScene_Bind);
    MenuHandleDisconnect(slot_MenuScene_Unbind);
    MenuHandleDisconnect(slot_MenuScene_CreateScene);

    MenuHandleDisconnect(slot_Menu_PerfectParams);
}

bool MenuHandle::init()
{
    MenuHandleSlot(slot_MenuScene_Contains, &MenuHandle::contains);
    MenuHandleSlot(slot_MenuScene_RegisterScene, &MenuHandle::registerScene);
    MenuHandleSlot(slot_MenuScene_UnregisterScene, &MenuHandle::unregisterScene);
    MenuHandleSlot(slot_MenuScene_Bind, &MenuHandle::bind);
    MenuHandleSlot(slot_MenuScene_Unbind, &MenuHandle::unbind);
    MenuHandleSlot(slot_MenuScene_CreateScene, &MenuHandle::createScene);

    MenuHandleSlot(slot_Menu_PerfectParams, &MenuHandle::perfectMenuParams);
    MenuHandleSlot(slot_Menu_IsDisable, &MenuHandle::isMenuDisable);

    registerScene(NewCreateMenuCreator::name(), new NewCreateMenuCreator);   // 注册新建场景
    registerScene(ClipBoardMenuCreator::name(), new ClipBoardMenuCreator);   // 注册剪切板场景
    registerScene(OpenDirMenuCreator::name(), new OpenDirMenuCreator);   // 注册文件夹场景
    registerScene(FileOperatorMenuCreator::name(), new FileOperatorMenuCreator);   // 注册文件场景
    registerScene(OpenWithMenuCreator::name(), new OpenWithMenuCreator);
    registerScene(ShareMenuCreator::name(), new ShareMenuCreator);
    registerScene(ExtendMenuCreator::name(), new ExtendMenuCreator);
    registerScene(OemMenuCreator::name(), new OemMenuCreator);
    registerScene(TemplateMenuCreator::name(), new TemplateMenuCreator);
    registerScene(SendToMenuCreator::name(), new SendToMenuCreator);
    registerScene(DConfigHiddenMenuCreator::name(), new DConfigHiddenMenuCreator);
    registerScene(ActionIconMenuCreator::name(), new ActionIconMenuCreator);

    return true;
}

bool MenuHandle::contains(const QString &name)
{
    QReadLocker lk(&locker);
    return creators.contains(name);
}

bool MenuHandle::registerScene(const QString &name, AbstractSceneCreator *creator)
{
    QWriteLocker lk(&locker);
    if (creators.contains(name) || !creator || name.isEmpty())
        return false;

    creators.insert(name, creator);
    lk.unlock();

    publishSceneAdded(name);
    return true;
}

AbstractSceneCreator *MenuHandle::unregisterScene(const QString &name)
{
    QWriteLocker lk(&locker);
    auto scene = creators.take(name);
    lk.unlock();

    unbind(name);

    if (scene)
        emit publishSceneRemoved(name);

    return scene;
}

bool MenuHandle::bind(const QString &name, const QString &parent)
{
    QReadLocker lk(&locker);
    if (!creators.contains(name) || !creators.contains(parent))
        return false;

    auto creator = creators.value(parent);

    if (!creator)
        return false;

    return creator->addChild(name);
}

void MenuHandle::unbind(const QString &name, const QString &parent)
{
    if (name.isEmpty())
        return;

    QReadLocker lk(&locker);
    if (parent.isEmpty()) {
        for (auto it = creators.begin(); it != creators.end(); ++it)
            it.value()->removeChild(name);
    } else {
        auto creator = creators.value(parent);
        if (creator)
            creator->removeChild(name);
    }
}

dfmbase::AbstractMenuScene *MenuHandle::createScene(const QString &name)
{
    QReadLocker lk(&locker);

    AbstractMenuScene *top = nullptr;
    auto it = creators.find(name);
    if (it == creators.end())
        return top;

    auto creator = it.value();
    lk.unlock();

    if (creator)
        top = creator->create();

    if (!top)
        return top;

    createSubscene(creator, top);
    return top;
}

QVariantHash MenuHandle::perfectMenuParams(const QVariantHash &params)
{
    return MenuUtils::perfectMenuParams(params);
}

bool MenuHandle::isMenuDisable(const QVariantHash &params)
{
    QString app = params.value("ApplicationName").toString();
    if (app.isEmpty())
        app = qApp->applicationName();
    return Helper::isHiddenMenu(app);
}

void MenuHandle::publishSceneAdded(const QString &scene)
{
    MenuHandlePublish(signal_MenuScene_SceneAdded, scene);
}

void MenuHandle::publishSceneRemoved(const QString &scene)
{
    MenuHandlePublish(signal_MenuScene_SceneRemoved, scene);
}

void MenuHandle::createSubscene(AbstractSceneCreator *creator, AbstractMenuScene *parent)
{
    if (!parent) {
        fmDebug() << "target  SceneCreator not exist!!!";
        return;
    }

    if (!creator) {
        fmDebug() << "target MenuScene not exist!!!";
        return;
    }

    for (const QString &child : creator->getChildren()) {
        if (auto sub = createScene(child))
            parent->addSubscene(sub);
    }
}

void Menu::initialize()
{
    handle = new MenuHandle();
    handle->init();

    qApp->setProperty("_d_menu_keyboardsearch_disabled", true);
    qApp->setProperty("_d_menu_underlineshortcut", true);
}

bool Menu::start()
{
    const auto &appName = qApp->applicationName();
    if (appName == "org.deepin.dde-shell" || appName == "dde-desktop")
        ExtensionMonitor::instance()->start();
    return true;
}

void Menu::stop()
{
    delete handle;
    handle = nullptr;
}
}   // namespace dfmplugin_menu
