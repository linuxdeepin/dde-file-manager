/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef STUBMENUEVENTINTERFACE_H
#define STUBMENUEVENTINTERFACE_H

#include "stubext.h"
#include "addr_any.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <map>

namespace shared_stubs {
static inline bool stubContains(const QString &)
{
    __DBG_STUB_INVOKE__
    return true;
}

static inline bool stubRegScene(const QString &, DFMBASE_NAMESPACE::AbstractSceneCreator *creator)
{
    __DBG_STUB_INVOKE__
    if (creator)
        delete creator;
    return true;
}

static inline DFMBASE_NAMESPACE::AbstractSceneCreator* stubUnregScene(const QString &)
{
    __DBG_STUB_INVOKE__
    return nullptr;
}

static inline bool stubBindMenu(const QString &, const QString &)
{
    __DBG_STUB_INVOKE__
    return true;
}

static inline bool stubUnbindMenu(const QString &, const QString &)
{
    __DBG_STUB_INVOKE__
    return true;
}

static inline DFMBASE_NAMESPACE::AbstractMenuScene* stubCreateScene(const QString &)
{
    __DBG_STUB_INVOKE__
    return nullptr;
}

static inline QVariantHash stubPerfectParams(const QVariantHash &)
{
    __DBG_STUB_INVOKE__
    return QVariantHash();
}


namespace func_names {
inline constexpr char kMenuSceneContains[] {"dfmplugin_menu_util::menuSceneContains(QString const&)"};
inline constexpr char kMenuSceneRegisterScene[] {"dfmplugin_menu_util::menuSceneRegisterScene(QString const&, dfmbase::AbstractSceneCreator*)"};
inline constexpr char kMenuSceneUnregisterScene[] {"dfmplugin_menu_util::menuSceneUnregisterScene(QString const&)"};
inline constexpr char kMenuSceneBind[] {"dfmplugin_menu_util::menuSceneBind(QString const&, QString const&)"};
inline constexpr char kMenuSceneUnbind[] {"dfmplugin_menu_util::menuSceneUnbind(QString const&, QString const&)"};
inline constexpr char kMenuSceneCreateScene[] {"dfmplugin_menu_util::menuSceneCreateScene(QString const&)"};
inline constexpr char kMenuPerfectParams[] {"dfmplugin_menu_util::menuPerfectParams(QVariantHash const&)"};
}
static void stubAllMenuInterfaces(Stub *stub)
{
    if (!stub)
        return;

    using namespace func_names;
    ::AddrAny any;
    std::map<std::string, void *> ret;
    any.get_local_func_addr_symtab("^dfmplugin_menu_util", ret);

    auto findFunc = [&ret](std::string funcName) -> void * {
        auto iter = ret.find(funcName);
        if (iter != ret.end())
            return iter->second;
        return nullptr;
    };

    auto contains = findFunc(kMenuSceneContains);
    if (contains) {
        stub->set(contains, stubContains);
    }

    auto regScene = findFunc(kMenuSceneRegisterScene);
    if (regScene) {
        stub->set(regScene, stubRegScene);
    }

    auto unregScene = findFunc(kMenuSceneUnregisterScene);
    if (unregScene) {
        stub->set(unregScene, stubUnregScene);
    }

    auto bind = findFunc(kMenuSceneBind);
    if (bind) {
        stub->set(bind, stubBindMenu);
    }

    auto unbind = findFunc(kMenuSceneUnbind);
    if (unbind) {
        stub->set(unbind, stubUnbindMenu);
    }

    auto create = findFunc(kMenuSceneCreateScene);
    if (create) {
        stub->set(create, stubCreateScene);
    }

    auto params = findFunc(kMenuPerfectParams);
    if (params) {
        stub->set(params, stubPerfectParams);
    }
}
}

#endif // STUBMENUEVENTINTERFACE_H
