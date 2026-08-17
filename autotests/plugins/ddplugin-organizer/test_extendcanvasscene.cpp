// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "menus/extendcanvasscene.h"

#include <QMenu>
#include <QAction>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_ExtendCanvasCreator : public testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test objects
    }

    void TearDown() override
    {
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtendCanvasCreator, Name_ReturnsCorrectName)
{
    QString name = ExtendCanvasCreator::name();
    EXPECT_EQ(name, "OrganizerExtCanvasMenu");
}

TEST_F(UT_ExtendCanvasCreator, Create_ReturnsScene)
{
    ExtendCanvasCreator creator;
    DFMBASE_NAMESPACE::AbstractMenuScene *scene = creator.create();
    EXPECT_NE(scene, nullptr);
    delete scene;
}

class UT_ExtendCanvasScene : public testing::Test
{
protected:
    void SetUp() override
    {
        scene = new ExtendCanvasScene();
    }

    void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ExtendCanvasScene *scene = nullptr;
};

TEST_F(UT_ExtendCanvasScene, Constructor_CreatesScene)
{
    EXPECT_NE(scene, nullptr);
}

TEST_F(UT_ExtendCanvasScene, Name_ReturnsCorrectName)
{
    QString name = scene->name();
    EXPECT_EQ(name, "OrganizerExtCanvasMenu");
}

TEST_F(UT_ExtendCanvasScene, UpdateState_DoesNotCrash)
{
    QMenu parentMenu;
    EXPECT_NO_THROW(scene->updateState(&parentMenu));
}

