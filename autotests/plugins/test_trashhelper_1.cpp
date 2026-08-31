// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashhelper_1.cpp
 * @brief Unit tests for TrashHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/trashhelper.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashHelperTest, checkCanMove)
{
    // Test method: bool checkCanMove((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->checkCanMove(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, contenxtMenuHandle)
{
    // Test method: void contenxtMenuHandle((const quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->contenxtMenuHandle(0, _arg1, _arg2));
}

TEST_F(TrashHelperTest, createEmptyTrashTopWidget)
{
    // Test getter: QFrame createEmptyTrashTopWidget()
    auto result = obj->createEmptyTrashTopWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->createEmptyTrashTopWidget(); });

}

TEST_F(TrashHelperTest, customColumnRole)
{
    // Test method: bool customColumnRole((const QUrl &rootUrl, QList<Global::ItemRoles> *roleList))
    QUrl _arg0{};
    auto result = obj->customColumnRole(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, customRoleDisplayName)
{
    // Test method: bool customRoleDisplayName((const QUrl &url, const Global::ItemRoles role, QString *displayName))
    QUrl _arg0{};
    auto result = obj->customRoleDisplayName(_arg0, Global::ItemRoles(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, detailExtensionFunc)
{
    // Test method: TrashHelper::ExpandFieldMap detailExtensionFunc((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->detailExtensionFunc(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TrashHelperTest, detailViewIcon)
{
    // Test method: bool detailViewIcon((const QUrl &url, QString *iconName))
    QUrl _arg0{};
    auto result = obj->detailViewIcon(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, handleWindowUrlChanged)
{
    // Test method: void handleWindowUrlChanged((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleWindowUrlChanged(0, _arg1));
}

TEST_F(TrashHelperTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(TrashHelperTest, initEvent)
{
    // Test method: void initEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->initEvent());
}

TEST_F(TrashHelperTest, onTrashEmptyState)
{
    // Test method: void onTrashEmptyState(())
    EXPECT_NO_FATAL_FAILURE(obj->onTrashEmptyState());
}

TEST_F(TrashHelperTest, onTrashNotEmptyState)
{
    // Test method: void onTrashNotEmptyState(())
    EXPECT_NO_FATAL_FAILURE(obj->onTrashNotEmptyState());
}

TEST_F(TrashHelperTest, onTrashStateChanged)
{
    // Test method: void onTrashStateChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onTrashStateChanged());
}

TEST_F(TrashHelperTest, propetyExtensionFunc)
{
    // Test method: TrashHelper::ExpandFieldMap propetyExtensionFunc((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->propetyExtensionFunc(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(TrashHelperTest, restoreFromTrashHandle)
{
    // Test method: JobHandlePointer restoreFromTrashHandle((const quint64 windowId, const QList<QUrl> urls, const AbstractJobHandler::JobFlags flags))
    auto result = obj->restoreFromTrashHandle(0, QList<QUrl>(), AbstractJobHandler::JobFlags());
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(TrashHelperTest, showTopWidget)
{
    // Test method: bool showTopWidget((QWidget *w, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->showTopWidget(nullptr, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(TrashHelperTest, transToTrashFile)
{
    // Test method: QUrl transToTrashFile((const QString &filePath))
    QString _arg0{};
    auto result = obj->transToTrashFile(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TrashHelperTest, trashFileToTargetUrl)
{
    // Test method: QUrl trashFileToTargetUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->trashFileToTargetUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TrashHelperTest, trashNotEmpty)
{
    // Test method: void trashNotEmpty(())
    EXPECT_NO_FATAL_FAILURE(obj->trashNotEmpty());
}
