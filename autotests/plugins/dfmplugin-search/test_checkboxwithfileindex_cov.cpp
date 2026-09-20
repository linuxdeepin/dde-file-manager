// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/utils/checkboxwithfileindex.cpp
// (uncovered functions -> test case)
//   CheckBoxWithFileIndex(QWidget*) .................. Constructor_CreatesController /
//                                                      EnableFileIndexSearchChanged_SyncsCheckedState
//   ctor {lambda(bool)} (enableFileIndexSearchChanged)  EnableFileIndexSearchChanged_SyncsCheckedState
//   connectToBackend ................................... ConnectToBackend_DelegatesToController
//   initStatusBar ...................................... InitStatusBar_DelegatesToController
//   acceptCheckStateChange ............................ AcceptCheckStateChange_* (3 cases)
//   confirmDisableFileIndex ............................ ConfirmDisableFileIndex_DialogAccepted_ReturnsTrue
//
// Behaviour moved out with the old systemctl implementation (queryState /
// applyState / restartFileIndex / CommandResult / statusFilePath /
// formatDisplayTime / ...) is now covered by test_indexstatuscontroller_cov.cpp
// (IndexStatusController) and test_filenameindexclient.cpp (FileNameIndexClient).
// Private members are reached via -fno-access-control on the test target.
// ============================================================================

#include <gtest/gtest.h>
#include <QTimer>
#include <QApplication>

#include "stubext.h"

#include <DDialog>

#include "searchmanager/searchmanager.h"
#include "utils/indexstatuscheckbox.h"

#include "utils/checkboxwithfileindex.h"
#include "utils/indexstatuscontroller.h"

using namespace dfmplugin_search;

namespace {

// DDialog::exec() is virtual, so it cannot be patched with stubext
// (pointer-to-member yields a vtable offset). Instead, close the modal
// dialog from a timer that runs inside exec()'s nested event loop.
void autoFinishModalDialog(bool accept)
{
    QTimer::singleShot(300, [accept]() {
        QWidget *modal = qApp->activeModalWidget();
        if (auto *dialog = qobject_cast<Dtk::Widget::DDialog *>(modal)) {
            accept ? dialog->accept() : dialog->reject();
        } else if (modal) {
            modal->close();
        }
    });
}

}   // namespace

class UT_CheckBoxWithFileIndexCov : public testing::Test
{
protected:
    void TearDown() override
    {
        stub.clear();
        delete box;
        box = nullptr;
    }

    CheckBoxWithFileIndex *makeBox()
    {
        box = new CheckBoxWithFileIndex();
        return box;
    }

    stub_ext::StubExt stub;
    CheckBoxWithFileIndex *box = nullptr;
};

// ---------- construction / delegation ----------

TEST_F(UT_CheckBoxWithFileIndexCov, Constructor_CreatesController)
{
    // Arrange / Act
    makeBox();

    // Assert
    ASSERT_NE(box, nullptr);
    ASSERT_NE(box->m_controller, nullptr);
    EXPECT_FALSE(box->isChecked());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, EnableFileIndexSearchChanged_SyncsCheckedState)
{
    // Arrange
    makeBox();
    EXPECT_FALSE(box->isChecked());

    // Act
    emit SearchManager::instance()->enableFileIndexSearchChanged(true);
    const bool checkedAfterEnable = box->isChecked();
    emit SearchManager::instance()->enableFileIndexSearchChanged(false);

    // Assert
    EXPECT_TRUE(checkedAfterEnable);
    EXPECT_FALSE(box->isChecked());
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, ConnectToBackend_DelegatesToController)
{
    // Arrange
    makeBox();
    int backendCalls = 0;
    stub.set_lamda(&IndexStatusController::connectToBackend,
                   [&backendCalls](IndexStatusController *) {
                       ++backendCalls;
                   });

    // Act
    box->connectToBackend();

    // Assert
    EXPECT_EQ(backendCalls, 1);
}

TEST_F(UT_CheckBoxWithFileIndexCov, InitStatusBar_DelegatesToController)
{
    // Arrange
    makeBox();
    int initCalls = 0;
    stub.set_lamda(&IndexStatusController::initStatusBar,
                   [&initCalls](IndexStatusController *) {
                       ++initCalls;
                   });

    // Act
    box->initStatusBar();

    // Assert
    EXPECT_EQ(initCalls, 1);
}

// ---------- check state transitions ----------

TEST_F(UT_CheckBoxWithFileIndexCov, AcceptCheckStateChange_CheckedToUnchecked_AskConfirmDialog)
{
    // Arrange
    makeBox();
    autoFinishModalDialog(true);   // user presses "Confirm"

    // Act
    bool accepted = box->acceptCheckStateChange(Qt::CheckState::Checked, Qt::CheckState::Unchecked);

    // Assert
    EXPECT_TRUE(accepted);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);   // state untouched by the check itself
}

TEST_F(UT_CheckBoxWithFileIndexCov, AcceptCheckStateChange_DialogRejected_BlocksChange)
{
    // Arrange
    makeBox();
    autoFinishModalDialog(false);   // user presses "Cancel"

    // Act
    bool accepted = box->acceptCheckStateChange(Qt::CheckState::Checked, Qt::CheckState::Unchecked);

    // Assert
    EXPECT_FALSE(accepted);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}

TEST_F(UT_CheckBoxWithFileIndexCov, AcceptCheckStateChange_UncheckedToChecked_SkipsDialog)
{
    // Arrange
    makeBox();

    // Act
    bool allowed = box->acceptCheckStateChange(Qt::CheckState::Unchecked, Qt::CheckState::Checked);

    // Assert
    EXPECT_TRUE(allowed);   // no modal dialog: the test would have hung otherwise
}

TEST_F(UT_CheckBoxWithFileIndexCov, ConfirmDisableFileIndex_DialogAccepted_ReturnsTrue)
{
    // Arrange
    makeBox();
    autoFinishModalDialog(true);

    // Act
    bool confirmed = box->confirmDisableFileIndex();

    // Assert
    EXPECT_TRUE(confirmed);
    EXPECT_EQ(box->status(), IndexStatusCheckBox::Status::Inactive);
}
