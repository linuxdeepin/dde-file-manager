// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MENU_EVENTINTERFACE_HELPER_H
#define MENU_EVENTINTERFACE_HELPER_H

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/abstractscenecreator.h>

#include <dfm-framework/event/event.h>

#define MenuScenePush(topic, args...) \
        dpfSlotChannel->push("dfmplugin_menu", QT_STRINGIFY2(topic), ##args)

namespace dfmplugin_menu_util {

static inline bool menuSceneContains(const QString &name) {
    const QVariant &ret = MenuScenePush(slot_MenuScene_Contains, name);
    return ret.toBool();
}

static inline bool menuSceneRegisterScene(const QString &name, DFMBASE_NAMESPACE::AbstractSceneCreator *creator) {
    const QVariant &ret = MenuScenePush(slot_MenuScene_RegisterScene, name, creator);
    return ret.toBool();
}

static inline DFMBASE_NAMESPACE::AbstractSceneCreator* menuSceneUnregisterScene(const QString &name) {
    const QVariant &ret = MenuScenePush(slot_MenuScene_UnregisterScene, name);
    return ret.value<DFMBASE_NAMESPACE::AbstractSceneCreator *>();
}

static inline bool menuSceneBind(const QString &name, const QString &parent) {
    const QVariant &ret = MenuScenePush(slot_MenuScene_Bind, name, parent);
    return ret.toBool();
}

static inline bool menuSceneUnbind(const QString &name, const QString &parent = QString()) {
    const QVariant &ret = MenuScenePush(slot_MenuScene_Unbind, name, parent);
    return ret.toBool();
}

static inline DFMBASE_NAMESPACE::AbstractMenuScene* menuSceneCreateScene(const QString &name) {
    const QVariant &ret = MenuScenePush(slot_MenuScene_CreateScene, name);
    return ret.value<DFMBASE_NAMESPACE::AbstractMenuScene *>();
}

static inline QVariantHash menuPerfectParams(const QVariantHash &params) {
    const QVariant &ret = MenuScenePush(slot_Menu_PerfectParams, params);
    if (ret.isValid())
        return ret.value<QVariantHash>();
    else
        return params;
}
}



#endif // MENU_EVENTINTERFACE_HELPER_H
