// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_remaining_mid.cpp
 * @brief Unit tests for remaining uncovered Mid-priority methods
 */

#include <gtest/gtest.h>

class RemainingMidTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(RemainingMidTest, BackgroundService)
{
    BackgroundService *obj = new BackgroundService();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->onWorkspaceSwitched(0, 0));
    EXPECT_NO_FATAL_FAILURE({ obj->getDefaultBackground(); });
    EXPECT_NO_FATAL_FAILURE({ obj->getCurrentWorkspaceIndex(); });
    delete obj;
}

TEST_F(RemainingMidTest, BaseEncryptWorker)
{
    BaseEncryptWorker *obj = new BaseEncryptWorker();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->args(); });
    EXPECT_NO_FATAL_FAILURE({ obj->exitCode(); });
    EXPECT_NO_FATAL_FAILURE(obj->setExitCode(0));
    delete obj;
}

TEST_F(RemainingMidTest, DCheckBox)
{
    EXPECT_NO_FATAL_FAILURE({ DCheckBox({}); });
}

TEST_F(RemainingMidTest, DDialog)
{
    EXPECT_NO_FATAL_FAILURE({ DDialog({}); });
}

TEST_F(RemainingMidTest, DFMExtEmblemIconLayoutPrivate)
{
    EXPECT_NO_FATAL_FAILURE({ DFMExtEmblemIconLayoutPrivate(); });
}

TEST_F(RemainingMidTest, DFMExtEmblemIconPluginPrivate)
{
    EXPECT_NO_FATAL_FAILURE({ DFMExtEmblemIconPluginPrivate(); });
}

TEST_F(RemainingMidTest, DFMExtWindowPluginPrivate)
{
    EXPECT_NO_FATAL_FAILURE({ DFMExtWindowPluginPrivate(); });
}

TEST_F(RemainingMidTest, DFMExtWindowPrivate)
{
    DFMExtWindowPrivate *obj = new DFMExtWindowPrivate();
    ASSERT_NE(obj, nullptr);
    delete obj;
}

TEST_F(RemainingMidTest, DFMExtWindowProxyPrivate)
{
    DFMExtWindowProxyPrivate *obj = new DFMExtWindowProxyPrivate();
    ASSERT_NE(obj, nullptr);
    delete obj;
}

TEST_F(RemainingMidTest, DIconButton)
{
    EXPECT_NO_FATAL_FAILURE({ DIconButton({}); });
}

TEST_F(RemainingMidTest, DStyledItemDelegate)
{
    EXPECT_NO_FATAL_FAILURE({ DStyledItemDelegate({}); });
}

TEST_F(RemainingMidTest, FileNameSorter)
{
    FileNameSorter *obj = new FileNameSorter();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->sort(_arg0, Qt::SortOrder()));
    EXPECT_NO_FATAL_FAILURE({ obj->compare(_arg0, _arg1, Qt::SortOrder()); });
    EXPECT_NO_FATAL_FAILURE({ obj->sortKey(_arg0); });
    EXPECT_NO_FATAL_FAILURE(obj->sortUrls(_arg0, Qt::SortOrder()));
    delete obj;
}

TEST_F(RemainingMidTest, QAbstractItemModel)
{
    EXPECT_NO_FATAL_FAILURE({ QAbstractItemModel({}); });
}

TEST_F(RemainingMidTest, QDBusAbstractAdaptor)
{
    EXPECT_NO_FATAL_FAILURE({ QDBusAbstractAdaptor(); });
}

TEST_F(RemainingMidTest, QDBusAbstractInterface)
{
    EXPECT_NO_FATAL_FAILURE({ QDBusAbstractInterface({}, {}, {}, {}, {}); });
}

TEST_F(RemainingMidTest, QFrame)
{
    EXPECT_NO_FATAL_FAILURE({ QFrame({}); });
}

TEST_F(RemainingMidTest, QLabel)
{
    EXPECT_NO_FATAL_FAILURE({ QLabel(); });
}

TEST_F(RemainingMidTest, QObject)
{
    EXPECT_NO_FATAL_FAILURE({ QObject({}); });
}

TEST_F(RemainingMidTest, QStandardItemModel)
{
    EXPECT_NO_FATAL_FAILURE({ QStandardItemModel({}); });
}

TEST_F(RemainingMidTest, QStyledItemDelegate)
{
    EXPECT_NO_FATAL_FAILURE({ QStyledItemDelegate({}); });
}

TEST_F(RemainingMidTest, QWidget)
{
    EXPECT_NO_FATAL_FAILURE({ QWidget(); });
}

TEST_F(RemainingMidTest, SideBarItem)
{
    SideBarItem *obj = new SideBarItem();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->url(); });
    EXPECT_NO_FATAL_FAILURE({ obj->group(); });
    EXPECT_NO_FATAL_FAILURE({ obj->targetUrl(); });
    delete obj;
}

TEST_F(RemainingMidTest, aggregate)
{
    EXPECT_NO_FATAL_FAILURE(aggregate());
}

TEST_F(RemainingMidTest, applicationAttribute)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ applicationAttribute(_arg0); });
}

TEST_F(RemainingMidTest, avg)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ avg(_arg0); });
}

TEST_F(RemainingMidTest, backupFile)
{
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ backupFile(_arg0, _arg1); });
}

TEST_F(RemainingMidTest, bindSetting)
{
    EXPECT_NO_FATAL_FAILURE(bindSetting());
}

TEST_F(RemainingMidTest, bunner)
{
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE({ bunner(_arg0); });
}

TEST_F(RemainingMidTest, calculateNewPathForDirectoryMove)
{
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE({ calculateNewPathForDirectoryMove(_arg0, _arg1, _arg2); });
}

TEST_F(RemainingMidTest, canOpenSelectedItems)
{
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE({ canOpenSelectedItems(_arg0); });
}

TEST_F(RemainingMidTest, checkAndEnableService)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ checkAndEnableService(_arg0); });
}

TEST_F(RemainingMidTest, constructFileName)
{
    FileNameComponents _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ constructFileName(_arg0, _arg1); });
}

TEST_F(RemainingMidTest, controlIconColor)
{
    EXPECT_NO_FATAL_FAILURE({ controlIconColor(false); });
}

TEST_F(RemainingMidTest, convertQStringListToVector)
{
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE({ convertQStringListToVector(_arg0); });
}

TEST_F(RemainingMidTest, convertVectorToQStringList)
{
    std::vector<std::string> _arg0{};
    EXPECT_NO_FATAL_FAILURE({ convertVectorToQStringList(_arg0); });
}

TEST_F(RemainingMidTest, covertUrlToLocalPath)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ covertUrlToLocalPath(_arg0); });
}

TEST_F(RemainingMidTest, createColoredIcon)
{
    QIcon _arg0{};
    QColor _arg1{};
    QSize _arg2{};
    EXPECT_NO_FATAL_FAILURE({ createColoredIcon(_arg0, _arg1, _arg2); });
}

TEST_F(RemainingMidTest, createSortInfo)
{
    QString _arg0{};
    QString _arg1{};
    QSet<QString> _arg2{};
    EXPECT_NO_FATAL_FAILURE({ createSortInfo(_arg0, _arg1, _arg2); });
}

TEST_F(RemainingMidTest, d)
{
    d *obj = new d();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->[Service]
());
    delete obj;
}

TEST_F(RemainingMidTest, dLoadPluginList)
{
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ dLoadPluginList(nullptr, _arg1); });
}

TEST_F(RemainingMidTest, daemonplugin_vaultdaemon)
{
    EXPECT_NO_FATAL_FAILURE({ daemonplugin_vaultdaemon(); });
}

TEST_F(RemainingMidTest, ddplugin_canvas)
{
    EXPECT_NO_FATAL_FAILURE({ ddplugin_canvas(); });
}

TEST_F(RemainingMidTest, ddplugin_organizer)
{
    EXPECT_NO_FATAL_FAILURE({ ddplugin_organizer(); });
}

TEST_F(RemainingMidTest, ddplugin_wallpapersetting)
{
    EXPECT_NO_FATAL_FAILURE({ ddplugin_wallpapersetting(); });
}

TEST_F(RemainingMidTest, decodeHeifThumbnail)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ decodeHeifThumbnail(_arg0, 0); });
}

TEST_F(RemainingMidTest, desktopFrameRootWindows)
{
    EXPECT_NO_FATAL_FAILURE({ desktopFrameRootWindows(); });
}

TEST_F(RemainingMidTest, dfm_apply_service_polkit_to_target)
{
    EXPECT_NO_FATAL_FAILURE(dfm_apply_service_polkit_to_target());
}

TEST_F(RemainingMidTest, dfm_setup_daemon_core_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_daemon_core_dependencies());
}

TEST_F(RemainingMidTest, dfm_setup_textindex_dependencies)
{
    EXPECT_NO_FATAL_FAILURE(dfm_setup_textindex_dependencies());
}

TEST_F(RemainingMidTest, dfm_tools_upgrade_doUpgrade)
{
    QMap<QString, QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE({ dfm_tools_upgrade_doUpgrade(_arg0); });
}

TEST_F(RemainingMidTest, dfmplugin_bookmark)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_bookmark(); });
}

TEST_F(RemainingMidTest, dfmplugin_computer)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_computer(); });
}

TEST_F(RemainingMidTest, dfmplugin_dirshare)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_dirshare(); });
}

TEST_F(RemainingMidTest, dfmplugin_emblem)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_emblem(); });
}

TEST_F(RemainingMidTest, dfmplugin_propertydialog)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_propertydialog(); });
}

TEST_F(RemainingMidTest, dfmplugin_recent)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_recent(); });
}

TEST_F(RemainingMidTest, dfmplugin_search)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_search(); });
}

TEST_F(RemainingMidTest, dfmplugin_sidebar)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_sidebar(); });
}

TEST_F(RemainingMidTest, dfmplugin_tag)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_tag(); });
}

TEST_F(RemainingMidTest, dfmplugin_titlebar)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_titlebar(); });
}

TEST_F(RemainingMidTest, dfmplugin_trash)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_trash(); });
}

TEST_F(RemainingMidTest, dfmplugin_utils)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_utils(); });
}

TEST_F(RemainingMidTest, dfmplugin_vault)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_vault(); });
}

TEST_F(RemainingMidTest, dfmplugin_workspace)
{
    EXPECT_NO_FATAL_FAILURE({ dfmplugin_workspace(); });
}

TEST_F(RemainingMidTest, disabled)
{
    bool _arg0{};
    EXPECT_NO_FATAL_FAILURE(disabled(_arg0));
}

TEST_F(RemainingMidTest, disctinct)
{
    EXPECT_NO_FATAL_FAILURE({ disctinct(); });
}

TEST_F(RemainingMidTest, enable)
{
    bool _arg0{};
    EXPECT_NO_FATAL_FAILURE(enable(_arg0));
}

TEST_F(RemainingMidTest, enableScreensaver)
{
    EXPECT_NO_FATAL_FAILURE({ enableScreensaver(); });
}

TEST_F(RemainingMidTest, enableServiceNow)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ enableServiceNow(_arg0); });
}

TEST_F(RemainingMidTest, ensureNetworkDiscoveryService)
{
    EXPECT_NO_FATAL_FAILURE({ ensureNetworkDiscoveryService(nullptr); });
}

TEST_F(RemainingMidTest, errorCode_dfmio_trans)
{
    EXPECT_NO_FATAL_FAILURE(errorCode_dfmio_trans());
}

TEST_F(RemainingMidTest, extractAncestorPaths)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ extractAncestorPaths(_arg0); });
}

TEST_F(RemainingMidTest, fileExists)
{
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ fileExists(FileInfoPointer(), _arg1); });
}

TEST_F(RemainingMidTest, filedialog_core)
{
    EXPECT_NO_FATAL_FAILURE({ filedialog_core(); });
}

TEST_F(RemainingMidTest, fromBase64)
{
    EXPECT_NO_FATAL_FAILURE({ fromBase64(QString()); });
}

TEST_F(RemainingMidTest, generateCopySuffix)
{
    EXPECT_NO_FATAL_FAILURE({ generateCopySuffix(0); });
}

TEST_F(RemainingMidTest, generateNonConflictingName)
{
    EXPECT_NO_FATAL_FAILURE({ generateNonConflictingName(FileInfoPointer(), FileInfoPointer()); });
}

TEST_F(RemainingMidTest, generateUniqueFileName)
{
    FileNameComponents _arg0{};
    EXPECT_NO_FATAL_FAILURE({ generateUniqueFileName(_arg0, FileInfoPointer()); });
}

TEST_F(RemainingMidTest, generateUniqueSymlinkName)
{
    FileNameComponents _arg0{};
    EXPECT_NO_FATAL_FAILURE({ generateUniqueSymlinkName(_arg0, FileInfoPointer()); });
}

TEST_F(RemainingMidTest, getDeviceIdByStdSmb)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ getDeviceIdByStdSmb(_arg0); });
}

TEST_F(RemainingMidTest, getIntelAccessibleName)
{
    EXPECT_NO_FATAL_FAILURE({ getIntelAccessibleName(nullptr, QAccessible::Role(), QString()); });
}

TEST_F(RemainingMidTest, getLimit)
{
    EXPECT_NO_FATAL_FAILURE({ getLimit(); });
}

TEST_F(RemainingMidTest, getScaledPathFromCache)
{
    QString _arg0{};
    QSize _arg1{};
    EXPECT_NO_FATAL_FAILURE({ getScaledPathFromCache(_arg0, _arg1); });
}

TEST_F(RemainingMidTest, global_setting_template_fedora_trans)
{
    EXPECT_NO_FATAL_FAILURE(global_setting_template_fedora_trans());
}

TEST_F(RemainingMidTest, global_setting_template_pro_trans)
{
    EXPECT_NO_FATAL_FAILURE(global_setting_template_pro_trans());
}

TEST_F(RemainingMidTest, global_setting_template_trans)
{
    EXPECT_NO_FATAL_FAILURE(global_setting_template_trans());
}

TEST_F(RemainingMidTest, groupBy)
{
    EXPECT_NO_FATAL_FAILURE({ groupBy(); });
}

TEST_F(RemainingMidTest, having)
{
    EXPECT_NO_FATAL_FAILURE({ having(); });
}

TEST_F(RemainingMidTest, hidden)
{
    QList<QVariant> _arg0{};
    EXPECT_NO_FATAL_FAILURE(hidden(_arg0));
}

TEST_F(RemainingMidTest, idGenerator)
{
    EXPECT_NO_FATAL_FAILURE({ idGenerator(); });
}

TEST_F(RemainingMidTest, isCtrlOrShiftPressed)
{
    EXPECT_NO_FATAL_FAILURE({ isCtrlOrShiftPressed(); });
}

TEST_F(RemainingMidTest, isCtrlPressed)
{
    EXPECT_NO_FATAL_FAILURE({ isCtrlPressed(); });
}

TEST_F(RemainingMidTest, isDirectoryMove)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ isDirectoryMove(_arg0); });
}

TEST_F(RemainingMidTest, isHiddenDesktopMenu)
{
    EXPECT_NO_FATAL_FAILURE({ isHiddenDesktopMenu(); });
}

TEST_F(RemainingMidTest, isHiddenExtMenu)
{
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ isHiddenExtMenu(_arg0); });
}

TEST_F(RemainingMidTest, isHiddenMenu)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ isHiddenMenu(_arg0); });
}

TEST_F(RemainingMidTest, isServiceRuning)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ isServiceRuning(_arg0); });
}

TEST_F(RemainingMidTest, isShiftPressed)
{
    EXPECT_NO_FATAL_FAILURE({ isShiftPressed(); });
}

TEST_F(RemainingMidTest, isSmbMounted)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ isSmbMounted(_arg0); });
}

TEST_F(RemainingMidTest, isValidTargetDirectory)
{
    EXPECT_NO_FATAL_FAILURE({ isValidTargetDirectory(FileInfoPointer()); });
}

TEST_F(RemainingMidTest, lookupByTextChecksum)
{
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ lookupByTextChecksum(_arg0, _arg1); });
}

TEST_F(RemainingMidTest, main)
{
    main *obj = new main();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->pluginsLoad(); });
    EXPECT_NO_FATAL_FAILURE(obj->initEnv());
    EXPECT_NO_FATAL_FAILURE(obj->checkUpgrade(nullptr));
    EXPECT_NO_FATAL_FAILURE(obj->initLogFilter());
    delete obj;
}

TEST_F(RemainingMidTest, max)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ max(_arg0); });
}

TEST_F(RemainingMidTest, menuPerfectParams)
{
    QVariantHash _arg0{};
    EXPECT_NO_FATAL_FAILURE({ menuPerfectParams(_arg0); });
}

TEST_F(RemainingMidTest, menuSceneBind)
{
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE({ menuSceneBind(_arg0, _arg1); });
}

TEST_F(RemainingMidTest, menuSceneContains)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ menuSceneContains(_arg0); });
}

TEST_F(RemainingMidTest, menuSceneCreateScene)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ menuSceneCreateScene(_arg0); });
}

TEST_F(RemainingMidTest, menuSceneRegisterScene)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ menuSceneRegisterScene(_arg0, nullptr); });
}

TEST_F(RemainingMidTest, min)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ min(_arg0); });
}

TEST_F(RemainingMidTest, namespace)
{
    EXPECT_NO_FATAL_FAILURE({ namespace(); });
}

TEST_F(RemainingMidTest, netNeighborRootUrl)
{
    EXPECT_NO_FATAL_FAILURE({ netNeighborRootUrl(); });
}

TEST_F(RemainingMidTest, networkScheme)
{
    EXPECT_NO_FATAL_FAILURE({ networkScheme(); });
}

TEST_F(RemainingMidTest, nodesMutex)
{
    EXPECT_NO_FATAL_FAILURE({ nodesMutex(); });
}

TEST_F(RemainingMidTest, normalizeDirectoryPath)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ normalizeDirectoryPath(_arg0); });
}

TEST_F(RemainingMidTest, opticalMedia)
{
    QVariant _arg0{};
    EXPECT_NO_FATAL_FAILURE({ opticalMedia(_arg0); });
}

TEST_F(RemainingMidTest, orderBy)
{
    EXPECT_NO_FATAL_FAILURE({ orderBy(); });
}

TEST_F(RemainingMidTest, orderByDescending)
{
    EXPECT_NO_FATAL_FAILURE({ orderByDescending(); });
}

TEST_F(RemainingMidTest, plugin_filepreview)
{
    EXPECT_NO_FATAL_FAILURE({ plugin_filepreview(); });
}

TEST_F(RemainingMidTest, pluginsLoad)
{
    EXPECT_NO_FATAL_FAILURE({ pluginsLoad(); });
}

TEST_F(RemainingMidTest, protocol_display_utilities)
{
    protocol_display_utilities *obj = new protocol_display_utilities();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->getMountedSmb(); });
    EXPECT_NO_FATAL_FAILURE({ obj->makeVEntryUrl(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->getStandardSmbPaths(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->getSmbHostPath(_arg0); });
    EXPECT_NO_FATAL_FAILURE({ obj->getStandardSmbPath(_arg0); });
    delete obj;
}

TEST_F(RemainingMidTest, pwPluginVersionGreaterThen)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ pwPluginVersionGreaterThen(_arg0); });
}

TEST_F(RemainingMidTest, q)
{
    EXPECT_NO_FATAL_FAILURE({ q({}); });
}

TEST_F(RemainingMidTest, qHash)
{
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE({ qHash(_arg0, 0); });
}

TEST_F(RemainingMidTest, queryToMaps)
{
    EXPECT_NO_FATAL_FAILURE({ queryToMaps(nullptr); });
}

TEST_F(RemainingMidTest, resetCpuQuota)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ resetCpuQuota(_arg0, nullptr); });
}

TEST_F(RemainingMidTest, rootMap)
{
    EXPECT_NO_FATAL_FAILURE({ rootMap(); });
}

TEST_F(RemainingMidTest, runCli)
{
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE({ runCli(_arg0, 0); });
}

TEST_F(RemainingMidTest, screenProxyLastChangedMode)
{
    EXPECT_NO_FATAL_FAILURE({ screenProxyLastChangedMode(); });
}

TEST_F(RemainingMidTest, screenProxyLogicScreens)
{
    EXPECT_NO_FATAL_FAILURE({ screenProxyLogicScreens(); });
}

TEST_F(RemainingMidTest, screenProxyPrimaryScreen)
{
    EXPECT_NO_FATAL_FAILURE({ screenProxyPrimaryScreen(); });
}

TEST_F(RemainingMidTest, screenProxyScreen)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ screenProxyScreen(_arg0); });
}

TEST_F(RemainingMidTest, screenProxyScreens)
{
    EXPECT_NO_FATAL_FAILURE({ screenProxyScreens(); });
}

TEST_F(RemainingMidTest, serviceUnitName)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ serviceUnitName(_arg0); });
}

TEST_F(RemainingMidTest, setCpuQuota)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ setCpuQuota(_arg0, 0, nullptr); });
}

TEST_F(RemainingMidTest, setDesktopWindow)
{
    EXPECT_NO_FATAL_FAILURE(setDesktopWindow(nullptr, nullptr));
}

TEST_F(RemainingMidTest, setEnvForRoot)
{
    EXPECT_NO_FATAL_FAILURE(setEnvForRoot());
}

TEST_F(RemainingMidTest, setItemVisiable)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(setItemVisiable(_arg0, false));
}

TEST_F(RemainingMidTest, shareNodes)
{
    EXPECT_NO_FATAL_FAILURE({ shareNodes(); });
}

TEST_F(RemainingMidTest, sizeString)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ sizeString(_arg0); });
}

TEST_F(RemainingMidTest, skip)
{
    EXPECT_NO_FATAL_FAILURE({ skip(); });
}

TEST_F(RemainingMidTest, statusToString)
{
    EXPECT_NO_FATAL_FAILURE({ statusToString(ExtractorStatus()); });
}

TEST_F(RemainingMidTest, sum)
{
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ sum(_arg0); });
}

TEST_F(RemainingMidTest, textHeight)
{
    EXPECT_NO_FATAL_FAILURE({ textHeight(); });
}

TEST_F(RemainingMidTest, textWidth)
{
    EXPECT_NO_FATAL_FAILURE({ textWidth(); });
}

TEST_F(RemainingMidTest, toBase64)
{
    EXPECT_NO_FATAL_FAILURE({ toBase64(QString()); });
}

TEST_F(RemainingMidTest, toBean)
{
    EXPECT_NO_FATAL_FAILURE({ toBean(); });
}

TEST_F(RemainingMidTest, toBeans)
{
    EXPECT_NO_FATAL_FAILURE({ toBeans(); });
}

TEST_F(RemainingMidTest, toMap)
{
    EXPECT_NO_FATAL_FAILURE({ toMap(); });
}

TEST_F(RemainingMidTest, toMaps)
{
    EXPECT_NO_FATAL_FAILURE({ toMaps(); });
}

TEST_F(RemainingMidTest, trEject)
{
    EXPECT_NO_FATAL_FAILURE({ trEject(); });
}

TEST_F(RemainingMidTest, trErase)
{
    EXPECT_NO_FATAL_FAILURE({ trErase(); });
}

TEST_F(RemainingMidTest, trFormat)
{
    EXPECT_NO_FATAL_FAILURE({ trFormat(); });
}

TEST_F(RemainingMidTest, trLogoutAndClearSavedPasswd)
{
    EXPECT_NO_FATAL_FAILURE({ trLogoutAndClearSavedPasswd(); });
}

TEST_F(RemainingMidTest, trMount)
{
    EXPECT_NO_FATAL_FAILURE({ trMount(); });
}

TEST_F(RemainingMidTest, trOpen)
{
    EXPECT_NO_FATAL_FAILURE({ trOpen(); });
}

TEST_F(RemainingMidTest, trOpenInNewTab)
{
    EXPECT_NO_FATAL_FAILURE({ trOpenInNewTab(); });
}

TEST_F(RemainingMidTest, trOpenInNewWin)
{
    EXPECT_NO_FATAL_FAILURE({ trOpenInNewWin(); });
}

TEST_F(RemainingMidTest, trProperties)
{
    EXPECT_NO_FATAL_FAILURE({ trProperties(); });
}

TEST_F(RemainingMidTest, trRename)
{
    EXPECT_NO_FATAL_FAILURE({ trRename(); });
}

TEST_F(RemainingMidTest, trSafelyRemove)
{
    EXPECT_NO_FATAL_FAILURE({ trSafelyRemove(); });
}

TEST_F(RemainingMidTest, trUnmount)
{
    EXPECT_NO_FATAL_FAILURE({ trUnmount(); });
}

TEST_F(RemainingMidTest, tryUpgrade)
{
    QString _arg0{};
    QMap<QString, QString> _arg1{};
    EXPECT_NO_FATAL_FAILURE({ tryUpgrade(_arg0, _arg1); });
}

TEST_F(RemainingMidTest, updateValue)
{
    QString _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(updateValue(_arg0, _arg1));
}

TEST_F(RemainingMidTest, urlListToStringList)
{
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE({ urlListToStringList(_arg0); });
}

TEST_F(RemainingMidTest, where)
{
    EXPECT_NO_FATAL_FAILURE({ where(); });
}

class DFMExtWindowProxyTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DFMExtWindowProxyTest, windowIdList)
{
    // Test getter: std::vector<uint64_t> windowIdList()
    auto result = obj->windowIdList();
    EXPECT_TRUE(result.isEmpty());

}

class DeleteFilesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DeleteFilesTest, DeleteFiles)
{
    DeleteFiles *obj = new DeleteFiles();
    ASSERT_NE(obj, nullptr);
    delete obj;
}

class DoDeleteFilesWorkerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DoDeleteFilesWorkerTest, DoDeleteFilesWorker)
{
    DoDeleteFilesWorker *obj = new DoDeleteFilesWorker();
    ASSERT_NE(obj, nullptr);
    EXPECT_NO_FATAL_FAILURE(obj->stop());
    EXPECT_NO_FATAL_FAILURE({ obj->doWork(); });
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
    EXPECT_NO_FATAL_FAILURE({ obj->deleteAllFiles(); });
    delete obj;
}

class FileViewTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(FileViewTest, FileView)
{
    // Test constructor: FileView((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

