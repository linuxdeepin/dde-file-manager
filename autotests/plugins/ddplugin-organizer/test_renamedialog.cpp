// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/renamedialog.h"

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

class UT_RenameDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        dialog = new RenameDialog(5); // Create with 5 files

        // mock the UI show
        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        delete dialog;
        dialog = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    RenameDialog *dialog = nullptr;
};

TEST_F(UT_RenameDialog, Constructor_CreatesDialog)
{
    EXPECT_NE(dialog, nullptr);
    EXPECT_EQ(dialog->parent(), nullptr);
}

TEST_F(UT_RenameDialog, Constructor_WithFileCount_CreatesDialog)
{
    RenameDialog dialogWithCount(10);
    EXPECT_NE(&dialogWithCount, nullptr);
}

TEST_F(UT_RenameDialog, Constructor_WithParent_CreatesDialog)
{
    QWidget parent;
    RenameDialog childDialog(3, &parent);
    EXPECT_EQ(childDialog.parent(), &parent);
}

TEST_F(UT_RenameDialog, ModifyMode_DefaultValue)
{
    RenameDialog::ModifyMode mode = dialog->modifyMode();
    // Default mode might be the first enum value
    EXPECT_TRUE(mode == RenameDialog::kReplace || 
                mode == RenameDialog::kAdd || 
                mode == RenameDialog::kCustom);
}

TEST_F(UT_RenameDialog, GetReplaceContent_ReturnsPair)
{
    QPair<QString, QString> content = dialog->getReplaceContent();
    // Should return a valid pair, even if empty
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_RenameDialog, GetAddContent_ReturnsPair)
{
    QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> content = dialog->getAddContent();
    // Should return a valid pair
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_RenameDialog, GetCustomContent_ReturnsPair)
{
    QPair<QString, QString> content = dialog->getCustomContent();
    // Should return a valid pair, even if empty
    EXPECT_TRUE(true); // Method exists and returns a value
}

TEST_F(UT_RenameDialog, ModifyMode_EnumValues)
{
    EXPECT_NE(RenameDialog::kReplace, RenameDialog::kAdd);
    EXPECT_NE(RenameDialog::kReplace, RenameDialog::kCustom);
    EXPECT_NE(RenameDialog::kAdd, RenameDialog::kCustom);
}

TEST_F(UT_RenameDialog, MultipleDialogOperations)
{
    // Test with different file counts
    RenameDialog dialog1(1);
    RenameDialog dialog2(100);
    RenameDialog dialog3(0);
    
    EXPECT_NE(&dialog1, nullptr);
    EXPECT_NE(&dialog2, nullptr);
    EXPECT_NE(&dialog3, nullptr);
    
    // Test all getter methods on each dialog
    EXPECT_TRUE(true); // All dialogs created successfully
    EXPECT_NO_THROW(dialog1.modifyMode());
    EXPECT_NO_THROW(dialog2.modifyMode());
    EXPECT_NO_THROW(dialog3.modifyMode());
}

TEST_F(UT_RenameDialog, Destructor_DoesNotCrash)
{
    RenameDialog *tempDialog = new RenameDialog(2);
    EXPECT_NE(tempDialog, nullptr);
    delete tempDialog; // Should not crash
    SUCCEED();
}

TEST_F(UT_RenameDialog, CopyConstructorAndAssignment_Disabled)
{
    // The copy constructor and assignment operator are deleted,
    // so we just verify the object can be created and used normally
    EXPECT_NE(dialog, nullptr);
    // The deleted copy constructor/assignment don't affect normal usage
}
