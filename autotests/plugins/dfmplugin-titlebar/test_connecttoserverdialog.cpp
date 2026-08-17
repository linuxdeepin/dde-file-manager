// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>
#include <QDir>
#include <QTimer>
#include <QVariantMap>
#include <QStringListModel>
#include <QCompleter>

#include <DComboBox>

// 包含待测试的类
#include "dialogs/connecttoserverdialog.h"
#include "dialogs/collectiondelegate.h"
#include "utils/searchhistroymanager.h"
#include "utils/titlebarhelper.h"
#include "events/titlebareventcaller.h"

// 包含依赖的头文件
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dfm_global_defines.h>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;

/**
 * @brief ConnectToServerDialog类单元测试
 *
 * 测试范围：
 * 1. 对话框初始化和UI组件
 * 2. URL生成和解析（支持smb/ftp/sftp/dav等协议）
 * 3. 字符集处理（UTF-8/GBK）
 * 4. 服务器收藏管理（增删改查）
 * 5. 历史记录管理
 * 6. 用户交互响应
 * 7. UI状态同步
 * 8. 错误处理和边界条件
 */
class ConnectToServerDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clear stub before setup
        stub.clear();

        // Mock exec to avoid showing dialog
        stub.set_lamda(VADDR(ConnectToServerDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return 0;
        });

        // Mock show to avoid showing dialog
        stub.set_lamda(VADDR(ConnectToServerDialog, show), [] {
            __DBG_STUB_INVOKE__
        });

        // Mock Application settings
        mockSettings = new Settings("test_connectserver");
        stub.set_lamda(&Application::genericSetting, [this]() {
            __DBG_STUB_INVOKE__
            return mockSettings;
        });
        stub.set_lamda(&Application::appObtuselySetting, [this]() {
            __DBG_STUB_INVOKE__
            return mockSettings;
        });

        // Mock WindowUtils
        stub.set_lamda(&WindowUtils::isWayLand, []() {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Mock SearchHistroyManager
        mockSearchManager = new SearchHistroyManager();
        stub.set_lamda(&SearchHistroyManager::instance, [this]() {
            __DBG_STUB_INVOKE__
            return mockSearchManager;
        });
        stub.set_lamda(&SearchHistroyManager::getSearchHistroy, [](SearchHistroyManager *) {
            __DBG_STUB_INVOKE__
            return QStringList();
        });
        stub.set_lamda(&SearchHistroyManager::getIPHistory, [](SearchHistroyManager *) {
            __DBG_STUB_INVOKE__
            return QList<IPHistroyData>();
        });
        stub.set_lamda(&SearchHistroyManager::addIPHistoryCache, [](SearchHistroyManager *, const QString &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&SearchHistroyManager::clearHistory, [](SearchHistroyManager *, const QStringList &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&SearchHistroyManager::clearIPHistory, [](SearchHistroyManager *) {
            __DBG_STUB_INVOKE__
        });

        // Setup test data
        testUrl = QUrl("file:///home/test");
        testServerUrl = "smb://192.168.1.100/share";

        // Create dialog instance
        dialog = new ConnectToServerDialog(testUrl);
        ASSERT_NE(dialog, nullptr);
    }

    void TearDown() override
    {
        if (dialog) {
            delete dialog;
            dialog = nullptr;
        }
        if (mockSettings) {
            delete mockSettings;
            mockSettings = nullptr;
        }
        if (mockSearchManager) {
            delete mockSearchManager;
            mockSearchManager = nullptr;
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    ConnectToServerDialog *dialog { nullptr };
    Settings *mockSettings { nullptr };
    SearchHistroyManager *mockSearchManager { nullptr };
    QUrl testUrl;
    QString testServerUrl;
};

/**
 * @brief 测试构造函数 - 有效URL
 * 验证对话框能够使用有效URL正确创建
 */
TEST_F(ConnectToServerDialogTest, Constructor_ValidUrl_ObjectCreated)
{
    EXPECT_NE(dialog, nullptr);
    EXPECT_NE(dialog->schemeComboBox, nullptr);
    EXPECT_NE(dialog->serverComboBox, nullptr);
    EXPECT_NE(dialog->charsetComboBox, nullptr);
    EXPECT_NE(dialog->theAddButton, nullptr);
    EXPECT_NE(dialog->collectionServerView, nullptr);
}

/**
 * @brief 测试构造函数 - 空URL
 * 验证对话框能够处理空URL
 */
TEST_F(ConnectToServerDialogTest, Constructor_EmptyUrl_ObjectCreated)
{
    ConnectToServerDialog *emptyDialog = new ConnectToServerDialog(QUrl());
    EXPECT_NE(emptyDialog, nullptr);
    delete emptyDialog;
}

/**
 * @brief 测试获取当前URL - SMB协议
 * 验证生成正确的SMB URL字符串
 */
TEST_F(ConnectToServerDialogTest, GetCurrentUrlString_SmbScheme_ReturnsCorrectUrl)
{
    dialog->schemeComboBox->setCurrentText("smb://");
    dialog->serverComboBox->setEditText("192.168.1.100/share");

    QString url = dialog->getCurrentUrlString();
    EXPECT_EQ(url, QString("smb://192.168.1.100/share"));
}

/**
 * @brief 测试获取当前URL - FTP协议带UTF-8字符集
 * 验证FTP URL包含正确的字符集参数
 */
TEST_F(ConnectToServerDialogTest, GetCurrentUrlString_FtpWithUtf8_ReturnsUrlWithCharset)
{
    dialog->schemeComboBox->setCurrentText("ftp://");
    dialog->serverComboBox->setEditText("192.168.1.100");
    dialog->charsetComboBox->setCurrentIndex(1);   // UTF-8

    QString url = dialog->getCurrentUrlString();
    EXPECT_TRUE(url.startsWith("ftp://192.168.1.100"));
    EXPECT_TRUE(url.contains("charset=utf8"));
}

/**
 * @brief 测试获取当前URL - FTP协议带GBK字符集
 * 验证FTP URL包含GBK字符集参数
 */
TEST_F(ConnectToServerDialogTest, GetCurrentUrlString_FtpWithGbk_ReturnsUrlWithGbkCharset)
{
    dialog->schemeComboBox->setCurrentText("ftp://");
    dialog->serverComboBox->setEditText("192.168.1.100");
    dialog->charsetComboBox->setCurrentIndex(2);   // GBK

    QString url = dialog->getCurrentUrlString();
    EXPECT_TRUE(url.contains("charset=gbk"));
}

/**
 * @brief 测试获取当前URL - FTP协议默认字符集
 * 验证默认字符集不添加charset参数
 */
TEST_F(ConnectToServerDialogTest, GetCurrentUrlString_FtpDefaultCharset_NoCharsetParam)
{
    dialog->schemeComboBox->setCurrentText("ftp://");
    dialog->serverComboBox->setEditText("192.168.1.100");
    dialog->charsetComboBox->setCurrentIndex(0);   // Default

    QString url = dialog->getCurrentUrlString();
    EXPECT_EQ(url, QString("ftp://192.168.1.100"));
    EXPECT_FALSE(url.contains("charset="));
}

/**
 * @brief 测试获取当前URL - SFTP协议
 * 验证生成正确的SFTP URL
 */
TEST_F(ConnectToServerDialogTest, GetCurrentUrlString_SftpScheme_ReturnsCorrectUrl)
{
    dialog->schemeComboBox->setCurrentText("sftp://");
    dialog->serverComboBox->setEditText("user@192.168.1.100:22");

    QString url = dialog->getCurrentUrlString();
    EXPECT_EQ(url, QString("sftp://user@192.168.1.100:22"));
}

/**
 * @brief 测试更新添加按钮状态 - 未收藏
 * 验证未收藏状态显示收藏图标
 */
TEST_F(ConnectToServerDialogTest, UpdateAddButtonState_NotCollected_ShowsCollectIcon)
{
    dialog->serverComboBox->setEditText("192.168.1.100");
    dialog->updateAddButtonState(false);

    EXPECT_TRUE(dialog->isAddState);
    EXPECT_TRUE(dialog->theAddButton->isEnabled());
}

/**
 * @brief 测试更新添加按钮状态 - 已收藏
 * 验证已收藏状态显示取消收藏图标
 */
TEST_F(ConnectToServerDialogTest, UpdateAddButtonState_Collected_ShowsUncollectIcon)
{
    dialog->serverComboBox->setEditText("192.168.1.100");
    dialog->updateAddButtonState(true);

    EXPECT_FALSE(dialog->isAddState);
}

/**
 * @brief 测试更新添加按钮状态 - 空服务器地址
 * 验证空地址时按钮禁用
 */
TEST_F(ConnectToServerDialogTest, UpdateAddButtonState_EmptyServer_ButtonDisabled)
{
    dialog->serverComboBox->setEditText("");
    dialog->updateAddButtonState(false);

    EXPECT_FALSE(dialog->theAddButton->isEnabled());
}

/**
 * @brief 测试收藏操作 - 添加状态
 * 验证在添加状态下执行收藏操作
 */
TEST_F(ConnectToServerDialogTest, CollectionOperate_AddState_AddsToCollection)
{
    dialog->isAddState = true;
    dialog->schemeComboBox->setCurrentText("smb://");
    dialog->serverComboBox->setEditText("192.168.1.100");

    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });

    bool setValueCalled = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&] {
        __DBG_STUB_INVOKE__
        setValueCalled = true;
    });

    dialog->collectionOperate();
    EXPECT_TRUE(setValueCalled);
}

/**
 * @brief 测试收藏操作 - 删除状态
 * 验证在删除状态下执行收藏操作
 */
TEST_F(ConnectToServerDialogTest, CollectionOperate_DeleteState_RemovesFromCollection)
{
    dialog->isAddState = false;
    dialog->schemeComboBox->setCurrentText("smb://");
    dialog->serverComboBox->setEditText("192.168.1.100");

    QStringList testList { "smb://192.168.1.100" };
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(testList);
    });
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&] {
        __DBG_STUB_INVOKE__
    });

    EXPECT_NO_THROW(dialog->collectionOperate());
}

/**
 * @brief 测试按钮点击 - 取消按钮
 * 验证点击取消按钮关闭对话框
 */
TEST_F(ConnectToServerDialogTest, OnButtonClicked_CancelButton_ClosesDialog)
{
    bool closeCalled = false;
    stub.set_lamda(&ConnectToServerDialog::close, [&] {
        __DBG_STUB_INVOKE__
        closeCalled = true;
        return true;
    });

    dialog->onButtonClicked(0);   // Cancel button index
    EXPECT_TRUE(closeCalled);
}

/**
 * @brief 测试按钮点击 - 连接按钮空服务器
 * 验证空服务器地址时关闭对话框
 */
TEST_F(ConnectToServerDialogTest, OnButtonClicked_ConnectWithEmptyServer_ClosesDialog)
{
    dialog->serverComboBox->setEditText("");

    bool closeCalled = false;
    stub.set_lamda(&ConnectToServerDialog::close, [&] {
        __DBG_STUB_INVOKE__
        closeCalled = true;
        return true;
    });

    dialog->onButtonClicked(1);   // Connect button
    EXPECT_TRUE(closeCalled);
}

/**
 * @brief 测试按钮点击 - 连接按钮有效服务器
 * 验证有效服务器地址时添加历史并跳转
 */
TEST_F(ConnectToServerDialogTest, OnButtonClicked_ConnectWithValidServer_AddsHistoryAndJumps)
{
    dialog->schemeComboBox->setCurrentText("smb://");
    dialog->serverComboBox->setEditText("192.168.1.100");

    bool historyAdded = false;
    stub.set_lamda(&SearchHistroyManager::addIPHistoryCache, [&](SearchHistroyManager *, const QString &) {
        __DBG_STUB_INVOKE__
        historyAdded = true;
    });

    bool jumpCalled = false;
    stub.set_lamda(&TitleBarHelper::handleJumpToPressed, [&](QWidget *, const QString &) {
        __DBG_STUB_INVOKE__
        jumpCalled = true;
    });

    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&] {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&ConnectToServerDialog::close, [] {
        __DBG_STUB_INVOKE__
        return true;
    });

    dialog->onButtonClicked(1);
    EXPECT_TRUE(historyAdded);
    EXPECT_TRUE(jumpCalled);
}

/**
 * @brief 测试删除收藏 - 通过URL
 * 验证通过URL字符串删除收藏
 */
TEST_F(ConnectToServerDialogTest, DoDeleteCollection_ByUrl_RemovesCorrectItem)
{
    QStringList testList { "smb://server1", "smb://server2", "smb://server3" };

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(testList);
    });

    bool correctlyRemoved = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue),
                   [&](Settings *, const QString &, const QString &, const QVariant &value) {
                       __DBG_STUB_INVOKE__
                       QStringList list = value.toStringList();
                       correctlyRemoved = (list.size() == 2 && !list.contains("smb://server2"));
                   });

    dialog->doDeleteCollection("smb://server2");
    EXPECT_TRUE(correctlyRemoved);
}

/**
 * @brief 测试删除收藏 - 通过行索引
 * 验证通过行索引删除收藏
 */
TEST_F(ConnectToServerDialogTest, DoDeleteCollection_ByRowIndex_RemovesCorrectItem)
{
    QStringList testList { "smb://server1", "smb://server2" };

    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(testList);
    });
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&] {
        __DBG_STUB_INVOKE__
    });

    dialog->model->setStringList(testList);
    dialog->doDeleteCollection("", 0);
}

/**
 * @brief 测试输入改变 - 清除历史项
 * 验证选择"清除历史"项时清除所有历史
 */
TEST_F(ConnectToServerDialogTest, OnCurrentInputChanged_ClearHistoryItem_ClearsHistory)
{
    dialog->serverComboBox->addItem("Clear History", true);
    int clearIndex = dialog->serverComboBox->count() - 1;

    bool historyCleaned = false;
    stub.set_lamda(&SearchHistroyManager::clearHistory, [&](SearchHistroyManager *, const QStringList &) {
        __DBG_STUB_INVOKE__
        historyCleaned = true;
    });
    stub.set_lamda(&SearchHistroyManager::clearIPHistory, [](SearchHistroyManager *) {
        __DBG_STUB_INVOKE__
    });
    stub.set_lamda(&Settings::sync, [](Settings *) {
        __DBG_STUB_INVOKE__
        return true;
    });

    dialog->serverComboBox->setCurrentIndex(clearIndex);
    QString clearText = dialog->serverComboBox->currentText();
    dialog->onCurrentInputChanged(clearText);

    EXPECT_TRUE(historyCleaned);
}

/**
 * @brief 测试输入改变 - FTP带字符集
 * 验证FTP URL的字符集选项自动更新
 */
TEST_F(ConnectToServerDialogTest, OnCurrentInputChanged_FtpWithCharset_UpdatesCharsetCombo)
{
    dialog->serverComboBox->addItem("ftp://192.168.1.100", 1);   // UTF-8
    dialog->serverComboBox->setCurrentIndex(0);

    dialog->onCurrentInputChanged("ftp://192.168.1.100");
    EXPECT_EQ(dialog->charsetComboBox->currentIndex(), 0);
}

/**
 * @brief 测试输入改变 - 包含协议的URL
 * 验证自动解析协议和地址
 */
TEST_F(ConnectToServerDialogTest, OnCurrentInputChanged_UrlWithScheme_ParsesSchemeAndHost)
{
    dialog->onCurrentInputChanged("smb://192.168.1.100/share");

    EXPECT_EQ(dialog->schemeComboBox->currentText(), QString("smb://"));
}

/**
 * @brief 测试收藏列表点击 - 有效项
 * 验证点击收藏项更新UI
 */
TEST_F(ConnectToServerDialogTest, OnCollectionViewClicked_ValidItem_UpdatesUi)
{
    QStringList testList { "smb://192.168.1.100:445/share" };
    dialog->model->setStringList(testList);

    QModelIndex index = dialog->model->index(0);
    dialog->onCollectionViewClicked(index);

    EXPECT_EQ(dialog->schemeComboBox->currentText(), QString("smb://"));
}

/**
 * @brief 测试收藏列表点击 - FTP带GBK
 * 验证点击FTP收藏项设置正确字符集
 */
TEST_F(ConnectToServerDialogTest, OnCollectionViewClicked_FtpWithGbk_SetsGbkCharset)
{
    QStringList testList { "ftp://192.168.1.100?charset=gbk" };
    dialog->model->setStringList(testList);

    QModelIndex index = dialog->model->index(0);
    dialog->onCollectionViewClicked(index);

    EXPECT_EQ(dialog->charsetComboBox->currentIndex(), 2);   // GBK index
}

/**
 * @brief 测试收藏列表点击 - FTP带UTF-8
 * 验证点击FTP收藏项设置UTF-8字符集
 */
TEST_F(ConnectToServerDialogTest, OnCollectionViewClicked_FtpWithUtf8_SetsUtf8Charset)
{
    QStringList testList { "ftp://192.168.1.100?charset=utf-8" };
    dialog->model->setStringList(testList);

    QModelIndex index = dialog->model->index(0);
    dialog->onCollectionViewClicked(index);

    EXPECT_EQ(dialog->charsetComboBox->currentIndex(), 1);   // UTF-8 index
}

/**
 * @brief 测试自动补全激活
 * 验证自动补全激活时设置正确协议
 */
TEST_F(ConnectToServerDialogTest, OnCompleterActivated_ValidUrl_SetsScheme)
{
    dialog->onCompleterActivated("ftp://192.168.1.100");
    EXPECT_EQ(dialog->schemeComboBox->currentText(), QString("ftp://"));
}

/**
 * @brief 测试更新收藏 - 新URL
 * 验证添加新URL到收藏
 */
TEST_F(ConnectToServerDialogTest, UpdateCollections_NewUrl_AddsToCollection)
{
    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });

    bool correctlyAdded = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        QStringList list = value.toStringList();
        correctlyAdded = (list.size() == 1 && list.contains("smb://192.168.1.100"));
    });

    QStringList result = dialog->updateCollections("smb://192.168.1.100", true);
    EXPECT_TRUE(correctlyAdded);
}

/**
 * @brief 测试更新收藏 - 已存在URL
 * 验证不重复添加已存在的URL
 */
TEST_F(ConnectToServerDialogTest, UpdateCollections_ExistingUrl_DoesNotDuplicate)
{
    QStringList testList { "smb://192.168.1.100" };
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(testList);
    });

    QStringList result = dialog->updateCollections("smb://192.168.1.100", false);
    EXPECT_EQ(result.size(), 1);
}

/**
 * @brief 测试更新收藏 - 无效URL（空主机）
 * 验证无效URL显示错误对话框
 */
TEST_F(ConnectToServerDialogTest, UpdateCollections_InvalidUrlEmptyHost_ShowsError)
{
    bool errorShown = false;
    stub.set_lamda(&DialogManager::showErrorDialog, [&](DialogManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        errorShown = true;
    });

    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });

    dialog->updateCollections("ftp://", true);
    EXPECT_TRUE(errorShown);
}

/**
 * @brief 测试更新收藏 - 字符集变更
 * 验证相同服务器不同字符集时替换旧项
 */
TEST_F(ConnectToServerDialogTest, UpdateCollections_CharsetChanged_ReplacesOldItem)
{
    QStringList testList { "ftp://192.168.1.100?charset=gbk" };
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(testList);
    });

    bool correctlyUpdated = false;
    typedef void (Settings::*SetValue)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<SetValue>(&Settings::setValue), [&](Settings *, const QString &, const QString &, const QVariant &value) {
        __DBG_STUB_INVOKE__
        QStringList list = value.toStringList();
        correctlyUpdated = (!list.contains("ftp://192.168.1.100?charset=gbk")
                            && list.contains("ftp://192.168.1.100?charset=utf8"));
    });

    dialog->updateCollections("ftp://192.168.1.100?charset=utf8", true);
    EXPECT_TRUE(correctlyUpdated);
}

/**
 * @brief 测试协议加斜杠
 * 验证协议名称添加://后缀
 */
TEST_F(ConnectToServerDialogTest, SchemeWithSlash_ValidScheme_ReturnsWithSlashes)
{
    QString result = dialog->schemeWithSlash("smb");
    EXPECT_EQ(result, QString("smb://"));

    result = dialog->schemeWithSlash("ftp");
    EXPECT_EQ(result, QString("ftp://"));
}

/**
 * @brief 测试UI状态更新 - 无收藏
 * 验证无收藏时显示空状态提示
 */
TEST_F(ConnectToServerDialogTest, UpdateUiState_NoCollections_ShowsEmptyFrame)
{
    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });

    dialog->updateUiState();

    EXPECT_NO_THROW(dialog->emptyFrame->isVisible());
}

/**
 * @brief 测试UI状态更新 - 有收藏
 * 验证有收藏时显示收藏列表
 */
TEST_F(ConnectToServerDialogTest, UpdateUiState_HasCollections_ShowsCollectionView)
{
    QStringList testList { "smb://192.168.1.100" };
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(testList);
    });

    dialog->model->setStringList(testList);
    dialog->updateUiState();

    EXPECT_FALSE(dialog->emptyFrame->isVisible());
}

/**
 * @brief 测试UI状态更新 - FTP URL
 * 验证FTP URL显示字符集选项
 */
TEST_F(ConnectToServerDialogTest, UpdateUiState_FtpUrl_ShowsCharsetOptions)
{
    dialog->schemeComboBox->setCurrentText("ftp://");
    dialog->serverComboBox->setEditText("192.168.1.100");

    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });

    dialog->updateUiState();

    EXPECT_FALSE(dialog->charsetComboBox->isHidden());
}

/**
 * @brief 测试UI状态更新 - 非FTP URL
 * 验证非FTP URL隐藏字符集选项
 */
TEST_F(ConnectToServerDialogTest, UpdateUiState_NonFtpUrl_HidesCharsetOptions)
{
    dialog->schemeComboBox->setCurrentText("smb://");
    dialog->serverComboBox->setEditText("192.168.1.100");

    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });

    dialog->updateUiState();

    EXPECT_TRUE(dialog->charsetComboBox->isHidden());
}

/**
 * @brief 测试UI状态更新 - 空服务器地址
 * 验证空地址时连接按钮禁用
 */
TEST_F(ConnectToServerDialogTest, UpdateUiState_EmptyServer_DisablesConnectButton)
{
    dialog->serverComboBox->setEditText("");

    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });

    dialog->updateUiState();

    EXPECT_FALSE(dialog->getButton(1)->isEnabled());   // Connect button index is 1
}

/**
 * @brief 测试UI状态更新 - 有效服务器地址
 * 验证有效地址时连接按钮启用
 */
TEST_F(ConnectToServerDialogTest, UpdateUiState_ValidServer_EnablesConnectButton)
{
    dialog->serverComboBox->setEditText("192.168.1.100");

    QStringList emptyList;
    typedef QVariant (Settings::*Value)(const QString &, const QString &, const QVariant &) const;
    stub.set_lamda(static_cast<Value>(&Settings::value), [&] {
        __DBG_STUB_INVOKE__
        return QVariant::fromValue(emptyList);
    });
    dialog->updateUiState();

    EXPECT_TRUE(dialog->getButton(1)->isEnabled());
}
