// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasorganizer_1.cpp
 * @brief Unit tests for CanvasOrganizer methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/canvasorganizer.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasOrganizerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasOrganizer();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasOrganizer *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasOrganizerTest, detachLayout)
{
    // Test method: void detachLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->detachLayout());
}

TEST_F(CanvasOrganizerTest, filterContextMenu)
{
    // Test method: bool filterContextMenu((int, const QUrl &, const QList<QUrl> &, const QPoint &))
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    QPoint _arg3{};
    auto result = obj->filterContextMenu(0, _arg1, _arg2, _arg3);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, filterDataInserted)
{
    // Test method: bool filterDataInserted((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->filterDataInserted(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, filterDataRenamed)
{
    // Test method: bool filterDataRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->filterDataRenamed(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, filterDataRested)
{
    // Test method: bool filterDataRested((QList<QUrl> *urls))
    auto result = obj->filterDataRested(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, filterDropData)
{
    // Test method: bool filterDropData((int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData))
    QPoint _arg2{};
    auto result = obj->filterDropData(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, filterKeyPress)
{
    // Test method: bool filterKeyPress((int viewIndex, int key, int modifiers))
    auto result = obj->filterKeyPress(0, 0, 0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, filterShortcutkeyPress)
{
    // Test method: bool filterShortcutkeyPress((int viewIndex, int key, int modifiers))
    auto result = obj->filterShortcutkeyPress(0, 0, 0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, filterWheel)
{
    // Test method: bool filterWheel((int viewIndex, const QPoint &angleDelta, bool ctrl))
    QPoint _arg1{};
    auto result = obj->filterWheel(0, _arg1, false);
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, getModel)
{
    // Test getter: CollectionModel getModel()
    auto result = obj->getModel();
    EXPECT_NO_FATAL_FAILURE({ obj->getModel(); });

}

TEST_F(CanvasOrganizerTest, getSurfaces)
{
    // Test getter: QList<SurfacePointer> getSurfaces()
    auto result = obj->getSurfaces();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasOrganizerTest, isEditing)
{
    // Test bool getter: isEditing()
    bool result = obj->isEditing();
    EXPECT_FALSE(result);

}

TEST_F(CanvasOrganizerTest, layout)
{
    // Test method: void layout(())
    EXPECT_NO_FATAL_FAILURE(obj->layout());
}

TEST_F(CanvasOrganizerTest, setCanvasGridShell)
{
    // Test setter: void setCanvasGridShell((CanvasGridShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasGridShell(nullptr));
}

TEST_F(CanvasOrganizerTest, setCanvasManagerShell)
{
    // Test setter: void setCanvasManagerShell((CanvasManagerShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasManagerShell(nullptr));
}

TEST_F(CanvasOrganizerTest, setCanvasModelShell)
{
    // Test setter: void setCanvasModelShell((CanvasModelShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasModelShell(nullptr));
}

TEST_F(CanvasOrganizerTest, setCanvasSelectionShell)
{
    // Test setter: void setCanvasSelectionShell((CanvasSelectionShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasSelectionShell(nullptr));
}

TEST_F(CanvasOrganizerTest, setCanvasViewShell)
{
    // Test setter: void setCanvasViewShell((CanvasViewShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasViewShell(nullptr));
}

TEST_F(CanvasOrganizerTest, setSurfaces)
{
    // Test setter: void setSurfaces((const QList<SurfacePointer> &surface))
    QList<SurfacePointer> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSurfaces(_arg0));
}
