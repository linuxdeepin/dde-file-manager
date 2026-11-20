// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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

#include <algorithm>

#include "stubext.h"

#include "menu/avfsmenuscene.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_menu_defines.h>
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

// PMS: the archive view is shown by the workspace FileView, so Ctrl+Shift+C works
// there. The self-implemented menu must expose the same copy-path capability.
static bool hasActionId(QMenu *menu, const QString &id)
{
    const auto actions = menu->actions();
    return std::any_of(actions.cbegin(), actions.cend(), [&id](QAction *act) {
        return act->property(DFMBASE_NAMESPACE::ActionPropertyKey::kActionID).toString() == id;
    });
}

// The test binary registers no 'file' scheme creator, so inject the FileInfo and
// its isDir attribute instead of touching the filesystem.
using CreateFileInfoFn = QSharedPointer<DFMBASE_NAMESPACE::FileInfo> (*)(
        const QUrl &, const DFMBASE_NAMESPACE::Global::CreateFileInfoType, QString *);

static void stubFileInfo(stub_ext::StubExt &stub, bool isDir)
{
    stub.set_lamda(static_cast<CreateFileInfoFn>(&InfoFactory::create<DFMBASE_NAMESPACE::FileInfo>),
                   [](const QUrl &url, const DFMBASE_NAMESPACE::Global::CreateFileInfoType, QString *) {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<DFMBASE_NAMESPACE::FileInfo>::create(url);
                   });
    stub.set_lamda(VADDR(DFMBASE_NAMESPACE::FileInfo, isAttributes),
                   [isDir](DFMBASE_NAMESPACE::FileInfo *, DFMBASE_NAMESPACE::OptInfoType type) {
                       __DBG_STUB_INVOKE__
                       return type == DFMBASE_NAMESPACE::OptInfoType::kIsDir ? isDir : false;
                   });
}

TEST_F(TestAvfsMenuScene, Create_SelectedFile_HasCopyFilePath)
{
    stubFileInfo(stub, false);

    AvfsMenuScene scene;
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/a.txt") });
    params[MenuParamKey::kIsEmptyArea] = false;
    scene.initialize(params);

    QMenu menu;
    scene.create(&menu);

    EXPECT_TRUE(hasActionId(&menu, "copy-file-path"));
}

TEST_F(TestAvfsMenuScene, Create_SelectedDir_HasCopyFolderPath)
{
    stubFileInfo(stub, true);

    AvfsMenuScene scene;
    QVariantHash params;
    params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/tmp");
    params[MenuParamKey::kSelectFiles] = QVariant::fromValue(QList<QUrl> { QUrl::fromLocalFile("/tmp/sub") });
    params[MenuParamKey::kIsEmptyArea] = false;
    scene.initialize(params);

    QMenu menu;
    scene.create(&menu);

    EXPECT_TRUE(hasActionId(&menu, "copy-folder-path"));
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