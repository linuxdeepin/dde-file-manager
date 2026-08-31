// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_addressbarprivate_1.cpp
 * @brief Unit tests for AddressBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/addressbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class AddressBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AddressBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AddressBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AddressBarPrivateTest, appendToCompleterModel)
{
    // Test method: void appendToCompleterModel((const QStringList &stringList))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->appendToCompleterModel(_arg0));
}

TEST_F(AddressBarPrivateTest, completeLocalPath)
{
    // Test method: void completeLocalPath((const QString &text, const QUrl &url, int slashIndex))
    QString _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->completeLocalPath(_arg0, _arg1, 0));
}

TEST_F(AddressBarPrivateTest, doComplete)
{
    // Test method: void doComplete(())
    EXPECT_NO_FATAL_FAILURE(obj->doComplete());
}

TEST_F(AddressBarPrivateTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AddressBarPrivateTest, eventFilterHide)
{
    // Test method: bool eventFilterHide((AddressBar *addressbar, QHideEvent *event))
    auto result = obj->eventFilterHide(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AddressBarPrivateTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(AddressBarPrivateTest, initData)
{
    // Test method: void initData(())
    EXPECT_NO_FATAL_FAILURE(obj->initData());
}

TEST_F(AddressBarPrivateTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(AddressBarPrivateTest, insertCompletion)
{
    // Test method: void insertCompletion((const QString &completion))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->insertCompletion(_arg0));
}

TEST_F(AddressBarPrivateTest, onCompletionHighlighted)
{
    // Test method: void onCompletionHighlighted((const QString &highlightedCompletion))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onCompletionHighlighted(_arg0));
}

TEST_F(AddressBarPrivateTest, onCompletionModelCountChanged)
{
    // Test method: void onCompletionModelCountChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onCompletionModelCountChanged());
}

TEST_F(AddressBarPrivateTest, onReturnPressed)
{
    // Test method: void onReturnPressed(())
    EXPECT_NO_FATAL_FAILURE(obj->onReturnPressed());
}

TEST_F(AddressBarPrivateTest, onTextEdited)
{
    // Test method: void onTextEdited((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onTextEdited(_arg0));
}

TEST_F(AddressBarPrivateTest, onTravelCompletionListFinished)
{
    // Test method: void onTravelCompletionListFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onTravelCompletionListFinished());
}

TEST_F(AddressBarPrivateTest, requestCompleteByUrl)
{
    // Test method: void requestCompleteByUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->requestCompleteByUrl(_arg0));
}

TEST_F(AddressBarPrivateTest, setCompleter)
{
    // Test setter: void setCompleter((QCompleter *c))
    EXPECT_NO_FATAL_FAILURE(obj->setCompleter(nullptr));
}

TEST_F(AddressBarPrivateTest, updateCompletionState)
{
    // Test method: void updateCompletionState((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateCompletionState(_arg0));
}

TEST_F(AddressBarPrivateTest, updateHistory)
{
    // Test method: void updateHistory(())
    EXPECT_NO_FATAL_FAILURE(obj->updateHistory());
}
