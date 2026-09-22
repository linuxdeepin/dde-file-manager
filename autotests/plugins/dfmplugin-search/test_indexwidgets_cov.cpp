// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map (all offscreen, widgets only touched through public API + signals)
// utils/checkboxwithtextindex.cpp:
//   CheckBoxWithTextIndex(QWidget*) ................. TextCheckBox_Construction_WiresController
//   ctor {lambda(bool)#1} ........................... TextCheckBox_FullTextSearchChanged_SyncsState
//   connectToBackend() ............................... TextCheckBox_ConnectToBackend_NoCrash
//   initStatusBar() .................................. TextCheckBox_InitStatusBar_UncheckedInactive
//   (header tr) ..................................... via construction
// utils/checkboxwithocrindex.cpp:
//   CheckBoxWithOcrIndex(QWidget*) ................... OcrCheckBox_Construction_WiresController
//   ctor {lambda(bool)#1} ........................... OcrCheckBox_OcrSearchChanged_SyncsState
//   connectToBackend() / initStatusBar() ............. OcrCheckBox_BackendAndStatusBar_NoStateChange
// utils/checkboxwithsemanticindex.cpp:
//   CheckBoxWithSemanticIndex(QWidget*) .............. SemanticCheckBox_Construction_ReadsFileIndexState
//   ctor {lambda(bool)#1} ........................... SemanticCheckBox_SemanticSearchChanged_SetsChecked
//   initStatusBar() .................................. SemanticCheckBox_InitStatusBar_StaysInactive
//   setDisabledByFileIndex(bool) ..................... SemanticCheckBox_SetDisabledByFileIndex_TogglesEnabled
// utils/indexstatuscheckbox.cpp:
//   ctor {lambda(QString)#1} (linkActivated) ......... StatusBox_LinkActivated_ForwardsResetRequested
//   ctor {lambda(bool)#2} (clicked) .................. StatusBox_Clicked_AcceptsChangeAndEmits
//   setDisplayText(QString) .......................... StatusBox_SetDisplayText_UpdatesCheckBoxText
//   acceptCheckStateChange(Qt::CheckState,Qt::CheckState) ... StatusBox_Clicked_AcceptsChangeAndEmits (base virtual)
// ============================================================================

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCheckBox>
#include <QLabel>
#include <QVariant>
#include <QList>

#include "utils/indexstatuscheckbox.h"
#include "utils/checkboxwithtextindex.h"
#include "utils/checkboxwithocrindex.h"
#include "utils/checkboxwithsemanticindex.h"
#include "searchmanager/searchmanager.h"

using namespace dfmplugin_search;

namespace {

// The message label is the only word-wrapping DTipLabel owned by the box.
QLabel *messageLabelOf(IndexStatusCheckBox *box)
{
    const auto labels = box->findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->wordWrap())
            return label;
    }
    return nullptr;
}

}   // namespace

class UT_IndexWidgetsCov : public testing::Test
{
protected:
    void TearDown() override
    {
        qDeleteAll(widgets);
        widgets.clear();
    }

    template<typename T>
    T *track(T *widget)
    {
        widgets.append(widget);
        return widget;
    }

    QList<QWidget *> widgets;
};

// ---------- CheckBoxWithTextIndex ----------

TEST_F(UT_IndexWidgetsCov, TextCheckBox_Construction_WiresController)
{
    // Arrange
    const IndexStatusCheckBox::Status initialStatus = IndexStatusCheckBox::Status::Inactive;

    // Act
    auto *box = track(new CheckBoxWithTextIndex());

    // Assert
    EXPECT_EQ(box->status(), initialStatus);
    EXPECT_FALSE(box->isChecked());
}

TEST_F(UT_IndexWidgetsCov, TextCheckBox_FullTextSearchChanged_SyncsState)
{
    // Arrange
    auto *box = track(new CheckBoxWithTextIndex());
    EXPECT_FALSE(box->isChecked());

    // Act
    emit SearchManager::instance()->enableFullTextSearchChanged(true);
    const bool afterEnable = box->isChecked();
    emit SearchManager::instance()->enableFullTextSearchChanged(false);

    // Assert
    EXPECT_TRUE(afterEnable);
    EXPECT_FALSE(box->isChecked());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_IndexWidgetsCov, TextCheckBox_ConnectToBackend_NoCrash)
{
    // Arrange
    auto *box = track(new CheckBoxWithTextIndex());

    // Act
    box->connectToBackend();

    // Assert
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);   // backend absent, state untouched
    EXPECT_FALSE(box->isChecked());
}

TEST_F(UT_IndexWidgetsCov, TextCheckBox_InitStatusBar_UncheckedInactive)
{
    // Arrange
    auto *box = track(new CheckBoxWithTextIndex());

    // Act
    box->initStatusBar();

    // Assert
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
    EXPECT_FALSE(box->isChecked());
}

// ---------- CheckBoxWithOcrIndex ----------

TEST_F(UT_IndexWidgetsCov, OcrCheckBox_Construction_WiresController)
{
    // Arrange
    const IndexStatusCheckBox::Status initialStatus = IndexStatusCheckBox::Status::Inactive;

    // Act
    auto *box = track(new CheckBoxWithOcrIndex());

    // Assert
    EXPECT_EQ(box->status(), initialStatus);
    EXPECT_FALSE(box->isChecked());
}

TEST_F(UT_IndexWidgetsCov, OcrCheckBox_OcrSearchChanged_SyncsState)
{
    // Arrange
    auto *box = track(new CheckBoxWithOcrIndex());

    // Act
    emit SearchManager::instance()->enableOcrTextSearchChanged(true);
    const bool afterEnable = box->isChecked();
    emit SearchManager::instance()->enableOcrTextSearchChanged(false);

    // Assert
    EXPECT_TRUE(afterEnable);
    EXPECT_FALSE(box->isChecked());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_IndexWidgetsCov, OcrCheckBox_BackendAndStatusBar_NoStateChange)
{
    // Arrange
    auto *box = track(new CheckBoxWithOcrIndex());

    // Act
    box->connectToBackend();
    box->initStatusBar();

    // Assert
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
    EXPECT_FALSE(box->isChecked());
}

// ---------- CheckBoxWithSemanticIndex ----------

TEST_F(UT_IndexWidgetsCov, SemanticCheckBox_Construction_ReadsFileIndexState)
{
    // Arrange
    const IndexStatusCheckBox::Status initialStatus = IndexStatusCheckBox::Status::Inactive;

    // Act
    auto *box = track(new CheckBoxWithSemanticIndex());

    // Assert
    EXPECT_EQ(box->status(), initialStatus);
    EXPECT_FALSE(box->isChecked());
}

TEST_F(UT_IndexWidgetsCov, SemanticCheckBox_SemanticSearchChanged_SetsChecked)
{
    // Arrange
    auto *box = track(new CheckBoxWithSemanticIndex());

    // Act
    emit SearchManager::instance()->enableSemanticSearchChanged(true);
    const bool afterEnable = box->isChecked();
    emit SearchManager::instance()->enableSemanticSearchChanged(false);

    // Assert
    EXPECT_TRUE(afterEnable);
    EXPECT_FALSE(box->isChecked());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_IndexWidgetsCov, SemanticCheckBox_InitStatusBar_StaysInactive)
{
    // Arrange
    auto *box = track(new CheckBoxWithSemanticIndex());

    // Act
    box->initStatusBar();

    // Assert
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
    EXPECT_FALSE(box->isChecked());
}

TEST_F(UT_IndexWidgetsCov, SemanticCheckBox_SetDisabledByFileIndex_TogglesEnabled)
{
    // Arrange
    auto *box = track(new CheckBoxWithSemanticIndex());
    box->setChecked(true);

    // Act
    box->setDisabledByFileIndex(false);
    const bool disabled = box->isEnabled();
    const bool stillChecked = box->isChecked();
    box->setDisabledByFileIndex(true);

    // Assert
    EXPECT_FALSE(disabled);
    EXPECT_TRUE(stillChecked);   // check state is preserved while disabling
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

// ---------- IndexStatusCheckBox base behaviour ----------

TEST_F(UT_IndexWidgetsCov, StatusBox_SetDisplayText_UpdatesCheckBoxText)
{
    // Arrange
    auto *box = track(new IndexStatusCheckBox());
    auto *checkBox = box->findChild<QCheckBox *>(QStringLiteral("CheckBox"));
    ASSERT_NE(checkBox, nullptr);

    // Act
    box->setDisplayText(QStringLiteral("hello caption"));

    // Assert
    EXPECT_EQ(checkBox->text(), QString("hello caption"));
    EXPECT_NE(checkBox->text(), QString());
}

TEST_F(UT_IndexWidgetsCov, StatusBox_LinkActivated_ForwardsResetRequested)
{
    // Arrange
    auto *box = track(new IndexStatusCheckBox());
    QLabel *msgLabel = messageLabelOf(box);
    ASSERT_NE(msgLabel, nullptr);
    QSignalSpy spy(box, &IndexStatusCheckBox::resetRequested);

    // Act
    emit msgLabel->linkActivated(QStringLiteral("manual"));

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), QString("manual"));
}

TEST_F(UT_IndexWidgetsCov, StatusBox_Clicked_AcceptsChangeAndEmits)
{
    // Arrange
    auto *box = track(new IndexStatusCheckBox());
    auto *checkBox = box->findChild<QCheckBox *>(QStringLiteral("CheckBox"));
    ASSERT_NE(checkBox, nullptr);
    QSignalSpy spy(box, &IndexStatusCheckBox::checkStateChanged);

    // Act
    emit checkBox->clicked(true);   // base acceptCheckStateChange() returns true

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).value<Qt::CheckState>(), Qt::CheckState::Checked);
}
