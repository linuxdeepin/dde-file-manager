// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "extensionlibmenuscene.h"
#include "private/extensionlibmenuscene_p.h"

namespace dfmplugin_utils {

dfmbase::AbstractMenuScene *ExtensionLibMenuSceneCreator::create()
{
    return new ExtensionLibMenuScene;
}

ExtensionLibMenuScenePrivate::ExtensionLibMenuScenePrivate(ExtensionLibMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
}

ExtensionLibMenuScene::ExtensionLibMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new ExtensionLibMenuScenePrivate(this))
{
}

ExtensionLibMenuScene::~ExtensionLibMenuScene()
{
}

QString ExtensionLibMenuScene::name() const
{
    return ExtensionLibMenuSceneCreator::name();
}

}   // namespace dfmplugin_utils
