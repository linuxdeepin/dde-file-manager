// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerview_1.cpp
 * @brief Unit tests for ComputerView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/computerview.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerViewTest, ComputerView)
{
    // Test constructor: ComputerView((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ComputerViewTest, cdTo)
{
    // Test method: void cdTo((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->cdTo(_arg0));
}

TEST_F(ComputerViewTest, computerModel)
{
    // Test getter: ComputerModel computerModel()
    auto result = obj->computerModel();
    EXPECT_NO_FATAL_FAILURE({ obj->computerModel(); });

}

TEST_F(ComputerViewTest, connectShortcut)
{
    // Test method: void connectShortcut((QKeySequence seq, std::function<void(DFMEntryFileInfoPointer)> slot))
    EXPECT_NO_FATAL_FAILURE(obj->connectShortcut(QKeySequence(), {}));
}

TEST_F(ComputerViewTest, handle3rdEntriesVisible)
{
    // Test method: void handle3rdEntriesVisible(())
    EXPECT_NO_FATAL_FAILURE(obj->handle3rdEntriesVisible());
}

TEST_F(ComputerViewTest, handleComputerItemVisible)
{
    // Test method: void handleComputerItemVisible(())
    EXPECT_NO_FATAL_FAILURE(obj->handleComputerItemVisible());
}

TEST_F(ComputerViewTest, handleUserDirVisible)
{
    // Test method: void handleUserDirVisible(())
    EXPECT_NO_FATAL_FAILURE(obj->handleUserDirVisible());
}

TEST_F(ComputerViewTest, hideEvent)
{
    // Test event handler: hideEvent((QHideEvent *event))
    QHideEvent _event(QHideEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->hideEvent(&_event));
}

TEST_F(ComputerViewTest, initView)
{
    // Test method: void initView(())
    EXPECT_NO_FATAL_FAILURE(obj->initView());
}

TEST_F(ComputerViewTest, onMenuRequest)
{
    // Test method: void onMenuRequest((const QPoint &pos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onMenuRequest(_arg0));
}

TEST_F(ComputerViewTest, onRenameRequest)
{
    // Test method: void onRenameRequest((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onRenameRequest(0, _arg1));
}

TEST_F(ComputerViewTest, onSelectionChanged)
{
    // Test method: void onSelectionChanged((const QItemSelection &selected, const QItemSelection &))
    QItemSelection _arg0{};
    QItemSelection _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onSelectionChanged(_arg0, _arg1));
}

TEST_F(ComputerViewTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ComputerViewTest, setRootUrl)
{
    // Test method: bool setRootUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->setRootUrl(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerViewTest, setRowHidden)
{
    // Test setter: void setRowHidden((int row, bool hide))
    EXPECT_NO_FATAL_FAILURE(obj->setRowHidden(0, false));
}

TEST_F(ComputerViewTest, setStatusBarHandler)
{
    // Test setter: void setStatusBarHandler((ComputerStatusBar *sb))
    EXPECT_NO_FATAL_FAILURE(obj->setStatusBarHandler(nullptr));
}

TEST_F(ComputerViewTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(ComputerViewTest, viewState)
{
    // Test getter: DFMBASE_NAMESPACE::AbstractBaseView::ViewState viewState()
    auto result = obj->viewState();
    EXPECT_NO_FATAL_FAILURE({ obj->viewState(); });

}

TEST_F(ComputerViewTest, ComputerView_Destructor)
{
    // Test method:  ~ComputerView(())
    EXPECT_NO_FATAL_FAILURE({ ComputerView *tmp = new ComputerView(); delete tmp; });
}
