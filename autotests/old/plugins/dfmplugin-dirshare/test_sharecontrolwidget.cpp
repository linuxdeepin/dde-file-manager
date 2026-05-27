// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "widget/sharecontrolwidget.h"
#include "utils/usersharehelper.h"
#include "dfmplugin_dirshare_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dialogs/smbsharepasswddialog/usersharepasswordsettingdialog.h>

#include <QLineEdit>
#include <QDir>
#include <QFileInfo>
#include <QCheckBox>
#include <QComboBox>
#include <QTextBrowser>

using namespace dfmplugin_dirshare;
DFMBASE_USE_NAMESPACE

class UT_ShareControlWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        widget = new ShareControlWidget(QUrl::fromLocalFile("/home"));
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete widget;
    }

private:
    stub_ext::StubExt stub;
    ShareControlWidget *widget { nullptr };
};

TEST_F(UT_ShareControlWidget, SetOption)
{
    EXPECT_NO_FATAL_FAILURE(ShareControlWidget::setOption(widget, {}));
}

TEST_F(UT_ShareControlWidget, ValidateShareName)
{
    widget->shareNameEditor->clear();
    EXPECT_FALSE(widget->validateShareName());

    stub.set_lamda(&DialogManager::showErrorDialog, [] { __DBG_STUB_INVOKE__ });
    widget->shareNameEditor->setText(".");
    EXPECT_FALSE(widget->validateShareName());
    widget->shareNameEditor->setText("..");
    EXPECT_FALSE(widget->validateShareName());

    widget->shareNameEditor->setText("Hello");
    bool isShared = true;
    stub.set_lamda(&UserShareHelper::isShared, [&] { __DBG_STUB_INVOKE__ return isShared; });
    QString name = "Hello";
    stub.set_lamda(&UserShareHelper::shareNameByPath, [&] { __DBG_STUB_INVOKE__ return name; });
    EXPECT_TRUE(widget->validateShareName());

    typedef QFileInfoList (QDir::*EntryInfoList)(QDir::Filters, QDir::SortFlags) const;
    auto entryInfoList = static_cast<EntryInfoList>(&QDir::entryInfoList);
    stub.set_lamda(entryInfoList, [] { __DBG_STUB_INVOKE__ return QList<QFileInfo> { QFileInfo() }; });
    stub.set_lamda(&QFileInfo::fileName, [] { __DBG_STUB_INVOKE__ return "hello"; });
    bool writable = false;
    stub.set_lamda(&QFileInfo::isWritable, [&] { __DBG_STUB_INVOKE__ return writable; });
    int execRet = QDialog::Rejected;
    stub.set_lamda(VADDR(QDialog, exec), [&] { __DBG_STUB_INVOKE__ return execRet; });
    widget->shareNameEditor->setText("Hello");
    isShared = false;
    EXPECT_FALSE(widget->validateShareName());

    execRet = QDialog::Accepted;
    writable = true;
    EXPECT_TRUE(widget->validateShareName());
}

TEST_F(UT_ShareControlWidget, UpdateShare)
{
    stub.set_lamda(&ShareControlWidget::shareFolder, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(widget->updateShare());
}

TEST_F(UT_ShareControlWidget, ShareFolder)
{
    QSignalBlocker b4(widget->shareSwitcher);
    QSignalBlocker b3(widget->sharePermissionSelector);
    QSignalBlocker b2(widget->shareAnonymousSelector);
    QSignalBlocker b1(widget->shareNameEditor);

    widget->shareSwitcher->setChecked(false);
    EXPECT_NO_FATAL_FAILURE(widget->shareFolder());

    bool validName = false;
    stub.set_lamda(&ShareControlWidget::validateShareName, [&] { __DBG_STUB_INVOKE__ return validName; });
    widget->shareSwitcher->setChecked(true);
    EXPECT_NO_FATAL_FAILURE(widget->shareFolder());
    EXPECT_EQ(false, widget->shareSwitcher->isChecked());
    EXPECT_EQ(false, widget->sharePermissionSelector->isEnabled());
    EXPECT_EQ(false, widget->shareAnonymousSelector->isEnabled());

    validName = true;
    // disable anonymous
    widget->shareAnonymousSelector->setCurrentIndex(0);
    stub.set_lamda(&UserShareHelper::share, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_NO_FATAL_FAILURE(widget->shareFolder());
    EXPECT_EQ(false, widget->shareSwitcher->isChecked());
    EXPECT_EQ(false, widget->sharePermissionSelector->isEnabled());
    EXPECT_EQ(false, widget->shareAnonymousSelector->isEnabled());

    // enable anonymous
    widget->shareAnonymousSelector->setCurrentIndex(1);
}

TEST_F(UT_ShareControlWidget, UnshareFolder)
{
    stub.set_lamda(&UserShareHelper::removeShareByPath, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(widget->unshareFolder());
}

TEST_F(UT_ShareControlWidget, UpdateWidgetStatus)
{
    QSignalBlocker b1(widget->shareSwitcher);
    QSignalBlocker b2(widget->sharePermissionSelector);
    QSignalBlocker b3(widget->shareAnonymousSelector);
    QSignalBlocker b4(widget->shareNameEditor);

    widget->url = QUrl::fromLocalFile("/home");
    EXPECT_NO_FATAL_FAILURE(widget->updateWidgetStatus("/"));

    QVariantMap share;
    stub.set_lamda(&UserShareHelper::shareInfoByPath, [&] { __DBG_STUB_INVOKE__ return share; });
    EXPECT_NO_FATAL_FAILURE(widget->updateWidgetStatus("/home"));
    EXPECT_EQ(false, widget->shareSwitcher->isChecked());
    EXPECT_EQ(false, widget->sharePermissionSelector->isEnabled());
    EXPECT_EQ(false, widget->shareAnonymousSelector->isEnabled());

    share.insert(ShareInfoKeys::kName, "hello");
    share.insert(ShareInfoKeys::kPath, "/home");
    stub.set_lamda(&UserShareHelper::whoShared, [] { __DBG_STUB_INVOKE__ return 1000; });
    EXPECT_NO_FATAL_FAILURE(widget->updateWidgetStatus("/home"));
    //    EXPECT_EQ(true, widget->sharePermissionSelector->isEnabled());
    //    EXPECT_EQ(true, widget->shareAnonymousSelector->isEnabled());
}

TEST_F(UT_ShareControlWidget, OnSambapasswordSet)
{
    QSignalBlocker b1(widget->shareSwitcher);
    QSignalBlocker b2(widget->sharePermissionSelector);
    QSignalBlocker b3(widget->shareAnonymousSelector);
    QSignalBlocker b4(widget->shareNameEditor);

    EXPECT_NO_FATAL_FAILURE(widget->onSambaPasswordSet(true));
    EXPECT_TRUE(widget->isSharePasswordSet);
}

TEST_F(UT_ShareControlWidget, ShowMoreInfo)
{
    EXPECT_NO_FATAL_FAILURE(widget->showMoreInfo(true));
    EXPECT_FALSE(widget->m_shareNotes->isHidden());
    EXPECT_TRUE(widget->refreshIp);
    EXPECT_TRUE(widget->refreshIp->isActive());
    EXPECT_NO_FATAL_FAILURE(widget->showMoreInfo(false));
    EXPECT_TRUE(widget->m_shareNotes->isHidden());
    EXPECT_FALSE(widget->refreshIp->isActive());
}

TEST_F(UT_ShareControlWidget, UserShareOperation)
{
    QSignalBlocker b1(widget->shareSwitcher);
    QSignalBlocker b2(widget->sharePermissionSelector);
    QSignalBlocker b3(widget->shareAnonymousSelector);
    QSignalBlocker b4(widget->shareNameEditor);

    stub.set_lamda(&ShareControlWidget::showSharePasswordSettingsDialog, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ShareControlWidget::shareFolder, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&ShareControlWidget::unshareFolder, [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&ShareControlWidget::showMoreInfo, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(widget->userShareOperation(false));
    EXPECT_NO_FATAL_FAILURE(widget->userShareOperation(true));
}

TEST_F(UT_ShareControlWidget, ShowSharePasswordSettingsDialog)
{
    widget->setProperty("UserSharePwdSettingDialogShown", true);
    EXPECT_NO_FATAL_FAILURE(widget->showSharePasswordSettingsDialog());

    widget->setProperty("UserSharePwdSettingDialogShown", false);
    stub.set_lamda(VADDR(QDialog, show), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&UserSharePasswordSettingDialog::onButtonClicked, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&UserShareHelper::currentUserName, [] { __DBG_STUB_INVOKE__ return "test"; });
    stub.set_lamda(&UserShareHelper::setSambaPasswd, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(widget->showSharePasswordSettingsDialog());
}
