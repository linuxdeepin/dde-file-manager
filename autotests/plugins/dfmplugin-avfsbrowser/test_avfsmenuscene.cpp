// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QList>
#include <QProcess>
#include <QStandardPaths>
#include <QMenu>

#include "stubext.h"

#include "menu/avfsmenuscene.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>

#include <dfm-io/dfileinfo.h>

#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE
using namespace dfmbase;

class TestAvfsMenuScene : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// 测试AvfsMenuScene类
TEST_F(TestAvfsMenuScene, CreatorName)
{
    EXPECT_EQ(AvfsMenuSceneCreator::name(), "AvfsMenu");
}

TEST_F(TestAvfsMenuScene, CreatorCreate)
{
    AvfsMenuSceneCreator creator;
    AbstractMenuScene *scene = creator.create();
    EXPECT_NE(scene, nullptr);
    EXPECT_EQ(scene->name(), "AvfsMenu");
    delete scene;
}

TEST_F(TestAvfsMenuScene, Name)
{
    AvfsMenuScene scene;
    EXPECT_EQ(scene.name(), "AvfsMenu");
}

TEST_F(TestAvfsMenuScene, Initialize)
{
    AvfsMenuScene scene;
    QVariantHash params;
    EXPECT_NO_FATAL_FAILURE(scene.initialize(params));
}

TEST_F(TestAvfsMenuScene, Create)
{
    AvfsMenuScene scene;
    QMenu *parent = new QMenu();
    EXPECT_NO_FATAL_FAILURE(scene.create(parent));
    delete parent;
}

TEST_F(TestAvfsMenuScene, UpdateState)
{
    AvfsMenuScene scene;
    QMenu *parent = new QMenu();
    EXPECT_NO_FATAL_FAILURE(scene.updateState(parent));
    delete parent;
}

TEST_F(TestAvfsMenuScene, Triggered)
{
    AvfsMenuScene scene;
    QAction *action = new QAction();
    EXPECT_NO_FATAL_FAILURE(scene.triggered(action));
    delete action;
}

TEST_F(TestAvfsMenuScene, Scene)
{
    AvfsMenuScene scene;
    QAction *action = new QAction();
    EXPECT_NO_FATAL_FAILURE(scene.scene(action));
    delete action;
}

TEST_F(TestAvfsMenuScene, Destructor)
{
    AvfsMenuScene *scene = new AvfsMenuScene();
    EXPECT_NO_FATAL_FAILURE(delete scene);
}