// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/shred/shredmenuscene.h"
#include "plugins/common/dfmplugin-utils/shred/shredutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/universalutils.h>

#include <QMenu>
#include <QAction>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

// ========== ShredMenuCreator Tests ==========

class UT_ShredMenuCreator : public testing::Test
{
protected:
    void SetUp() override
    {
        creator = new ShredMenuCreator();
    }

    void TearDown() override
    {
        delete creator;
        creator = nullptr;
        stub.clear();
    }

    ShredMenuCreator *creator { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ShredMenuCreator, name_ReturnsShredMenu)
{
    EXPECT_EQ(ShredMenuCreator::name(), "ShredMenu");
}

TEST_F(UT_ShredMenuCreator, create_ReturnsNewScene)
{
    AbstractMenuScene *scene = creator->create();

    EXPECT_NE(scene, nullptr);

    delete scene;
}

// ========== ShredMenuScene Tests ==========

class UT_ShredMenuScene : public testing::Test
{
protected:
    void SetUp() override
    {
        scene = new ShredMenuScene();
    }

    void TearDown() override
    {
        delete scene;
        scene = nullptr;
        stub.clear();
    }

    ShredMenuScene *scene { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_ShredMenuScene, Constructor_CreatesScene)
{
    EXPECT_NE(scene, nullptr);
}

TEST_F(UT_ShredMenuScene, name_ReturnsShredMenu)
{
    EXPECT_EQ(scene->name(), "ShredMenu");
}

TEST_F(UT_ShredMenuScene, initialize_SetsParameters)
{
    QVariantHash params;
    params.insert("kCurrentDir", QUrl::fromLocalFile("/tmp"));
    params.insert("kSelectFiles", QVariant::fromValue(QList<QUrl>({ QUrl::fromLocalFile("/tmp/test.txt") })));
    params.insert("kIsEmptyArea", false);
    params.insert("kWindowId", 1ULL);

    bool result = scene->initialize(params);

    EXPECT_TRUE(result);
}

TEST_F(UT_ShredMenuScene, create_ShredDisabled_ReturnsFalse)
{
    stub.set_lamda(ADDR(ShredUtils, isShredEnabled),
                   [](ShredUtils *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    QMenu menu;
    bool result = scene->create(&menu);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredMenuScene, create_EmptyArea_ReturnsFalse)
{
    stub.set_lamda(ADDR(ShredUtils, isShredEnabled),
                   [](ShredUtils *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    QVariantHash params;
    params.insert("kIsEmptyArea", true);
    scene->initialize(params);

    QMenu menu;
    bool result = scene->create(&menu);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredMenuScene, create_NullMenu_ReturnsFalse)
{
    stub.set_lamda(ADDR(ShredUtils, isShredEnabled),
                   [](ShredUtils *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    bool result = scene->create(nullptr);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredMenuScene, triggered_UnknownAction_ReturnsFalse)
{
    QAction unknownAction;

    bool result = scene->triggered(&unknownAction);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredMenuScene, scene_NullAction_ReturnsNull)
{
    AbstractMenuScene *result = scene->scene(nullptr);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ShredMenuScene, scene_UnknownAction_ReturnsNull)
{
    QAction unknownAction;

    AbstractMenuScene *result = scene->scene(&unknownAction);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_ShredMenuScene, updateState_EmptyPredicateAction_Returns)
{
    QMenu menu;

    scene->updateState(&menu);
}

TEST_F(UT_ShredMenuScene, create_VirtualUrl_ReturnsFalse)
{
    stub.set_lamda(ADDR(ShredUtils, isShredEnabled),
                   [](ShredUtils *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(qOverload<const QUrl &>(&UrlRoute::isVirtual),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&UniversalUtils::urlTransformToLocal,
                   [](const QUrl &, QUrl *target) -> bool {
                       __DBG_STUB_INVOKE__
                       if (target)
                           *target = QUrl("trash:///");
                       return true;
                   });

    QVariantHash params;
    params.insert("kCurrentDir", QUrl("trash:///"));
    params.insert("kSelectFiles", QVariant::fromValue(QList<QUrl>({ QUrl::fromLocalFile("/tmp/test.txt") })));
    params.insert("kIsEmptyArea", false);
    scene->initialize(params);

    QMenu menu;
    bool result = scene->create(&menu);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredMenuScene, create_NullFileInfo_ReturnsFalse)
{
    stub.set_lamda(ADDR(ShredUtils, isShredEnabled),
                   [](ShredUtils *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(qOverload<const QUrl &>(&UrlRoute::isVirtual),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    QVariantHash params;
    params.insert("kCurrentDir", QUrl::fromLocalFile("/tmp"));
    params.insert("kSelectFiles", QVariant::fromValue(QList<QUrl>({ QUrl::fromLocalFile("/tmp/test.txt") })));
    params.insert("kIsEmptyArea", false);
    scene->initialize(params);

    QMenu menu;
    bool result = scene->create(&menu);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredMenuScene, create_InvalidFile_ReturnsFalse)
{
    stub.set_lamda(ADDR(ShredUtils, isShredEnabled),
                   [](ShredUtils *) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(qOverload<const QUrl &>(&UrlRoute::isVirtual),
                   [](const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(QUrl::fromLocalFile("/tmp/test.txt")));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(ADDR(ShredUtils, isValidFile),
                   [](ShredUtils *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    QVariantHash params;
    params.insert("kCurrentDir", QUrl::fromLocalFile("/tmp"));
    params.insert("kSelectFiles", QVariant::fromValue(QList<QUrl>({ QUrl::fromLocalFile("/tmp/test.txt") })));
    params.insert("kIsEmptyArea", false);
    scene->initialize(params);

    QMenu menu;
    bool result = scene->create(&menu);

    EXPECT_FALSE(result);
}

TEST_F(UT_ShredMenuScene, initialize_EmptySelectFiles_SetsFocusFileEmpty)
{
    QVariantHash params;
    params.insert("kCurrentDir", QUrl::fromLocalFile("/tmp"));
    params.insert("kSelectFiles", QVariant::fromValue(QList<QUrl>()));
    params.insert("kIsEmptyArea", false);
    params.insert("kWindowId", 1ULL);

    bool result = scene->initialize(params);

    EXPECT_TRUE(result);
}

TEST_F(UT_ShredMenuScene, triggered_UnknownAction_CallsParent)
{
    QAction unknownAction;
    unknownAction.setProperty("kActionID", "unknown-action");

    bool result = scene->triggered(&unknownAction);

    EXPECT_FALSE(result);
}

