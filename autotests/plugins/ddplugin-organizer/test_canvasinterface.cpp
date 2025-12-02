// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "interface/canvasinterface.h"
#include "interface/canvasinterface_p.h"
#include "interface/fileinfomodelshell.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"
#include "interface/canvasselectionshell.h"

#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;
DPF_USE_NAMESPACE

class UT_CanvasInterface : public testing::Test
{
protected:
    void SetUp() override
    {
        interface = new CanvasInterface();
    }

    void TearDown() override
    {
        delete interface;
        interface = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasInterface *interface = nullptr;
};

TEST_F(UT_CanvasInterface, Constructor_Default_InitializesCorrectly)
{
    EXPECT_NE(interface, nullptr);
    EXPECT_NE(interface->d, nullptr);
}

TEST_F(UT_CanvasInterface, Constructor_WithParent_SetsParent)
{
    QObject parent;
    CanvasInterface interfaceWithParent(&parent);
    EXPECT_EQ(interfaceWithParent.parent(), &parent);
}

TEST_F(UT_CanvasInterface, initialize_CreatesAllShells_ReturnsTrue)
{
    stub.set_lamda(&FileInfoModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasViewShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGridShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasManagerShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasSelectionShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = interface->initialize();
    EXPECT_TRUE(result);
    EXPECT_NE(interface->d->fileInfoModel, nullptr);
    EXPECT_NE(interface->d->canvaModel, nullptr);
    EXPECT_NE(interface->d->canvasView, nullptr);
    EXPECT_NE(interface->d->canvasGrid, nullptr);
    EXPECT_NE(interface->d->canvasManager, nullptr);
    EXPECT_NE(interface->d->canvasSelectionShell, nullptr);
}

TEST_F(UT_CanvasInterface, iconLevel_CallsChannel_ReturnsLevel)
{
    int expectedLevel = 3;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [expectedLevel]() {
        __DBG_STUB_INVOKE__
        return QVariant(expectedLevel);
    });

    int result = interface->iconLevel();
    EXPECT_EQ(result, expectedLevel);
}

TEST_F(UT_CanvasInterface, setIconLevel_CallsChannel_NoReturn)
{
    bool channelCalled = false;
    typedef QVariant (EventChannelManager::*Push)(const QString &, const QString &, int);
    stub.set_lamda(static_cast<Push>(&EventChannelManager::push), [&channelCalled]() {
        __DBG_STUB_INVOKE__
        channelCalled = true;
        return QVariant();
    });

    interface->setIconLevel(2);
    EXPECT_TRUE(channelCalled);
}

TEST_F(UT_CanvasInterface, fileInfoModel_AfterInit_ReturnsShell)
{
    stub.set_lamda(&FileInfoModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasViewShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGridShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasManagerShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasSelectionShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    interface->initialize();
    FileInfoModelShell *shell = interface->fileInfoModel();
    EXPECT_NE(shell, nullptr);
}

TEST_F(UT_CanvasInterface, canvasModel_AfterInit_ReturnsShell)
{
    stub.set_lamda(&FileInfoModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasViewShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGridShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasManagerShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasSelectionShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    interface->initialize();
    CanvasModelShell *shell = interface->canvasModel();
    EXPECT_NE(shell, nullptr);
}

TEST_F(UT_CanvasInterface, canvasView_AfterInit_ReturnsShell)
{
    stub.set_lamda(&FileInfoModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasViewShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGridShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasManagerShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasSelectionShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    interface->initialize();
    CanvasViewShell *shell = interface->canvasView();
    EXPECT_NE(shell, nullptr);
}

TEST_F(UT_CanvasInterface, canvasGrid_AfterInit_ReturnsShell)
{
    stub.set_lamda(&FileInfoModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasViewShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGridShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasManagerShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasSelectionShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    interface->initialize();
    CanvasGridShell *shell = interface->canvasGrid();
    EXPECT_NE(shell, nullptr);
}

TEST_F(UT_CanvasInterface, canvasManager_AfterInit_ReturnsShell)
{
    stub.set_lamda(&FileInfoModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasViewShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGridShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasManagerShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasSelectionShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    interface->initialize();
    CanvasManagerShell *shell = interface->canvasManager();
    EXPECT_NE(shell, nullptr);
}

TEST_F(UT_CanvasInterface, canvasSelectionShell_AfterInit_ReturnsShell)
{
    stub.set_lamda(&FileInfoModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasModelShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasViewShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasGridShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasManagerShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(&CanvasSelectionShell::initialize, []() {
        __DBG_STUB_INVOKE__
        return true;
    });

    interface->initialize();
    CanvasSelectionShell *shell = interface->canvasSelectionShell();
    EXPECT_NE(shell, nullptr);
}

class UT_CanvasInterfacePrivate : public testing::Test
{
protected:
    void SetUp() override
    {
        interface = new CanvasInterface();
    }

    void TearDown() override
    {
        delete interface;
        interface = nullptr;
    }

public:
    CanvasInterface *interface = nullptr;
};

TEST_F(UT_CanvasInterfacePrivate, Constructor_WithInterface_InitializesCorrectly)
{
    EXPECT_NE(interface->d, nullptr);
}

TEST_F(UT_CanvasInterfacePrivate, Members_Default_AreNull)
{
    EXPECT_EQ(interface->d->fileInfoModel, nullptr);
    EXPECT_EQ(interface->d->canvaModel, nullptr);
    EXPECT_EQ(interface->d->canvasView, nullptr);
    EXPECT_EQ(interface->d->canvasGrid, nullptr);
    EXPECT_EQ(interface->d->canvasManager, nullptr);
    EXPECT_EQ(interface->d->canvasSelectionShell, nullptr);
}
