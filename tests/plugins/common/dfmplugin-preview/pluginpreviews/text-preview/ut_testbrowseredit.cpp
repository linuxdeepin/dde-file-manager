// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "textbrowseredit.h"

#include <gtest/gtest.h>

#include <QAbstractSlider>

PREVIEW_USE_NAMESPACE

TEST(UT_textBrowserEdit, setFileData)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&TextBrowserEdit::appendText, [ &isOk ]{
        isOk = true;
    });

    TextBrowserEdit edit;
    std::string data("UT_TEST");
    edit.setFileData(data);

    EXPECT_TRUE(isOk);
}

TEST(UT_textBrowserEdit, wheelEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QWheelEvent::angleDelta, []{
        return QPoint(0, -1);
    });
    stub.set_lamda(&QAbstractSlider::value, []{
        return 100;
    });
    stub.set_lamda(&TextBrowserEdit::appendText, []{});
    stub.set_lamda(VADDR(QPlainTextEdit, wheelEvent), [ &isOk ]{
        isOk = true;
    });

    TextBrowserEdit edit;
    edit.filestr = "UT_TEST";
    QWheelEvent event(QPoint(0, 0), 1, Qt::LeftButton, Qt::NoModifier);
    edit.wheelEvent(&event);

    EXPECT_TRUE(isOk);
}

TEST(UT_textBrowserEdit, scrollbarValueChange)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QAbstractSlider::setValue, [ &isOk ]{
        isOk = true;
    });

    TextBrowserEdit edit;
    edit.scrollbarValueChange(100);

    EXPECT_TRUE(isOk);
}

TEST(UT_textBrowserEdit, sliderPositionValueChange)
{
    TextBrowserEdit edit;
    edit.lastPosition = 10;
    edit.filestr = "UT_TEST";
    edit.sliderPositionValueChange(100);

    EXPECT_TRUE(edit.lastPosition == 100);
}

TEST(UT_textBrowserEdit, verifyEndOfStrIntegrity_one)
{
    TextBrowserEdit edit;
    int re = edit.verifyEndOfStrIntegrity(nullptr, 0);

    EXPECT_TRUE(re == 0);
}

TEST(UT_textBrowserEdit, verifyEndOfStrIntegrity_two)
{
    TextBrowserEdit edit;
    int re = edit.verifyEndOfStrIntegrity("UT", 2);

    EXPECT_TRUE(re == 2);
}

TEST(UT_textBrowserEdit, appendText_one)
{
    TextBrowserEdit edit;
    edit.filestr = "UT";
    std::string::iterator data = edit.filestr.begin();
    edit.appendText(data);

    EXPECT_TRUE(edit.filestr.length() == 0);
}
