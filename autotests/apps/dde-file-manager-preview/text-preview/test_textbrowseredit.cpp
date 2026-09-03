// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textbrowseredit.h"

#include <dfm-base/dfm_log_defines.h>

#include <gtest/gtest.h>

#include <QApplication>
#include <QFont>
#include <QPointer>
#include <QScrollBar>
#include <QTextCursor>
#include <QWheelEvent>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>

#include <string>

using namespace plugin_filepreview;

namespace {
constexpr int kChunkLimit { 1024 * 1024 * 5 };
}

class UT_TextBrowserEdit : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        edit = new TextBrowserEdit();
    }

    virtual void TearDown() override
    {
        delete edit;
        edit = nullptr;
    }

    QString knownCodeFilePath()
    {
        const auto defs = edit->m_repository.definitions();
        for (const auto &def : defs) {
            if (!def.isValid())
                continue;
            for (const QString &ext : def.extensions()) {
                QString candidate;
                if (ext.startsWith("*."))
                    candidate = "ut_text_sample" + ext.mid(1);
                else if (ext.startsWith('.'))
                    candidate = "ut_text_sample" + ext;
                else
                    candidate = "ut_text_sample." + ext;
                if (edit->m_repository.definitionForFileName(candidate).isValid())
                    return candidate;
            }
        }
        return QString();
    }

protected:
    TextBrowserEdit *edit { nullptr };
};

TEST_F(UT_TextBrowserEdit, Constructor_DefaultArgs_WidgetConfiguredForPreview)
{
    EXPECT_TRUE(edit->isReadOnly());
    EXPECT_EQ(edit->textInteractionFlags(),
              Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    EXPECT_EQ(edit->lineWrapMode(), QPlainTextEdit::WidgetWidth);
    EXPECT_EQ(edit->size(), QSize(800, 500));
    EXPECT_EQ(edit->frameShape(), QFrame::NoFrame);

    QFont font = edit->font();
    EXPECT_EQ(font.family(), QString("Noto Mono"));
    EXPECT_TRUE(font.fixedPitch());
    EXPECT_DOUBLE_EQ(font.pointSizeF(), 10.0);
}

TEST_F(UT_TextBrowserEdit, Destructor_WithActiveHighlighter_DestroysHighlighterWithoutCrash)
{
    const QString path = knownCodeFilePath();
    if (path.isEmpty())
        GTEST_SKIP() << "no syntax definitions available";

    edit->setSyntaxDefinition(path);
    ASSERT_TRUE(edit->m_highlighter);

    QPointer<KSyntaxHighlighting::SyntaxHighlighter> highlighter = edit->m_highlighter;

    delete edit;
    edit = nullptr;

    EXPECT_TRUE(highlighter.isNull());
}

TEST_F(UT_TextBrowserEdit, SetFileData_PlainText_ContentLoadedAndCursorAtStart)
{
    std::string data = "hello world\nsecond line";
    edit->setFileData(data);

    EXPECT_TRUE(edit->toPlainText().contains("hello world"));
    EXPECT_TRUE(edit->toPlainText().contains("second line"));
    EXPECT_EQ(edit->textCursor().position(), 0);
    EXPECT_TRUE(edit->filestr.empty());
}

TEST_F(UT_TextBrowserEdit, SetFileData_EmptyString_NothingLoaded)
{
    std::string data;
    edit->setFileData(data);

    EXPECT_TRUE(edit->toPlainText().isEmpty());
    EXPECT_TRUE(edit->filestr.empty());
}

TEST_F(UT_TextBrowserEdit, SetFileData_DataExceedsChunkLimit_ChunkAppendedAndRemainderKept)
{
    std::string data;
    data.reserve(kChunkLimit + 10);
    while (data.size() < kChunkLimit + 10)
        data += "abcdefghijklmnopqrstuvwxyz0123456789\n";
    data.resize(kChunkLimit + 10);

    edit->setFileData(data);

    EXPECT_EQ(edit->toPlainText().size(), kChunkLimit);
    EXPECT_EQ(static_cast<int>(edit->filestr.size()), 10);
}

TEST_F(UT_TextBrowserEdit, WheelEvent_ScrollDownAtBottomWithPendingData_AppendsPendingData)
{
    edit->filestr = std::string("pending tail");

    QWheelEvent event(QPointF(0, 0), QPointF(0, 0), QPoint(0, -120), QPoint(0, -120),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    edit->wheelEvent(&event);

    EXPECT_TRUE(edit->toPlainText().contains("pending tail"));
    EXPECT_TRUE(edit->filestr.empty());
}

TEST_F(UT_TextBrowserEdit, WheelEvent_ScrollUpWithPendingData_PendingDataNotAppended)
{
    edit->filestr = std::string("pending tail");

    QWheelEvent event(QPointF(0, 0), QPointF(0, 0), QPoint(0, 120), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    edit->wheelEvent(&event);

    EXPECT_FALSE(edit->toPlainText().contains("pending tail"));
    EXPECT_EQ(edit->filestr, std::string("pending tail"));
}

TEST_F(UT_TextBrowserEdit, WheelEvent_ScrollDownAwayFromBottom_PendingDataNotAppended)
{
    QString content;
    for (int i = 0; i < 500; ++i)
        content += QString("line %1\n").arg(i);
    edit->setPlainText(content);
    edit->verticalScrollBar()->setValue(0);
    ASSERT_GT(edit->verticalScrollBar()->maximum(), 0);

    edit->filestr = std::string("tail");

    QWheelEvent event(QPointF(0, 0), QPointF(0, 0), QPoint(0, -120), QPoint(0, -120),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    edit->wheelEvent(&event);

    EXPECT_FALSE(edit->toPlainText().contains("tail"));
    EXPECT_EQ(edit->filestr, std::string("tail"));
}

TEST_F(UT_TextBrowserEdit, ScrollbarValueChange_ValueAtMaximum_NoCrash)
{
    QString content;
    for (int i = 0; i < 500; ++i)
        content += QString("line %1\n").arg(i);
    edit->setPlainText(content);
    ASSERT_GT(edit->verticalScrollBar()->maximum(), 0);

    EXPECT_NO_FATAL_FAILURE(edit->scrollbarValueChange(edit->verticalScrollBar()->maximum()));
}

TEST_F(UT_TextBrowserEdit, ScrollbarValueChange_ValueBelowMaximum_ValueUnchanged)
{
    QString content;
    for (int i = 0; i < 500; ++i)
        content += QString("line %1\n").arg(i);
    edit->setPlainText(content);
    edit->verticalScrollBar()->setValue(0);
    ASSERT_GT(edit->verticalScrollBar()->maximum(), 0);

    edit->scrollbarValueChange(0);

    EXPECT_EQ(edit->verticalScrollBar()->value(), 0);
}

TEST_F(UT_TextBrowserEdit, SliderPositionValueChange_PositionIncreasesToMaximum_AppendsPendingData)
{
    edit->filestr = std::string("chunk two");
    edit->lastPosition = 0;
    ASSERT_EQ(edit->verticalScrollBar()->maximum(), 0);

    edit->sliderPositionValueChange(1);

    EXPECT_TRUE(edit->toPlainText().contains("chunk two"));
    EXPECT_TRUE(edit->filestr.empty());
    EXPECT_EQ(edit->lastPosition, 1);
}

TEST_F(UT_TextBrowserEdit, SliderPositionValueChange_PositionDecreases_PendingDataNotAppended)
{
    edit->filestr = std::string("chunk two");
    edit->lastPosition = 5;

    edit->sliderPositionValueChange(3);

    EXPECT_TRUE(edit->toPlainText().isEmpty());
    EXPECT_EQ(edit->filestr, std::string("chunk two"));
    EXPECT_EQ(edit->lastPosition, 3);
}

TEST_F(UT_TextBrowserEdit, SliderPositionValueChange_PositionIncreasesBelowMaximum_PendingDataNotAppended)
{
    QString content;
    for (int i = 0; i < 500; ++i)
        content += QString("line %1\n").arg(i);
    edit->setPlainText(content);
    edit->lastPosition = 0;
    ASSERT_GT(edit->verticalScrollBar()->maximum(), 1);

    edit->filestr = std::string("tail");

    edit->sliderPositionValueChange(1);

    EXPECT_FALSE(edit->toPlainText().contains("tail"));
    EXPECT_EQ(edit->filestr, std::string("tail"));
    EXPECT_EQ(edit->lastPosition, 1);
}

TEST_F(UT_TextBrowserEdit, SliderPositionValueChange_EmptyPendingData_NothingAppended)
{
    edit->lastPosition = 0;
    ASSERT_EQ(edit->verticalScrollBar()->maximum(), 0);

    edit->sliderPositionValueChange(1);

    EXPECT_TRUE(edit->toPlainText().isEmpty());
    EXPECT_EQ(edit->lastPosition, 1);
}

TEST_F(UT_TextBrowserEdit, SetSyntaxDefinition_KnownCodeFile_HighlighterCreated)
{
    const QString path = knownCodeFilePath();
    if (path.isEmpty())
        GTEST_SKIP() << "no syntax definitions available";

    edit->setSyntaxDefinition(path);

    EXPECT_TRUE(edit->m_highlighter);
}

TEST_F(UT_TextBrowserEdit, SetSyntaxDefinition_UnknownSuffix_HighlighterNotCreated)
{
    edit->setSyntaxDefinition("/tmp/ut_text_no_def.zzzz123");

    EXPECT_FALSE(edit->m_highlighter);
}

TEST_F(UT_TextBrowserEdit, SetSyntaxDefinition_RepeatedCall_OldHighlighterDeleted)
{
    const QString path = knownCodeFilePath();
    if (path.isEmpty())
        GTEST_SKIP() << "no syntax definitions available";

    edit->setSyntaxDefinition(path);
    QPointer<KSyntaxHighlighting::SyntaxHighlighter> oldHighlighter = edit->m_highlighter;
    ASSERT_TRUE(oldHighlighter);

    edit->setSyntaxDefinition(path);

    EXPECT_TRUE(oldHighlighter.isNull());
    EXPECT_TRUE(edit->m_highlighter);
}

TEST_F(UT_TextBrowserEdit, OnThemeTypeChanged_WithHighlighter_NoCrash)
{
    const QString path = knownCodeFilePath();
    if (path.isEmpty())
        GTEST_SKIP() << "no syntax definitions available";

    edit->setSyntaxDefinition(path);
    ASSERT_TRUE(edit->m_highlighter);

    EXPECT_NO_FATAL_FAILURE(edit->onThemeTypeChanged());
}

TEST_F(UT_TextBrowserEdit, OnThemeTypeChanged_WithoutHighlighter_NoCrash)
{
    EXPECT_FALSE(edit->m_highlighter);

    EXPECT_NO_FATAL_FAILURE(edit->onThemeTypeChanged());
}

TEST_F(UT_TextBrowserEdit, UpdateHighlighterTheme_WithHighlighter_NoCrash)
{
    const QString path = knownCodeFilePath();
    if (path.isEmpty())
        GTEST_SKIP() << "no syntax definitions available";

    edit->setSyntaxDefinition(path);
    ASSERT_TRUE(edit->m_highlighter);

    EXPECT_NO_FATAL_FAILURE(edit->updateHighlighterTheme());
}

TEST_F(UT_TextBrowserEdit, UpdateHighlighterTheme_WithoutHighlighter_EarlyReturn)
{
    EXPECT_FALSE(edit->m_highlighter);

    EXPECT_NO_FATAL_FAILURE(edit->updateHighlighterTheme());
    EXPECT_FALSE(edit->m_highlighter);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_NullInput_ReturnsZero)
{
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(nullptr, 10), 0);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_EmptyInput_ReturnsZero)
{
    const char *empty = "";
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(empty, 10), 0);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_NonPositiveLength_ReturnsZero)
{
    const char ascii[] = "abc";
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(ascii, 0), 0);
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(ascii, -5), 0);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_AsciiShorterThanLimit_ReturnsFullLength)
{
    const QByteArray ascii = QByteArrayLiteral("abcdef");
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(ascii.constData(), 10), 6);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_AsciiExactLimit_ReturnsLimit)
{
    const QByteArray ascii = QByteArrayLiteral("abcd");
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(ascii.constData(), 4), 4);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_MultibyteExactLimit_ReturnsByteLength)
{
    const QByteArray multibyte = QByteArray::fromHex("e4bda0");
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(multibyte.constData(), 3), 3);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_MultibyteExceedsLimit_TrimsTrailingCharacter)
{
    const QByteArray multibyte = QByteArray::fromHex("e4bda0e4bda0");
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(multibyte.constData(), 4), 3);
}

TEST_F(UT_TextBrowserEdit, VerifyEndOfStrIntegrity_TruncatedMultibyteAtEnd_ReturnsPartialLength)
{
    const QByteArray truncated = QByteArray::fromHex("61e4");
    EXPECT_EQ(edit->verifyEndOfStrIntegrity(truncated.constData(), 10), 1);
}

TEST_F(UT_TextBrowserEdit, AppendText_EmptyPendingData_NothingInserted)
{
    auto it = edit->filestr.begin();
    edit->appendText(it);

    EXPECT_TRUE(edit->toPlainText().isEmpty());
}

TEST_F(UT_TextBrowserEdit, AppendText_SmallPendingData_InsertsAndClearsPending)
{
    edit->filestr = std::string("hello");
    auto it = edit->filestr.begin();
    edit->appendText(it);

    EXPECT_EQ(edit->toPlainText(), QString("hello"));
    EXPECT_TRUE(edit->filestr.empty());
}
