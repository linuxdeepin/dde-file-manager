// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSignalSpy>

#include <dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h>

#include <DButtonBox>
#include <DPasswordEdit>
#include <QJsonObject>

// Include stub headers
#include "stubext.h"

using namespace DTK_WIDGET_NAMESPACE;

using namespace dfmbase;

class TestMountAskPasswordDialog : public testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
        stub.clear();
        capturedTextIndex = 0;
        
        // Stub UI methods to avoid actual dialog display
        stub.set_lamda(VADDR(QDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;  // or QDialog::Rejected as needed
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override {
        // Cleanup code after each test
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    int capturedTextIndex = 0;
};

// Test constructor
TEST_F(TestMountAskPasswordDialog, TestConstructor) {
    QWidget parent;
    MountAskPasswordDialog dialog(&parent);
    
    EXPECT_EQ(dialog.parent(), &parent);
    EXPECT_TRUE(dialog.isModal());
    EXPECT_NE(dialog.anonymousButton, nullptr);
    EXPECT_NE(dialog.registerButton, nullptr);
    EXPECT_NE(dialog.usernameLineEdit, nullptr);
    EXPECT_NE(dialog.domainLineEdit, nullptr);
    EXPECT_NE(dialog.passwordLineEdit, nullptr);
    EXPECT_NE(dialog.passwordCheckBox, nullptr);
}

// Test constructor with nullptr parent
TEST_F(TestMountAskPasswordDialog, TestConstructorWithNullParent) {
    MountAskPasswordDialog dialog(nullptr);
    
    EXPECT_EQ(dialog.parent(), nullptr);
    EXPECT_TRUE(dialog.isModal());
    EXPECT_NE(dialog.anonymousButton, nullptr);
    EXPECT_NE(dialog.registerButton, nullptr);
}

// Test getDomainLineVisible
TEST_F(TestMountAskPasswordDialog, TestGetDomainLineVisible) {
    MountAskPasswordDialog dialog;
    
    bool visible = dialog.getDomainLineVisible();
    EXPECT_EQ(visible, true);  // Default value
}

// Test setDomainLineVisible with true
TEST_F(TestMountAskPasswordDialog, TestSetDomainLineVisibleTrue) {
    MountAskPasswordDialog dialog;
    
    // Mock label and line edit visibility
    bool domainLabelShown = false;
    bool domainLineEditShown = false;
    
    stub.set_lamda(ADDR(QLabel, show), [&]() {
        domainLabelShown = true;
    });
    
    stub.set_lamda(ADDR(QLabel, hide), [&]() {
        domainLabelShown = false;
    });
    
    stub.set_lamda(ADDR(QLineEdit, show), [&]() {
        domainLineEditShown = true;
    });
    
    stub.set_lamda(ADDR(QLineEdit, hide), [&]() {
        domainLineEditShown = false;
    });
    
    dialog.setDomainLineVisible(true);
    
    EXPECT_TRUE(dialog.getDomainLineVisible());
}

// Test setDomainLineVisible with false
TEST_F(TestMountAskPasswordDialog, TestSetDomainLineVisibleFalse) {
    MountAskPasswordDialog dialog;
    
    // Mock label and line edit visibility
    bool domainLabelShown = true;
    bool domainLineEditShown = true;
    
    stub.set_lamda(ADDR(QLabel, show), [&]() {
        domainLabelShown = true;
    });
    
    stub.set_lamda(ADDR(QLabel, hide), [&]() {
        domainLabelShown = false;
    });
    
    stub.set_lamda(ADDR(QLineEdit, show), [&]() {
        domainLineEditShown = true;
    });
    
    stub.set_lamda(ADDR(QLineEdit, hide), [&]() {
        domainLineEditShown = false;
    });
    
    dialog.setDomainLineVisible(false);
    
    EXPECT_FALSE(dialog.getDomainLineVisible());
}

// Test setDomain
TEST_F(TestMountAskPasswordDialog, TestSetDomain) {
    MountAskPasswordDialog dialog;
    QString testDomain = "TESTDOMAIN";
    
    // Mock QLineEdit::setText
    QString capturedText;
    stub.set_lamda(ADDR(QLineEdit, setText), [&](QLineEdit*, const QString &text) {
        capturedText = text;
    });
    
    dialog.setDomain(testDomain);
    
    EXPECT_EQ(capturedText, testDomain);
}

// Test setDomain with empty string
TEST_F(TestMountAskPasswordDialog, TestSetDomainEmpty) {
    MountAskPasswordDialog dialog;
    QString emptyDomain = "";
    
    // Mock QLineEdit::setText
    QString capturedText;
    stub.set_lamda(ADDR(QLineEdit, setText), [&](QLineEdit*, const QString &text) {
        capturedText = text;
    });
    
    dialog.setDomain(emptyDomain);
    
    EXPECT_EQ(capturedText, emptyDomain);
}

// Test setUser
TEST_F(TestMountAskPasswordDialog, TestSetUser) {
    MountAskPasswordDialog dialog;
    QString testUser = "testuser";
    
    // Mock QLineEdit::setText
    QString capturedText;
    stub.set_lamda(ADDR(QLineEdit, setText), [&](QLineEdit*, const QString &text) {
        capturedText = text;
    });
    
    dialog.setUser(testUser);
    
    EXPECT_EQ(capturedText, testUser);
}

// Test setUser with empty string
TEST_F(TestMountAskPasswordDialog, TestSetUserEmpty) {
    MountAskPasswordDialog dialog;
    QString emptyUser = "";
    
    // Mock QLineEdit::setText
    QString capturedText;
    stub.set_lamda(ADDR(QLineEdit, setText), [&](QLineEdit*, const QString &text) {
        capturedText = text;
    });
    
    dialog.setUser(emptyUser);
    
    EXPECT_EQ(capturedText, emptyUser);
}

// Test getLoginData with anonymous selected
TEST_F(TestMountAskPasswordDialog, TestGetLoginDataAnonymous) {
    MountAskPasswordDialog dialog;
    
    // Mock button states
    stub.set_lamda(ADDR(DButtonBoxButton, isChecked), []() {
        return true;  // Anonymous selected
    });
    
    // Mock text retrievals
    stub.set_lamda(ADDR(QLineEdit, text), []() {
        return QString();
    });
    
    // Mock title
    stub.set_lamda(ADDR(MountAskPasswordDialog, title), []() {
        return QString("Test Title");
    });
    
    // Mock checkbox state
    stub.set_lamda(ADDR(QCheckBox, isChecked), []() {
        return false;
    });
    
    QJsonObject loginData = dialog.getLoginData();
    
    EXPECT_TRUE(loginData.contains("anonymous"));
    EXPECT_EQ(loginData.value("anonymous").toBool(), true);
    EXPECT_TRUE(loginData.contains("passwd_save_mode"));
    EXPECT_EQ(loginData.value("passwd_save_mode").toInt(), 0);  // kNeverSave
}

// Test getLoginData with registered user
TEST_F(TestMountAskPasswordDialog, TestGetLoginDataRegistered) {
    MountAskPasswordDialog dialog;
    
    // Mock button states
    stub.set_lamda(ADDR(DButtonBoxButton, isChecked), [](QAbstractButton* button) {
        // Anonymous button should return false, registered button true
        return button->objectName() == "RegisterButton";
    });
    
    // Mock text retrievals
    QString testUser = "testuser";
    QString testDomain = "testdomain";
    QString testPassword = "testpass";
    
    stub.set_lamda(ADDR(QLineEdit, text), [&](QLineEdit*) {
        if (capturedTextIndex == 0) return testUser;
        if (capturedTextIndex == 1) return testDomain;
        if (capturedTextIndex == 2) return testPassword;
        return QString();
    });
    
    // Mock title
    stub.set_lamda(ADDR(MountAskPasswordDialog, title), []() {
        return QString("Test Title");
    });
    
    // Mock checkbox state - checked for permanent save
    stub.set_lamda(ADDR(QCheckBox, isChecked), []() {
        return true;
    });
    
    QJsonObject loginData = dialog.getLoginData();
    
    EXPECT_TRUE(loginData.contains("anonymous"));
    EXPECT_EQ(loginData.value("anonymous").toBool(), false);
    EXPECT_TRUE(loginData.contains("user"));
    EXPECT_TRUE(loginData.contains("domain"));
    EXPECT_TRUE(loginData.contains("passwd"));
    EXPECT_EQ(loginData.value("passwd_save_mode").toInt(), 2);  // kSavePermanently
}

// Test handleButtonClicked with Connect button
TEST_F(TestMountAskPasswordDialog, TestHandleButtonClickedConnect) {
    MountAskPasswordDialog dialog;
    
    // Mock accept
    bool acceptCalled = false;
    stub.set_lamda(VADDR(MountAskPasswordDialog, accept), [&]() {
        acceptCalled = true;
    });
    
    // Mock getLoginData
    stub.set_lamda(ADDR(MountAskPasswordDialog, getLoginData), [&]() {
        return QJsonObject();
    });
    
    dialog.handleButtonClicked(1, "Connect");
    
    EXPECT_TRUE(acceptCalled);
}

// Test handleButtonClicked with Cancel button
TEST_F(TestMountAskPasswordDialog, TestHandleButtonClickedCancel) {
    MountAskPasswordDialog dialog;
    
    // Mock accept should not be called
    bool acceptCalled = false;
    stub.set_lamda(VADDR(MountAskPasswordDialog, accept), [&]() {
        acceptCalled = true;
    });
    
    dialog.handleButtonClicked(0, "Cancel");
    
    EXPECT_FALSE(acceptCalled);
}

// Test handleConnect with anonymous login
TEST_F(TestMountAskPasswordDialog, TestHandleConnectAnonymous) {
    MountAskPasswordDialog dialog;
    
    // Mock anonymous button checked
    stub.set_lamda(ADDR(DButtonBoxButton, isChecked), []() {
        return true;  // Anonymous selected
    });
    
    // Mock text retrievals
    stub.set_lamda(ADDR(QLineEdit, text), []() {
        return QString();
    });
    
    // Mock title
    stub.set_lamda(ADDR(MountAskPasswordDialog, title), []() {
        return QString("Test Title");
    });
    
    // Mock checkbox state
    stub.set_lamda(ADDR(QCheckBox, isChecked), []() {
        return false;
    });
    
    // Mock accept
    bool acceptCalled = false;
    stub.set_lamda(VADDR(MountAskPasswordDialog, accept), [&]() {
        acceptCalled = true;
    });
    
    dialog.handleConnect();
    
    EXPECT_TRUE(acceptCalled);
    
    // Check login data structure
    QJsonObject loginData = dialog.getLoginData();
    EXPECT_EQ(loginData.value("anonymous").toBool(), true);
}

// Test anonymous button click hides password frame
TEST_F(TestMountAskPasswordDialog, TestAnonymousButtonClick) {
    MountAskPasswordDialog dialog;
    
    // Mock password frame visibility
    bool frameVisible = true;
    stub.set_lamda(VADDR(QWidget, setVisible), [&](QWidget*, bool visible) {
        frameVisible = visible;
    });
    
    // Emit anonymous button clicked signal
    emit dialog.anonymousButton->clicked();
    
    EXPECT_FALSE(frameVisible);
}

// Test registered button click shows password frame
TEST_F(TestMountAskPasswordDialog, TestRegisteredButtonClick) {
    MountAskPasswordDialog dialog;
    
    // Mock password frame visibility
    bool frameVisible = false;
    stub.set_lamda(VADDR(QWidget, setVisible), [&](QWidget*, bool visible) {
        frameVisible = visible;
    });
    
    // Emit registered button clicked signal
    emit dialog.registerButton->clicked();
    
    EXPECT_TRUE(frameVisible);
}

// Test password checkbox checked state
TEST_F(TestMountAskPasswordDialog, TestPasswordSaveMode) {
    MountAskPasswordDialog dialog;
    
    // Mock registered user button
    stub.set_lamda(ADDR(DButtonBoxButton, isChecked), []() {
        return false;  // Not anonymous
    });
    
    // Mock text retrievals
    stub.set_lamda(ADDR(QLineEdit, text), []() {
        return QString("test");
    });
    
    // Mock title
    stub.set_lamda(ADDR(MountAskPasswordDialog, title), []() {
        return QString("Test Title");
    });
    
    // Mock checkbox state - checked
    stub.set_lamda(ADDR(QCheckBox, isChecked), []() {
        return true;
    });
    
    // Mock accept
    stub.set_lamda(VADDR(MountAskPasswordDialog, accept), []() {});
    
    dialog.handleConnect();
    
    QJsonObject loginData = dialog.getLoginData();
    EXPECT_EQ(loginData.value("passwd_save_mode").toInt(), 2);  // kSavePermanently
}

// Test password checkbox unchecked state
TEST_F(TestMountAskPasswordDialog, TestPasswordSaveModeNotChecked) {
    MountAskPasswordDialog dialog;
    
    // Mock registered user button
    stub.set_lamda(ADDR(DButtonBoxButton, isChecked), []() {
        return false;  // Not anonymous
    });
    
    // Mock text retrievals
    stub.set_lamda(ADDR(QLineEdit, text), []() {
        return QString("test");
    });
    
    // Mock title
    stub.set_lamda(ADDR(MountAskPasswordDialog, title), []() {
        return QString("Test Title");
    });
    
    // Mock checkbox state - not checked
    stub.set_lamda(ADDR(QCheckBox, isChecked), []() {
        return false;
    });
    
    // Mock accept
    stub.set_lamda(VADDR(MountAskPasswordDialog, accept), []() {});
    
    dialog.handleConnect();
    
    QJsonObject loginData = dialog.getLoginData();
    EXPECT_EQ(loginData.value("passwd_save_mode").toInt(), 0);  // kNeverSave
}

// Test initialization sets correct default values
TEST_F(TestMountAskPasswordDialog, TestInitializationDefaults) {
    MountAskPasswordDialog dialog;
    
    // Verify default domain line is visible
    EXPECT_TRUE(dialog.getDomainLineVisible());
    
    // Verify registered button is checked by default
    // This is set in initUI via registerButton->click()
    // We can verify the loginObj is initialized
    QJsonObject loginData = dialog.getLoginData();
    EXPECT_TRUE(loginData.isEmpty() || loginData.contains("message"));
}