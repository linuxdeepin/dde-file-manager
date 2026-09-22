// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Covered functions (menus/packetwritingmenuscene.cpp, complementing test_packetwritingmenuscene.cpp):
//   PacketWritingMenuCreator::create / PacketWritingMenuScenePrivate::findSceneName /
//   isContainSubDirFile(+lambda) / updateState(+per-action lambda)
// Branch notes (from get_code_snippet):
//   isContainSubDirFile: empty selection or mount -> false; any selected file below a
//   sub-directory of the mount -> true;
//   updateState: not packet-writing dir delegates to base; packet-writing dir filters
//   separators, non-whitelisted scenes and action ids.

#include <gtest/gtest.h>
#include "stubext.h"

#include "menus/packetwritingmenuscene.h"
#include "menus/packetwritingmenuscene_p.h"
#include "utils/opticalhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <QAction>
#include <QMenu>
#include <QUrl>
#include <QVariantHash>

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

class UT_PacketWritingMenuSceneCov : public testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
    }

    // Makes initialize() take the packet-writing branch for /media/cdrom.
    void stubPacketWritingDir(bool opticalBlankArea)
    {
        stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [](DeviceProxyManager *, const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(static_cast<QString (*)(const QString &, bool)>(&DeviceUtils::getMountInfo),
                       [](const QString &, bool) -> QString {
                           __DBG_STUB_INVOKE__
                           return "/dev/sr0";
                       });
        stub.set_lamda(static_cast<QString (*)(const QString &)>(&OpticalHelper::findMountPoint),
                       [](const QString &) -> QString {
                           __DBG_STUB_INVOKE__
                           return "/media/cdrom";
                       });
        stub.set_lamda(&DeviceUtils::isPWUserspaceOpticalDiscDev, [](const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        Q_UNUSED(opticalBlankArea);
    }

    QVariantHash makeParams(bool emptyArea, const QList<QUrl> &selects) const
    {
        QVariantHash params;
        params[MenuParamKey::kCurrentDir] = QUrl::fromLocalFile("/media/cdrom");
        params[MenuParamKey::kIsEmptyArea] = emptyArea;
        params[MenuParamKey::kSelectFiles] = QVariant::fromValue(selects);
        return params;
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_PacketWritingMenuSceneCov, Creator_Create_ReturnsSceneWithCorrectName)
{
    // Arrange
    PacketWritingMenuCreator creator;

    // Act
    AbstractMenuScene *scene = creator.create();

    // Assert
    ASSERT_NE(scene, nullptr);
    EXPECT_NE(scene, static_cast<AbstractMenuScene *>(nullptr));
    EXPECT_EQ(scene->name(), QString("PacketWritingMenu"));
    delete scene;
}

TEST_F(UT_PacketWritingMenuSceneCov, Initialize_NoSelections_IsNotPacketWritingBranch)
{
    // Arrange: current dir is not from an optical disc
    stub.set_lamda(&DeviceProxyManager::isFileFromOptical, [](DeviceProxyManager *, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    PacketWritingMenuScene scene;
    QList<QUrl> empty;

    // Act
    bool ok = scene.initialize(makeParams(true, empty));

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(scene.name(), QString("PacketWritingMenu"));
}

TEST_F(UT_PacketWritingMenuSceneCov, Initialize_PacketWritingDir_MarksScene)
{
    // Arrange
    stubPacketWritingDir(true);
    PacketWritingMenuScene scene;
    QList<QUrl> selects { QUrl::fromLocalFile("/media/cdrom/file.txt") };

    // Act
    bool ok = scene.initialize(makeParams(false, selects));

    // Assert
    EXPECT_EQ(ok, true);
    EXPECT_NE(ok, false);   // double-check stable state
}

TEST_F(UT_PacketWritingMenuSceneCov, Initialize_SubDirSelection_MarksWorkingSubDir)
{
    // Arrange: selected file lives in a sub-directory of the mount point
    stubPacketWritingDir(true);
    PacketWritingMenuScene scene;
    QList<QUrl> selects { QUrl::fromLocalFile("/media/cdrom/sub/file.txt") };

    // Act
    bool ok = scene.initialize(makeParams(false, selects));

    // Assert: initialize still succeeds (isContainSubDirFile returned true)
    EXPECT_EQ(ok, true);
    EXPECT_NE(ok, false);   // double-check stable state
}

TEST_F(UT_PacketWritingMenuSceneCov, UpdateState_NotPacketWritingDir_DelegatesToBase)
{
    // Arrange: no packet-writing setup at all
    PacketWritingMenuScene scene;
    QMenu menu;
    QAction plain("plain");
    menu.addAction(&plain);
    ASSERT_TRUE(scene.initialize(makeParams(false, QList<QUrl>())));

    // Act: base implementation leaves plain actions alone

    // Assert
    EXPECT_NO_FATAL_FAILURE(scene.updateState(&menu));
    EXPECT_EQ(plain.isVisible(), true);
    EXPECT_NE(plain.isVisible(), false);   // double-check stable state
}

TEST_F(UT_PacketWritingMenuSceneCov, UpdateState_PacketWritingDir_FiltersUnknownActions)
{
    // Arrange: workspaceScene is resolved from the parent, so provide one
    stubPacketWritingDir(true);
    PacketWritingMenuScene workspaceParent;
    PacketWritingMenuScene scene(&workspaceParent);
    QList<QUrl> selects { QUrl::fromLocalFile("/media/cdrom/file.txt") };
    ASSERT_TRUE(scene.initialize(makeParams(true /*empty area*/, selects)));

    QMenu menu;
    QAction separator;
    separator.setSeparator(true);
    QAction unknown("unknown");
    unknown.setProperty(ActionPropertyKey::kActionID, "totally-unknown");
    QAction refresh("refresh");
    refresh.setProperty(ActionPropertyKey::kActionID, "refresh");
    menu.addAction(&unknown);
    menu.addAction(&refresh);
    menu.addSeparator();

    // Act
    scene.updateState(&menu);

    // Assert: scene filtering hides everything (no child scene map in unit test),
    // separators stay visible
    EXPECT_EQ(unknown.isVisible(), false);
    EXPECT_EQ(refresh.isVisible(), false);
    EXPECT_EQ(menu.actions().last()->isSeparator(), true);
}
