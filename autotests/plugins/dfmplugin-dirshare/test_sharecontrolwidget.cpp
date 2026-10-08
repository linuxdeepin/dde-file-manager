// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
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

    // switcher unchecked: shareFolder() aborts early
    widget->shareSwitcher->setChecked(false);
    EXPECT_FALSE(widget->shareFolder());

    // invalid share name: aborted before sharing
    bool validName = false;
    stub.set_lamda(&ShareControlWidget::validateShareName, [&] { __DBG_STUB_INVOKE__ return validName; });
    widget->shareSwitcher->setChecked(true);
    EXPECT_FALSE(widget->shareFolder());

    validName = true;
    // UserShareHelper::share() fails
    // (anonymous selector stays disabled so the anonymous permission chmod
    // path in shareFolder() is never exercised in the test environment)
    stub.set_lamda(&UserShareHelper::share, [] { __DBG_STUB_INVOKE__ return false; });
    EXPECT_FALSE(widget->shareFolder());
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
    // showMoreInfo() only toggles moreInfoFrame visibility (and the refreshIp
    // timer when that has been created by the share init path); m_shareNotes
    // lives inside moreInfoFrame.
    EXPECT_NO_FATAL_FAILURE(widget->showMoreInfo(true));
    EXPECT_FALSE(widget->moreInfoFrame->isHidden());
    EXPECT_NO_FATAL_FAILURE(widget->showMoreInfo(false));
    EXPECT_TRUE(widget->moreInfoFrame->isHidden());
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

// PMS:303377 修改密码仅首次有响应：修复前对话框连接 finished→onButtonClicked，
// QDialogPrivate::close 安装的事件过滤器拦截 Close 事件导致 closed 不触发、
// UserSharePwdSettingDialogShown 常驻 true，再次点击修改密码按钮被属性拦截；
// 修复后改连 buttonClicked→onButtonClicked，且 closed 需复位属性保证可再次打开
TEST_F(UT_ShareControlWidget, BUG303377_ShowSharePasswordSettingsDialog_ButtonClickedWiredAndClosedResets)
{
    stub.set_lamda(VADDR(QDialog, show), [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&UserShareHelper::currentUserName, [] { __DBG_STUB_INVOKE__ return QString("test"); });
    stub.set_lamda(&UserShareHelper::setSambaPasswd, [] { __DBG_STUB_INVOKE__ });
    int onBtnCalls = 0;
    stub.set_lamda(&UserSharePasswordSettingDialog::onButtonClicked,
                   [&onBtnCalls] { __DBG_STUB_INVOKE__ ++onBtnCalls; });

    widget->setProperty("UserSharePwdSettingDialogShown", false);
    widget->showSharePasswordSettingsDialog();

    UserSharePasswordSettingDialog *dlg = widget->findChild<UserSharePasswordSettingDialog *>();
    ASSERT_NE(dlg, nullptr);
    EXPECT_TRUE(widget->property("UserSharePwdSettingDialogShown").toBool());

    // 修复后：buttonClicked 必须触发 onButtonClicked（密码提交入口）
    emit dlg->buttonClicked(1, QString());
    EXPECT_EQ(onBtnCalls, 1);

    // 修复前缺陷路径：finished 不得再触发 onButtonClicked
    emit dlg->finished(0);
    EXPECT_EQ(onBtnCalls, 1);

    // closed 必须复位属性，否则二次修改密码被 UserSharePwdSettingDialogShown 拦截（本缺陷现象）
    emit dlg->closed();
    EXPECT_FALSE(widget->property("UserSharePwdSettingDialogShown").toBool());

    // 属性复位后二次打开不再被拦截，且创建的是新对话框实例
    widget->showSharePasswordSettingsDialog();
    EXPECT_EQ(widget->findChildren<UserSharePasswordSettingDialog *>().count(), 2);
}

// ---------- PMS sev-2 regression: BUG267179 ----------
#include <dfm-base/file/local/syncfileinfo.h>

// PMS:267179 系统盘-空白处右键单击属性文件管理器闪退（修复 c94d91d9e）：系统盘等特殊路径下
// 父目录 watcher 创建失败（WatcherFactory::create 返回空），修复前 init() 未判空直接
// watcher->startWatcher() 解引用空指针崩溃，initConnection() 亦未判空就 connect(watcher.data(),...)；
// 修复后两处均需判空跳过。回归：watcher 创建返回空时属性页构造（init+initConnection）
// 全程不得解引用空指针
TEST_F(UT_ShareControlWidget, BUG267179_InitWatcherCreateFailedNoCrash)
{
    // 根因场景：info 创建成功但父目录 watcher 创建失败（系统盘路径 create 返回空）
    stub.set_lamda(&WatcherFactory::create<AbstractFileWatcher>, [&](const QUrl &, bool, QString *) {
        __DBG_STUB_INVOKE__
        return AbstractFileWatcherPointer(nullptr);
    });
    stub.set_lamda(static_cast<QSharedPointer<FileInfo> (*)(const QUrl &, Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
                   [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                       __DBG_STUB_INVOKE__
                       return QSharedPointer<FileInfo>(new SyncFileInfo(url));
                   });
    // 共享状态查询短路，避免依赖真实 samba 环境
    stub.set_lamda(&UserShareHelper::shareNameByPath, [] { __DBG_STUB_INVOKE__ return QString(); });
    stub.set_lamda(&UserShareHelper::isShared, [] { __DBG_STUB_INVOKE__ return false; });

    // 构造即触发 init() + initConnection()：修复前空 watcher 解引用此处闪退
    ShareControlWidget w(QUrl::fromLocalFile("/home"));

    // init() 需越过 watcher 判空点，继续完成未共享状态的 UI 初始化
    EXPECT_FALSE(w.shareSwitcher->isChecked());
    EXPECT_FALSE(w.sharePermissionSelector->isEnabled());
    EXPECT_FALSE(w.shareAnonymousSelector->isEnabled());
    // initConnection() 需执行到底（其末尾 showMoreInfo(false) 收起更多信息区）
    EXPECT_TRUE(w.moreInfoFrame->isHidden());
}
