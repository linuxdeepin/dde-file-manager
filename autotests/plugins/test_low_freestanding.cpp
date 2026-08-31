// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_freestanding.cpp
 * @brief Unit tests for free-standing Low-priority functions
 */

#include <gtest/gtest.h>

class LowFreeStandingTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(LowFreeStandingTest, createVault)
{
    EXPECT_NO_FATAL_FAILURE(createVault());
}

TEST_F(LowFreeStandingTest, AbstractDirIterator)
{
    AbstractDirIterator *obj = new AbstractDirIterator();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->close());
    EXPECT_NO_FATAL_FAILURE(obj->setArguments());
    EXPECT_NO_FATAL_FAILURE({ obj->initIterator(); });
    EXPECT_NO_FATAL_FAILURE({ obj->fileInfos(); });
    delete obj;
}

TEST_F(LowFreeStandingTest, AbstractMenuScenePrivate)
{
    AbstractMenuScenePrivate *obj = new AbstractMenuScenePrivate();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->initializeParamsIsValid(); });
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
    EXPECT_NO_FATAL_FAILURE({ obj->currentDir(); });
    EXPECT_NO_FATAL_FAILURE({ obj->selectFiles(); });
    delete obj;
}

TEST_F(LowFreeStandingTest, DArrowLineDrawer)
{
    EXPECT_NO_FATAL_FAILURE({ DArrowLineDrawer({}); });
}

TEST_F(LowFreeStandingTest, InitSuffixTable)
{
    QSet<QString> _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(InitSuffixTable(_arg0, _arg1));
}

TEST_F(LowFreeStandingTest, ProxyFileInfo)
{
    ProxyFileInfo *obj = new ProxyFileInfo();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->exists(); });
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
    EXPECT_NO_FATAL_FAILURE({ obj->fileUrl(); });
    delete obj;
}

TEST_F(LowFreeStandingTest, accessibleFactory)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ accessibleFactory(_arg0, nullptr); });
}

TEST_F(LowFreeStandingTest, applyReplacementIfNeeded)
{
    FileOperateBaseWorker::ReplacementTarget _arg0{};
    FileCleanupManager _arg1{};
    EXPECT_NO_FATAL_FAILURE({ applyReplacementIfNeeded(_arg0, _arg1); });
}

TEST_F(LowFreeStandingTest, compatibilityFuncForDisbaleAutoMerage)
{
    EXPECT_NO_FATAL_FAILURE(compatibilityFuncForDisbaleAutoMerage(nullptr));
}

TEST_F(LowFreeStandingTest, connectSliderTip)
{
    EXPECT_NO_FATAL_FAILURE(connectSliderTip(nullptr, Func()));
}

TEST_F(LowFreeStandingTest, constructSymlinkFileName)
{
    FileNameComponents _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ constructSymlinkFileName(_arg0, _arg1); });
}

TEST_F(LowFreeStandingTest, covertDesktopUrlToFile)
{
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ covertDesktopUrlToFile(_arg0); });
}

TEST_F(LowFreeStandingTest, covertFileUrlToDesktop)
{
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ covertFileUrlToDesktop(_arg0); });
}

TEST_F(LowFreeStandingTest, createFileInfo)
{
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ createFileInfo(_arg0, {}); });
}

TEST_F(LowFreeStandingTest, currentFilePath)
{
    EXPECT_NO_FATAL_FAILURE({ currentFilePath({}); });
}

TEST_F(LowFreeStandingTest, ddplugin_background)
{
    EXPECT_NO_FATAL_FAILURE({ ddplugin_background(); });
}

TEST_F(LowFreeStandingTest, dealRectRatio)
{
    EXPECT_NO_FATAL_FAILURE({ dealRectRatio(QRect()); });
}

TEST_F(LowFreeStandingTest, dfm_setup_computer_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_computer_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_daemon_filemanager1_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_daemon_filemanager1_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_daemon_opticalshare_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_daemon_opticalshare_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_daemon_recent_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_daemon_recent_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_daemon_tag_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_daemon_tag_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_daemon_vault_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_daemon_vault_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_desktop_background_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_desktop_background_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_desktop_canvas_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_desktop_canvas_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_desktop_core_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_desktop_core_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_desktop_wallpapersetting_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_desktop_wallpapersetting_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_dirshare_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_dirshare_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_disk_encrypt_entry_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_disk_encrypt_entry_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_filedialog_core_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_filedialog_core_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_fileoperations_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_fileoperations_dbus_interfaces());
}

TEST_F(LowFreeStandingTest, dfm_setup_fileoperations_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_fileoperations_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_optical_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_optical_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_recent_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_recent_dbus_interfaces());
}

TEST_F(LowFreeStandingTest, dfm_setup_recent_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_recent_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_search_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_search_dbus_interfaces());
}

TEST_F(LowFreeStandingTest, dfm_setup_search_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_search_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_sidebar_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_sidebar_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_smbbrowser_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_smbbrowser_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_tag_dbus_interfaces)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_tag_dbus_interfaces());
}

TEST_F(LowFreeStandingTest, dfm_setup_tag_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_tag_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_trash_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_trash_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_utils_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_utils_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_vault_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_vault_dependencies());
}

TEST_F(LowFreeStandingTest, dfm_setup_workspace_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_workspace_dependencies());
}

TEST_F(LowFreeStandingTest, dfmplugin_diskenc)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_diskenc(); });
}

TEST_F(LowFreeStandingTest, dfmplugin_myshares)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_myshares(); });
}

TEST_F(LowFreeStandingTest, dfmplugin_trashcore)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_trashcore(); });
}

TEST_F(LowFreeStandingTest, doActionForEveryPlugin)
{
    EXPECT_NO_FATAL_FAILURE({ doActionForEveryPlugin(nullptr); });
}

TEST_F(LowFreeStandingTest, editorMarginTop)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ editorMarginTop(_arg0); });
}

TEST_F(LowFreeStandingTest, elideTemplatePath)
{
    QFontMetrics _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE({ elideTemplatePath(_arg0, _arg1, _arg2, 0); });
}

TEST_F(LowFreeStandingTest, extractHighlightKeyword)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ extractHighlightKeyword(_arg0); });
}

TEST_F(LowFreeStandingTest, formatCap)
{
    EXPECT_NO_FATAL_FAILURE({ formatCap(0, 0, {}); });
}

TEST_F(LowFreeStandingTest, generateCacheKey)
{
    QString _arg0{};
    QSize _arg1{};
    EXPECT_NO_FATAL_FAILURE({ generateCacheKey(_arg0, _arg1, 0); });
}

TEST_F(LowFreeStandingTest, generateNonConflictingSymlinkName)
{
    EXPECT_NO_FATAL_FAILURE({ generateNonConflictingSymlinkName(FileInfoPointer(), FileInfoPointer()); });
}

TEST_F(LowFreeStandingTest, generateSymlinkSuffix)
{
    EXPECT_NO_FATAL_FAILURE({ generateSymlinkSuffix(0); });
}

TEST_F(LowFreeStandingTest, getAccessibleName)
{
    EXPECT_NO_FATAL_FAILURE({ getAccessibleName(nullptr, QAccessible::Role(), QString()); });
}

TEST_F(LowFreeStandingTest, getCompressorPidShell)
{
    EXPECT_NO_FATAL_FAILURE({ getCompressorPidShell(QString()); });
}

TEST_F(LowFreeStandingTest, getIconName)
{
    CrumbData _arg0{};
    EXPECT_NO_FATAL_FAILURE({ getIconName(_arg0); });
}

TEST_F(LowFreeStandingTest, getObjPrefix)
{
    EXPECT_NO_FATAL_FAILURE({ getObjPrefix(QAccessible::Role()); });
}

TEST_F(LowFreeStandingTest, getOriginalPixmap)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ getOriginalPixmap(_arg0); });
}

TEST_F(LowFreeStandingTest, getScaledPixmap)
{
    QIcon _arg0{};
    EXPECT_NO_FATAL_FAILURE({ getScaledPixmap(_arg0, 0, nullptr); });
}

TEST_F(LowFreeStandingTest, getScreenName)
{
    EXPECT_NO_FATAL_FAILURE({ getScreenName(nullptr); });
}

TEST_F(LowFreeStandingTest, hasComputerMenuRegisted)
{
    EXPECT_NO_FATAL_FAILURE({ hasComputerMenuRegisted(); });
}

TEST_F(LowFreeStandingTest, highlightRequestedKey)
{
    EXPECT_NO_FATAL_FAILURE({ highlightRequestedKey(); });
}

TEST_F(LowFreeStandingTest, initSettingPane)
{
    EXPECT_NO_FATAL_FAILURE(initSettingPane());
}

TEST_F(LowFreeStandingTest, insertToList)
{
    QList<T> _arg0{};
    T _arg2{};
    EXPECT_NO_FATAL_FAILURE(insertToList(_arg0, 0, _arg2));
}

TEST_F(LowFreeStandingTest, intervalMs)
{
    EXPECT_NO_FATAL_FAILURE({ intervalMs({}); });
}

TEST_F(LowFreeStandingTest, isAltPressed)
{
    EXPECT_NO_FATAL_FAILURE({ isAltPressed(); });
}

TEST_F(LowFreeStandingTest, isForce)
{
    EXPECT_NO_FATAL_FAILURE({ isForce(); });
}

TEST_F(LowFreeStandingTest, mapBytesToDisplayableCharset)
{
    EXPECT_NO_FATAL_FAILURE({ mapBytesToDisplayableCharset(nullptr, {}, nullptr, {}); });
}

TEST_F(LowFreeStandingTest, mappedAnimationRect)
{
    GridPos _arg1{};
    EXPECT_NO_FATAL_FAILURE({ mappedAnimationRect(nullptr, _arg1); });
}

TEST_F(LowFreeStandingTest, menuSceneUnbind)
{
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ menuSceneUnbind(_arg0, _arg1); });
}

TEST_F(LowFreeStandingTest, menuSceneUnregisterScene)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ menuSceneUnregisterScene(_arg0); });
}

TEST_F(LowFreeStandingTest, menu_action_id)
{
    EXPECT_NO_FATAL_FAILURE({ menu_action_id(); });
}

TEST_F(LowFreeStandingTest, oemMenuExtensionsPath)
{
    EXPECT_NO_FATAL_FAILURE({ oemMenuExtensionsPath(); });
}

TEST_F(LowFreeStandingTest, Operator_ShiftLeft)
{
    EXPECT_NO_FATAL_FAILURE({ Operator_ShiftLeft *obj = new Operator_ShiftLeft(); delete obj; });
}

TEST_F(LowFreeStandingTest, Operator_ShiftRight)
{
    EXPECT_NO_FATAL_FAILURE({ Operator_ShiftRight *obj = new Operator_ShiftRight(); delete obj; });
}

TEST_F(LowFreeStandingTest, paintSeparator)
{
    QStyleOptionViewItem _arg1{};
    EXPECT_NO_FATAL_FAILURE(paintSeparator(nullptr, _arg1));
}

TEST_F(LowFreeStandingTest, pbkdf2)
{
    pbkdf2 *obj = new pbkdf2();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->pbkdf2EncrypyPassword(_arg0, _arg1, 0, 0); });
    EXPECT_NO_FATAL_FAILURE({ obj->octalToHexadecimal(nullptr, 0); });
    EXPECT_NO_FATAL_FAILURE({ obj->createRandomSalt(0); });
    EXPECT_NO_FATAL_FAILURE({ obj->charToHexadecimalChar('\0'); });
    delete obj;
}

TEST_F(LowFreeStandingTest, readRandomBytesFromDevice)
{
    EXPECT_NO_FATAL_FAILURE({ readRandomBytesFromDevice(nullptr, {}); });
}

TEST_F(LowFreeStandingTest, registScheme)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ registScheme(_arg0); });
}

TEST_F(LowFreeStandingTest, registSchemeHandler)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(registSchemeHandler(_arg0, PrehandlerFunc()));
}

TEST_F(LowFreeStandingTest, registerFileSystem)
{
    EXPECT_NO_FATAL_FAILURE(registerFileSystem());
}

TEST_F(LowFreeStandingTest, requestHighlightContent)
{
    SortInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(requestHighlightContent(_arg0));
}

TEST_F(LowFreeStandingTest, roleToGroupingStrategy)
{
    DFMGLOBAL_NAMESPACE::ItemRoles _arg0{};
    EXPECT_NO_FATAL_FAILURE({ roleToGroupingStrategy(_arg0); });
}

TEST_F(LowFreeStandingTest, setDesktopWindowOld)
{
    EXPECT_NO_FATAL_FAILURE(setDesktopWindowOld(nullptr));
}

TEST_F(LowFreeStandingTest, setPrviewWindow)
{
    EXPECT_NO_FATAL_FAILURE(setPrviewWindow(nullptr));
}

TEST_F(LowFreeStandingTest, setRepeat)
{
    EXPECT_NO_FATAL_FAILURE(setRepeat());
}

TEST_F(LowFreeStandingTest, setupDbusInterface)
{
    QDBusInterface _arg0{};
    EXPECT_NO_FATAL_FAILURE(setupDbusInterface(_arg0));
}

TEST_F(LowFreeStandingTest, shouldRequestHighlight)
{
    SortInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE({ shouldRequestHighlight(_arg0); });
}

TEST_F(LowFreeStandingTest, travers_prehandler)
{
    travers_prehandler *obj = new travers_prehandler();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->smbAccessPrehandler(0, _arg1, {}));
    EXPECT_NO_FATAL_FAILURE(obj->doChangeCurrentUrl(0, _arg1, _arg2, _arg3));
    EXPECT_NO_FATAL_FAILURE(obj->onSmbRootMounted(_arg0, Handler()));
    delete obj;
}

TEST_F(LowFreeStandingTest, trimTrailingSlashes)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ trimTrailingSlashes(_arg0); });
}

TEST_F(LowFreeStandingTest, validPrimaryChanged)
{
    EXPECT_NO_FATAL_FAILURE({ validPrimaryChanged(nullptr); });
}

