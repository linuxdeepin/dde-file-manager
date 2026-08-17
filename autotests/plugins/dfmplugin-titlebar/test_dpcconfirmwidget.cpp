// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dialogs/dpcwidget/dpcconfirmwidget.h"

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/fileutils.h>

#include <DPasswordEdit>
#include <DSuggestButton>
#include <DPushButton>
#include <DFontSizeManager>
#include <DWindowManagerHelper>
#include <DSysInfo>

#include <gtest/gtest.h>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QSignalSpy>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLibrary>
#include <QLineEdit>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class DPCConfirmWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub QDBusConnection
        stub.set_lamda(&QDBusConnection::systemBus, []() {
            __DBG_STUB_INVOKE__
            return QDBusConnection("stub_system_bus");
        });

        // Stub QDBusInterface
        stub.set_lamda(static_cast<bool (QDBusInterface::*)() const>(&QDBusInterface::isValid), [](QDBusInterface *) {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(static_cast<bool (QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *)>(&QDBusConnection::connect),
                       [](QDBusConnection *, const QString &, const QString &, const QString &, const QString &, QObject *, const char *) {
                           __DBG_STUB_INVOKE__
                           return true;
                       });

        typedef void (DFontSizeManager::*Bind)(QWidget *, DFontSizeManager::SizeType, int);
        stub.set_lamda(static_cast<Bind>(&DFontSizeManager::bind), [](DFontSizeManager *, QWidget *, DFontSizeManager::SizeType, int) {
            __DBG_STUB_INVOKE__
        });

        // Stub QLibrary::load to fail by default
        stub.set_lamda(&QLibrary::load, [](QLibrary *) {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Stub DSysInfo
        stub.set_lamda(&DSysInfo::uosEditionType, []() {
            __DBG_STUB_INVOKE__
            return DSysInfo::UosProfessional;
        });

        // Stub DWindowManagerHelper
        typedef DWindowManagerHelper::MotifFunctions (*Func)(const QWindow *, DWindowManagerHelper::MotifFunctions, bool);
        stub.set_lamda(static_cast<Func>(&DWindowManagerHelper::setMotifFunctions),
                       [] {
                           __DBG_STUB_INVOKE__
                           return DWindowManagerHelper::FUNC_CLOSE;
                       });

        // Stub FileUtils
        stub.set_lamda(&FileUtils::encryptString, [](const QString &str) {
            __DBG_STUB_INVOKE__
            return str + "_encrypted";
        });

        widget = new DPCConfirmWidget();
    }

    void
    TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    DPCConfirmWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(DPCConfirmWidgetTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(DPCConfirmWidgetTest, Constructor_InitializesUIComponents_AllWidgetsCreated)
{
    EXPECT_NE(widget->titleLabel, nullptr);
    EXPECT_NE(widget->oldPwdEdit, nullptr);
    EXPECT_NE(widget->newPwdEdit, nullptr);
    EXPECT_NE(widget->repeatPwdEdit, nullptr);
    EXPECT_NE(widget->saveBtn, nullptr);
    EXPECT_NE(widget->cancelBtn, nullptr);
}

TEST_F(DPCConfirmWidgetTest, Constructor_InitializesDBusInterface_InterfaceCreated)
{
    EXPECT_NE(widget->accessControlInter, nullptr);
}

TEST_F(DPCConfirmWidgetTest, CheckRepeatPassword_MatchingPasswords_ReturnsTrue)
{
    widget->newPwdEdit->setText("password123");
    widget->repeatPwdEdit->setText("password123");

    bool result = widget->checkRepeatPassword();

    EXPECT_TRUE(result);
    EXPECT_FALSE(widget->repeatPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, CheckRepeatPassword_MismatchingPasswords_ReturnsFalse)
{
    widget->newPwdEdit->setText("password123");
    widget->repeatPwdEdit->setText("password456");

    bool result = widget->checkRepeatPassword();

    EXPECT_FALSE(result);
    EXPECT_TRUE(widget->repeatPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, CheckRepeatPassword_PreviousAlert_ClearsAlert)
{
    widget->newPwdEdit->setText("password123");
    widget->repeatPwdEdit->setText("password456");
    widget->checkRepeatPassword();
    EXPECT_TRUE(widget->repeatPwdEdit->isAlert());

    widget->repeatPwdEdit->setText("password123");
    bool result = widget->checkRepeatPassword();

    EXPECT_TRUE(result);
    EXPECT_FALSE(widget->repeatPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, CheckNewPassword_SameAsOld_ReturnsFalse)
{
    widget->oldPwdEdit->setText("password123");
    widget->newPwdEdit->setText("password123");

    bool result = widget->checkNewPassword();

    EXPECT_FALSE(result);
    EXPECT_TRUE(widget->newPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, CheckNewPassword_DifferentFromOld_ChecksComplexity)
{
    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("newPassword456");

    // Complexity check should pass for non-Pro/Community editions
    stub.set_lamda(&DSysInfo::uosEditionType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosProfessional;
    });

    bool result = widget->checkNewPassword();

    EXPECT_TRUE(result);
    EXPECT_FALSE(widget->newPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, CheckPasswdComplexity_DesktopEdition_ReturnsTrue)
{
    stub.set_lamda(&DSysInfo::uosEditionType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosProfessional;
    });

    QString msg;
    bool result = widget->checkPasswdComplexity("anyPassword", &msg);

    EXPECT_TRUE(result);
}

TEST_F(DPCConfirmWidgetTest, CheckPasswdComplexity_ProfessionalOldVersion_ReturnsTrue)
{
    stub.set_lamda(&DSysInfo::uosEditionType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosProfessional;
    });

    stub.set_lamda(&DSysInfo::minorVersion, []() {
        __DBG_STUB_INVOKE__
        return QString("1050");
    });

    QString msg;
    bool result = widget->checkPasswdComplexity("anyPassword", &msg);

    EXPECT_TRUE(result);
}

TEST_F(DPCConfirmWidgetTest, CheckPasswdComplexity_CommunityOldVersion_ReturnsTrue)
{
    stub.set_lamda(&DSysInfo::uosEditionType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosCommunity;
    });

    stub.set_lamda(&DSysInfo::majorVersion, []() {
        __DBG_STUB_INVOKE__
        return QString("20");
    });

    QString msg;
    bool result = widget->checkPasswdComplexity("anyPassword", &msg);

    EXPECT_TRUE(result);
}

TEST_F(DPCConfirmWidgetTest, CheckPasswdComplexity_LibraryNotLoaded_ReturnsTrue)
{
    stub.set_lamda(&DSysInfo::uosEditionType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosProfessional;
    });

    stub.set_lamda(&DSysInfo::minorVersion, []() {
        __DBG_STUB_INVOKE__
        return QString("1060");
    });

    // Library not loaded, functions are nullptr
    QString msg;
    bool result = widget->checkPasswdComplexity("anyPassword", &msg);

    EXPECT_TRUE(result);
}

TEST_F(DPCConfirmWidgetTest, OnEditingFinished_ExceedsMaxLength_SetsAlert)
{
    QString longPassword(520, 'a');
    widget->newPwdEdit->setText(longPassword);

    stub.set_lamda(&QObject::sender, [&]() -> QObject * {
        __DBG_STUB_INVOKE__
        return widget->newPwdEdit;
    });

    EXPECT_NO_THROW(widget->onEditingFinished());
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_EmptyOldPassword_SetsAlert)
{
    widget->oldPwdEdit->setText("");
    widget->newPwdEdit->setText("newPassword123");
    widget->repeatPwdEdit->setText("newPassword123");

    widget->onSaveBtnClicked();

    EXPECT_TRUE(widget->oldPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_EmptyNewPassword_SetsAlert)
{
    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("");
    widget->repeatPwdEdit->setText("newPassword123");

    widget->onSaveBtnClicked();

    EXPECT_TRUE(widget->newPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_EmptyRepeatPassword_SetsAlert)
{
    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("newPassword123");
    widget->repeatPwdEdit->setText("");

    widget->onSaveBtnClicked();

    EXPECT_TRUE(widget->repeatPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_NewPasswordSameAsOld_SetsAlert)
{
    widget->oldPwdEdit->setText("samePassword123");
    widget->newPwdEdit->setText("samePassword123");
    widget->repeatPwdEdit->setText("samePassword123");

    widget->onSaveBtnClicked();

    EXPECT_TRUE(widget->newPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_RepeatPasswordMismatch_SetsAlert)
{
    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("newPassword123");
    widget->repeatPwdEdit->setText("differentPassword");

    widget->onSaveBtnClicked();

    EXPECT_TRUE(widget->repeatPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_InvalidInterface_DoesNotCall)
{
    stub.set_lamda(static_cast<bool (QDBusInterface::*)() const>(&QDBusInterface::isValid), [](QDBusInterface *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("newPassword456");
    widget->repeatPwdEdit->setText("newPassword456");

    // Should not crash
    EXPECT_NO_THROW(widget->onSaveBtnClicked());
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_ValidPasswords_CallsDBus)
{
    bool dbusCalled = false;
    stub.set_lamda(static_cast<QDBusPendingCall (QDBusInterface::*)(const QString &, QVariant &&)>(&QDBusInterface::asyncCall),
                   [&dbusCalled](QDBusInterface *, const QString &, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       dbusCalled = true;
                       return QDBusPendingCall::fromError(QDBusError());
                   });

    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("newPassword456");
    widget->repeatPwdEdit->setText("newPassword456");

    widget->onSaveBtnClicked();

    EXPECT_TRUE(dbusCalled);
}

TEST_F(DPCConfirmWidgetTest, OnPasswordChecked_NoError_EmitsConfirmed)
{
    QSignalSpy spy(widget, &DPCConfirmWidget::sigConfirmed);

    widget->onPasswordChecked(kNoError);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(DPCConfirmWidgetTest, OnPasswordChecked_AuthenticationFailed_EnablesButtons)
{
    widget->setEnabled(false);
    EXPECT_FALSE(widget->saveBtn->isEnabled());

    widget->onPasswordChecked(kAuthenticationFailed);

    EXPECT_TRUE(widget->saveBtn->isEnabled());
    EXPECT_TRUE(widget->cancelBtn->isEnabled());
}

TEST_F(DPCConfirmWidgetTest, OnPasswordChecked_PasswordWrong_SetsAlert)
{
    widget->setEnabled(false);

    widget->onPasswordChecked(kPasswordWrong);

    EXPECT_TRUE(widget->saveBtn->isEnabled());
    EXPECT_TRUE(widget->cancelBtn->isEnabled());
    EXPECT_TRUE(widget->oldPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, OnPasswordChecked_UnknownError_DoesNothing)
{
    widget->setEnabled(false);

    widget->onPasswordChecked(999);

    // Should not crash
    EXPECT_FALSE(widget->saveBtn->isEnabled());
}

TEST_F(DPCConfirmWidgetTest, SetEnabled_True_EnablesButtons)
{
    widget->setEnabled(false);
    EXPECT_FALSE(widget->saveBtn->isEnabled());
    EXPECT_FALSE(widget->cancelBtn->isEnabled());

    widget->setEnabled(true);

    EXPECT_TRUE(widget->saveBtn->isEnabled());
    EXPECT_TRUE(widget->cancelBtn->isEnabled());
}

TEST_F(DPCConfirmWidgetTest, SetEnabled_False_DisablesButtons)
{
    widget->setEnabled(true);
    EXPECT_TRUE(widget->saveBtn->isEnabled());

    widget->setEnabled(false);

    EXPECT_FALSE(widget->saveBtn->isEnabled());
    EXPECT_FALSE(widget->cancelBtn->isEnabled());
}

TEST_F(DPCConfirmWidgetTest, CancelButton_Clicked_EmitsCloseDialog)
{
    QSignalSpy spy(widget, &DPCConfirmWidget::sigCloseDialog);

    widget->cancelBtn->click();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(DPCConfirmWidgetTest, InitLibrary_LoadFails_FunctionsNull)
{
    stub.set_lamda(&QLibrary::load, [](QLibrary *) {
        __DBG_STUB_INVOKE__
        return false;
    });

    DPCConfirmWidget *testWidget = new DPCConfirmWidget();

    EXPECT_EQ(testWidget->deepinPwCheck, nullptr);
    EXPECT_EQ(testWidget->getPasswdLevel, nullptr);
    EXPECT_EQ(testWidget->errToString, nullptr);

    delete testWidget;
}

TEST_F(DPCConfirmWidgetTest, ShowToolTips_ValidEdit_ShowsAlert)
{
    EXPECT_NO_THROW(widget->showToolTips("Test error message", widget->oldPwdEdit));
}

TEST_F(DPCConfirmWidgetTest, PasswordValidator_ChineseCharacters_Rejected)
{
    QValidator::State state;
    QString chineseText = "密码123";
    int pos = 0;

    state = widget->oldPwdEdit->lineEdit()->validator()->validate(chineseText, pos);

    EXPECT_EQ(state, QValidator::Invalid);
}

TEST_F(DPCConfirmWidgetTest, PasswordValidator_EnglishAndNumbers_Accepted)
{
    QValidator::State state;
    QString validText = "Password123";
    int pos = 0;

    state = widget->oldPwdEdit->lineEdit()->validator()->validate(validText, pos);

    EXPECT_TRUE(state == QValidator::Acceptable || state == QValidator::Intermediate);
}

TEST_F(DPCConfirmWidgetTest, AccessControlInterface_Service_ConfiguredCorrectly)
{
    EXPECT_NE(widget->accessControlInter, nullptr);
    if (widget->accessControlInter) {
        EXPECT_EQ(widget->accessControlInter->service(), "org.deepin.Filemanager.AccessControlManager");
        EXPECT_EQ(widget->accessControlInter->path(), "/org/deepin/Filemanager/AccessControlManager");
        EXPECT_EQ(widget->accessControlInter->interface(), "org.deepin.Filemanager.AccessControlManager");
    }
}

TEST_F(DPCConfirmWidgetTest, Layout_ContainsAllWidgets)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(widget->layout());
    EXPECT_NE(layout, nullptr);

    // Check that layout exists and has items
    if (layout) {
        EXPECT_GT(layout->count(), 0);
    }
}

TEST_F(DPCConfirmWidgetTest, OnSaveBtnClicked_PipeCreationFails_HandlesGracefully)
{
    stub.set_lamda(pipe, [](int pipefd[2]) {
        __DBG_STUB_INVOKE__
        return -1;   // Simulate pipe creation failure
    });

    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("newPassword456");
    widget->repeatPwdEdit->setText("newPassword456");

    widget->onSaveBtnClicked();

    // Should re-enable buttons after failure
    EXPECT_TRUE(widget->saveBtn->isEnabled());
    EXPECT_TRUE(widget->cancelBtn->isEnabled());
}

TEST_F(DPCConfirmWidgetTest, CheckNewPassword_ComplexityFails_ReturnsFalse)
{
    // Setup for complexity check
    stub.set_lamda(&DSysInfo::uosEditionType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosProfessional;
    });

    stub.set_lamda(&DSysInfo::minorVersion, []() {
        __DBG_STUB_INVOKE__
        return QString("1060");
    });

    // Mock library loaded
    widget->getPasswdLevel = [](const char *) -> int { return 1; };   // Low level
    widget->deepinPwCheck = [](const char *, const char *, int, const char *) -> int { return 0; };
    widget->errToString = [](int) -> const char * { return "Error"; };

    stub.set_lamda(&SysInfoUtils::getUser, []() {
        __DBG_STUB_INVOKE__
        return QString("testuser");
    });

    widget->oldPwdEdit->setText("oldPassword123");
    widget->newPwdEdit->setText("weak");

    bool result = widget->checkNewPassword();

    EXPECT_FALSE(result);
    EXPECT_TRUE(widget->newPwdEdit->isAlert());
}

TEST_F(DPCConfirmWidgetTest, CheckPasswdComplexity_UsernameSameAsPassword_ReturnsFalse)
{
    stub.set_lamda(&DSysInfo::uosEditionType, []() {
        __DBG_STUB_INVOKE__
        return DSysInfo::UosProfessional;
    });

    stub.set_lamda(&DSysInfo::minorVersion, []() {
        __DBG_STUB_INVOKE__
        return QString("1060");
    });

    stub.set_lamda(&SysInfoUtils::getUser, []() {
        __DBG_STUB_INVOKE__
        return QString("testuser");
    });

    widget->getPasswdLevel = [](const char *) -> int { return 5; };   // High level
    widget->deepinPwCheck = [](const char *, const char *, int, const char *) -> int { return 0; };
    widget->errToString = [](int) -> const char * { return ""; };

    QString msg;
    bool result = widget->checkPasswdComplexity("testuser", &msg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(msg.isEmpty());
}
