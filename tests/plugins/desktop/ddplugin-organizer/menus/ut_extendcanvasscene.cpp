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

#include "menus/extendcanvasscene_p.h"
#include "menus/organizermenu_defines.h"
#include "config/configpresenter.h"

#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"

#include "dfm-base/dfm_menu_defines.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QAction>

DFMBASE_USE_NAMESPACE
using namespace testing;
using namespace ddplugin_organizer;

TEST(ExtendCanvasScene, initialize_empty)
{
    QUrl dir("file://desktop");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = dir;
    params[MenuParamKey::kOnDesktop] = false;
    params[MenuParamKey::kWindowId] = 1;
    params[MenuParamKey::kIsEmptyArea] = true;
    params[CollectionMenuParams::kOnColletion] = true;
    params[CollectionMenuParams::kColletionView] = 2;

    {
        ExtendCanvasScene scene;
        EXPECT_FALSE(scene.initialize(params));
    }

    {
        stub_ext::StubExt stub;
        stub.set_lamda(&ConfigPresenter::isEnable, []() {
            return true;
        });

        ExtendCanvasScene scene;
        params[MenuParamKey::kOnDesktop] = true;
        EXPECT_TRUE(scene.initialize(params));
        EXPECT_EQ(scene.d->turnOn, true);
        EXPECT_EQ(scene.d->isEmptyArea, true);
        EXPECT_EQ(scene.d->onDesktop, true);
        EXPECT_EQ(scene.d->selectFiles.size(), 0);
        EXPECT_EQ(scene.d->onCollection, true);
        EXPECT_EQ(reinterpret_cast<qintptr>(scene.d->view), 2);
        EXPECT_EQ(scene.d->focusFile.isEmpty(), true);
        EXPECT_EQ(scene.d->currentDir, dir);
    }
}

TEST(ExtendCanvasScene, initialize_normal)
{
    QUrl dir("file://desktop");
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = dir;
    params[MenuParamKey::kOnDesktop] = true;
    params[MenuParamKey::kWindowId] = 1;
    params[MenuParamKey::kIsEmptyArea] = false;
    params[CollectionMenuParams::kOnColletion] = true;
    params[CollectionMenuParams::kColletionView] = 2;
    QList<QUrl> selectUrls {QUrl("file://etc"), QUrl("file://usr")};
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selectUrls);

    stub_ext::StubExt stub;
    stub.set_lamda(&ConfigPresenter::isEnable, []() {
        return false;
    });

    ExtendCanvasScene scene;
    EXPECT_TRUE(scene.initialize(params));
    EXPECT_EQ(scene.d->turnOn, false);
    EXPECT_EQ(scene.d->isEmptyArea, false);
    EXPECT_EQ(scene.d->onDesktop, true);
    EXPECT_EQ(scene.d->onCollection, true);
    EXPECT_EQ(reinterpret_cast<qintptr>(scene.d->view), 2);
    EXPECT_EQ(scene.d->currentDir, dir);

    ASSERT_EQ(scene.d->selectFiles, selectUrls);
    EXPECT_EQ(scene.d->focusFile, selectUrls.first());
}

TEST(ExtendCanvasScene, triggered_organizeoptions)
{
    ExtendCanvasScene scene;

    QAction tempAction;
    scene.d->predicateAction[ActionID::kOrganizeOptions] = &tempAction;
    tempAction.setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeOptions));

    stub_ext::StubExt stub;
    bool show = false;
    stub.set_lamda(&ConfigPresenter::showOptionWindow, [&show]() {
        show = true;
    });

    EXPECT_TRUE(scene.triggered(&tempAction));
    EXPECT_TRUE(show);
}

TEST(ExtendCanvasScenePrivate, triggerSortby)
{
    ExtendCanvasScenePrivate obj(nullptr);

    EXPECT_FALSE(obj.triggerSortby("name"));
    EXPECT_FALSE(obj.triggerSortby("sort-by"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-name"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-time-modified"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-size"));
    EXPECT_TRUE(obj.triggerSortby("sort-by-type"));
}
