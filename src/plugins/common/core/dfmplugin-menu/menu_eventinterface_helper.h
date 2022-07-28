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
#ifndef MENU_EVENTINTERFACE_HELPER_H
#define MENU_EVENTINTERFACE_HELPER_H

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

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
    return ret.value<QVariantHash>();
}
}



#endif // MENU_EVENTINTERFACE_HELPER_H
