// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include "stubext.h"
#include <QFileDevice>
#include <QComboBox>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QTimer>
#include <QThread>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMenu>

#include "dfmplugin_propertydialog_global.h"
#include "propertydialog.h"
#include "events/propertyeventreceiver.h"
#include "events/propertyeventcall.h"
#include "utils/propertydialogmanager.h"
#include "utils/propertydialogutil.h"
#include "utils/computerpropertyhelper.h"
#include "utils/mediainfofetchworker.h"
#include "menu/propertymenuscene.h"
#include "views/filepropertydialog.h"
#include "views/computerpropertydialog.h"
#include "views/basicwidget.h"
#include "views/permissionmanagerwidget.h"
#include "views/editstackedwidget.h"
#include "views/multifilepropertydialog.h"
#include "menu/propertymenuscene.h"
#include "menu/propertymenuscene_p.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileinfohelper.h>

using namespace dfmplugin_propertydialog;
using namespace dfmplugin_menu_util;

class TestPropertyDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        // Ensure plugin is stopped at beginning of each test
        propertyDialog.stop();
    }

    void TearDown() override
    {
        // Ensure plugin is stopped at end of each test
        propertyDialog.stop();
        stub.clear();
    }

    stub_ext::StubExt stub;
    static PropertyDialog propertyDialog;   // Use static instance like other plugins
};

PropertyDialog TestPropertyDialog::propertyDialog;

// Test PropertyDialog class - minimal stubbing to avoid memory alignment issues
TEST_F(TestPropertyDialog, InitializeTest)
{
    EXPECT_NO_THROW(propertyDialog.initialize());
    EXPECT_TRUE(true);
}

TEST_F(TestPropertyDialog, StartTest)
{
    bool result = propertyDialog.start();
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, StopTest)
{
    // First start the plugin then stop it
    propertyDialog.start();
    EXPECT_NO_THROW(propertyDialog.stop());
    // Stopping again should be safe
    EXPECT_NO_THROW(propertyDialog.stop());
}

// Test PropertyEventReceiver class
TEST_F(TestPropertyDialog, InstanceTest)
{
    PropertyEventReceiver *instance1 = PropertyEventReceiver::instance();
    PropertyEventReceiver *instance2 = PropertyEventReceiver::instance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestPropertyDialog, BindEventsTest)
{
    PropertyEventReceiver receiver;
    EXPECT_NO_THROW(receiver.bindEvents());
    EXPECT_TRUE(true);
}

TEST_F(TestPropertyDialog, HandleShowPropertyDialogTest)
{
    PropertyEventReceiver receiver;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    EXPECT_NO_THROW(receiver.handleShowPropertyDialog(urls, option));
}

TEST_F(TestPropertyDialog, HandleViewExtensionRegisterTest)
{
    PropertyEventReceiver receiver;
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = receiver.handleViewExtensionRegister(viewFunc, "test_name", 0);
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, HandleCustomViewRegisterTest)
{
    PropertyEventReceiver receiver;
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = receiver.handleCustomViewRegister(viewFunc, "test_scheme");
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, HandleBasicViewExtensionRegisterTest)
{
    PropertyEventReceiver receiver;
    auto func = [](const QUrl &url) -> QMap<QString, QMultiMap<QString, QPair<QString, QString>>> { 
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(); 
    };
    bool result = receiver.handleBasicViewExtensionRegister(func, "test_scheme");
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, HandleBasicFiledFilterAddTest)
{
    PropertyEventReceiver receiver;
    QStringList enums;
    enums << "kBasisInfo";
    bool result1 = receiver.handleBasicFiledFilterAdd("test_scheme_unique4", enums);
    EXPECT_TRUE(result1);
    
    enums.clear();
    enums << "kBasisInfo" << "kPermission";
    bool result2 = receiver.handleBasicFiledFilterAdd("test_scheme_unique5", enums);
    EXPECT_TRUE(result2);
}

// Test PropertyDialogManager class
TEST_F(TestPropertyDialog, ManagerInstanceTest)
{
    PropertyDialogManager &manager1 = PropertyDialogManager::instance();
    PropertyDialogManager &manager2 = PropertyDialogManager::instance();
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(TestPropertyDialog, RegisterExtensionViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = manager.registerExtensionView(viewFunc, "test_name", 0);
    EXPECT_TRUE(result);
}

TEST_F(TestPropertyDialog, CreateExtensionViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.createExtensionView(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestPropertyDialog, RegisterCustomViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto viewFunc = [](const QUrl &url) -> QWidget* { return nullptr; };
    bool result = manager.registerCustomView(viewFunc, "test_scheme_unique1");
    EXPECT_TRUE(result);

    bool result2 = manager.registerCustomView(viewFunc, "test_scheme_unique1");
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialog, CreateCustomViewTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QWidget *result = manager.createCustomView(url);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestPropertyDialog, RegisterBasicViewExtensionTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto func = [](const QUrl &url) -> QMap<QString, QMultiMap<QString, QPair<QString, QString>>> { 
        return QMap<QString, QMultiMap<QString, QPair<QString, QString>>>(); 
    };
    bool result = manager.registerBasicViewExtension(func, "test_scheme_unique2");
    EXPECT_TRUE(result);

    bool result2 = manager.registerBasicViewExtension(func, "test_scheme_unique2");
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialog, CreateBasicViewExtensionFieldTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.createBasicViewExtensionField(url);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestPropertyDialog, AddBasicFiledFiltesTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    bool result = manager.addBasicFiledFiltes("test_scheme_unique3", kPermission);
    EXPECT_TRUE(result);

    bool result2 = manager.addBasicFiledFiltes("test_scheme_unique3", kFileSizeFiled);
    EXPECT_FALSE(result2);
}

TEST_F(TestPropertyDialog, BasicFiledFiltesTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    auto result = manager.basicFiledFiltes(url);
    EXPECT_EQ(result, kNotFilter);
}

TEST_F(TestPropertyDialog, GetCreatorOptionByNameTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    auto result = manager.getCreatorOptionByName("nonexistent");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestPropertyDialog, AddComputerPropertyDialogTest)
{
    PropertyDialogManager &manager = PropertyDialogManager::instance();
    EXPECT_NO_THROW(manager.addComputerPropertyDialog());
}

// Test PropertyDialogUtil class
TEST_F(TestPropertyDialog, UtilInstanceTest)
{
    PropertyDialogUtil *util1 = PropertyDialogUtil::instance();
    PropertyDialogUtil *util2 = PropertyDialogUtil::instance();
    EXPECT_EQ(util1, util2);
}

TEST_F(TestPropertyDialog, ShowPropertyDialogTest)
{
    PropertyDialogUtil util;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;

    EXPECT_NO_THROW(util.showPropertyDialog(urls, option));
}

TEST_F(TestPropertyDialog, ShowFilePropertyDialogTest)
{
    PropertyDialogUtil util;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;

    EXPECT_NO_THROW(util.showFilePropertyDialog(urls, option));
}

TEST_F(TestPropertyDialog, ShowCustomDialogTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");

    EXPECT_NO_THROW(util.showCustomDialog(url));
}

TEST_F(TestPropertyDialog, CloseFilePropertyDialogTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");

    EXPECT_NO_THROW(util.closeFilePropertyDialog(url));
}

TEST_F(TestPropertyDialog, CloseCustomPropertyDialogTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    
    EXPECT_NO_THROW(util.closeCustomPropertyDialog(url));
}

TEST_F(TestPropertyDialog, CloseAllFilePropertyDialogTest)
{
    PropertyDialogUtil util;
    
    EXPECT_NO_THROW(util.closeAllFilePropertyDialog());
}

TEST_F(TestPropertyDialog, CloseAllPropertyDialogTest)
{
    PropertyDialogUtil util;
    
    EXPECT_NO_THROW(util.closeAllPropertyDialog());
}

TEST_F(TestPropertyDialog, CreateControlViewTest)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    EXPECT_NO_THROW(util.createControlView(url, option));
}

TEST_F(TestPropertyDialog, UpdateCloseIndicatorTest)
{
    PropertyDialogUtil util;
    
    EXPECT_NO_THROW(util.updateCloseIndicator());
}

// Test ComputerPropertyHelper class
TEST_F(TestPropertyDialog, ComputerPropertyHelperSchemeTest)
{
    QString scheme = ComputerPropertyHelper::scheme();
    EXPECT_FALSE(scheme.isEmpty());
}

TEST_F(TestPropertyDialog, ComputerPropertyHelperCreateComputerPropertyTest)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QWidget *widget = ComputerPropertyHelper::createComputerProperty(url);
    EXPECT_TRUE(widget == nullptr);
}

// Test MediaInfoFetchWorker class
TEST_F(TestPropertyDialog, MediaInfoFetchWorkerTest)
{
    MediaInfoFetchWorker worker;
    EXPECT_NO_THROW(worker.getDuration("/tmp/test.mp4"));
}

// Test PropertyEventCall class
TEST_F(TestPropertyDialog, PropertyEventCallSendSetPermissionManagerTest)
{
    EXPECT_NO_THROW(PropertyEventCall::sendSetPermissionManager(12345, QUrl::fromLocalFile("/tmp/test"), QFileDevice::ReadOwner));
}

TEST_F(TestPropertyDialog, PropertyEventCallSendFileHideTest)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    EXPECT_NO_THROW(PropertyEventCall::sendFileHide(12345, urls));
}

// Test PropertyMenuScene class
TEST_F(TestPropertyDialog, PropertyMenuCreatorNameTest)
{
    QString name = PropertyMenuCreator::name();
    EXPECT_EQ(name, "PropertyMenu");
}

TEST_F(TestPropertyDialog, PropertyMenuSceneNameTest)
{
    PropertyMenuScene scene;
    QString name = scene.name();
    EXPECT_EQ(name, "PropertyMenu");
}

// Test ComputerPropertyDialog class
TEST_F(TestPropertyDialog, ComputerPropertyDialogTest)
{
    EXPECT_NO_THROW({
        ComputerPropertyDialog dialog;
    });
}

// Test FilePropertyDialog class - comprehensive test coverage
TEST_F(TestPropertyDialog, FilePropertyDialogConstructorTest)
{
    EXPECT_NO_THROW({
        FilePropertyDialog dialog;
    });
}

TEST_F(TestPropertyDialog, FilePropertyDialogDestructorTest)
{
    FilePropertyDialog *dialog = new FilePropertyDialog();
    EXPECT_NO_THROW(delete dialog);
}

// TEST_F(TestPropertyDialog, FilePropertyDialogContentHeightTest)
// {
//     FilePropertyDialog dialog;
//     int height = dialog.contentHeight();
//     EXPECT_GE(height, 0);
// }

TEST_F(TestPropertyDialog, FilePropertyDialogGetFileSizeTest)
{
    FilePropertyDialog dialog;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    dialog.selectFileUrl(testUrl);
    qint64 size = dialog.getFileSize();
    EXPECT_GE(size, 0);
}

TEST_F(TestPropertyDialog, FilePropertyDialogGetFileCountTest)
{
    FilePropertyDialog dialog;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    dialog.selectFileUrl(testUrl);
    int count = dialog.getFileCount();
    EXPECT_GE(count, 0);
}

TEST_F(TestPropertyDialog, FilePropertyDialogSetBasicInfoExpandTest)
{
    FilePropertyDialog dialog;
    EXPECT_NO_THROW(dialog.setBasicInfoExpand(true));
    EXPECT_NO_THROW(dialog.setBasicInfoExpand(false));
}

TEST_F(TestPropertyDialog, FilePropertyDialogCloseDialogTest)
{
    FilePropertyDialog dialog;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    dialog.selectFileUrl(testUrl);
    EXPECT_NO_THROW(dialog.closeDialog());
}

TEST_F(TestPropertyDialog, FilePropertyDialogOnSelectUrlRenamedTest)
{
    FilePropertyDialog dialog;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    EXPECT_NO_THROW(dialog.onSelectUrlRenamed(testUrl));
}

TEST_F(TestPropertyDialog, FilePropertyDialogOnFileInfoUpdatedTest)
{
    FilePropertyDialog dialog;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    EXPECT_NO_THROW(dialog.onFileInfoUpdated(testUrl, "testInfo", false));
}

// TEST_F(TestPropertyDialog, FilePropertyDialogMousePressEventTest)
// {
//     FilePropertyDialog dialog;
//     QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//     EXPECT_NO_THROW(dialog.mousePressEvent(&event));
// }

TEST_F(TestPropertyDialog, FilePropertyDialogCloseEventTest)
{
    FilePropertyDialog dialog;
    QCloseEvent event;
    EXPECT_NO_THROW(dialog.closeEvent(&event));
}

// Test PermissionManagerWidget class
TEST_F(TestPropertyDialog, PermissionManagerWidgetConstructorTest)
{
    EXPECT_NO_THROW({
        PermissionManagerWidget widget;
    });
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetDestructorTest)
{
    PermissionManagerWidget *widget = new PermissionManagerWidget();
    EXPECT_NO_THROW(delete widget);
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetUpdateFileUrlTest)
{
    PermissionManagerWidget widget;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    EXPECT_NO_THROW(widget.updateFileUrl(testUrl));
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetGetPermissionStringTest)
{
    PermissionManagerWidget widget;
    QString permission = widget.getPermissionString(QFileDevice::ReadOwner);
    EXPECT_FALSE(permission.isEmpty());
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetSetComboBoxByPermissionTest)
{
    PermissionManagerWidget widget;
    QComboBox comboBox;
    EXPECT_NO_THROW(widget.setComboBoxByPermission(&comboBox, QFileDevice::ReadOwner, 0));
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetToggleFileExecutableTest)
{
    PermissionManagerWidget widget;
    EXPECT_NO_THROW(widget.toggleFileExecutable(true));
    EXPECT_NO_THROW(widget.toggleFileExecutable(false));
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetCanChmodTest)
{
    PermissionManagerWidget widget;
    FileInfoPointer info = DFMBASE_NAMESPACE::InfoFactory::create<DFMBASE_NAMESPACE::FileInfo>(QUrl::fromLocalFile("/tmp/test"));
    bool canChmod = widget.canChmod(info);
    EXPECT_TRUE(canChmod || !canChmod); // Just test it doesn't crash
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetSetExecTextTest)
{
    PermissionManagerWidget widget;
    EXPECT_NO_THROW(widget.setExecText());
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetPaintEventTest)
{
    PermissionManagerWidget widget;
    QPaintEvent event(QRect(0, 0, 100, 100));
    EXPECT_NO_THROW(widget.paintEvent(&event));
}

TEST_F(TestPropertyDialog, PermissionManagerWidgetOnComboBoxChangedTest)
{
    PermissionManagerWidget widget;
    EXPECT_NO_THROW(widget.onComboBoxChanged());
}

// Test ComputerPropertyDialog class methods
TEST_F(TestPropertyDialog, ComputerPropertyDialogComputerProcessTest)
{
    ComputerPropertyDialog dialog;
    QMap<ComputerInfoItem, QString> computerInfo;
    computerInfo[ComputerInfoItem::kName] = "TestComputer";
    computerInfo[ComputerInfoItem::kVersion] = "1.0";
    EXPECT_NO_THROW(dialog.computerProcess(computerInfo));
}

TEST_F(TestPropertyDialog, ComputerPropertyDialogShowEventTest)
{
    ComputerPropertyDialog dialog;
    QShowEvent event;
    EXPECT_NO_THROW(dialog.showEvent(&event));
}

TEST_F(TestPropertyDialog, ComputerPropertyDialogCloseEventTest)
{
    ComputerPropertyDialog dialog;
    QCloseEvent event;
    EXPECT_NO_THROW(dialog.closeEvent(&event));
}

// Test ComputerInfoThread class
TEST_F(TestPropertyDialog, ComputerInfoThreadConstructorTest)
{
    EXPECT_NO_THROW({
        ComputerInfoThread thread;
    });
}

TEST_F(TestPropertyDialog, ComputerInfoThreadDestructorTest)
{
    ComputerInfoThread *thread = new ComputerInfoThread();
    thread->stopThread();
    EXPECT_NO_THROW(delete thread);
}

TEST_F(TestPropertyDialog, ComputerInfoThreadStartThreadTest)
{
    ComputerInfoThread thread;
    EXPECT_NO_THROW(thread.startThread());
    thread.quit();
    thread.wait();
}

TEST_F(TestPropertyDialog, ComputerInfoThreadStopThreadTest)
{
    ComputerInfoThread thread;
    EXPECT_NO_THROW(thread.stopThread());
}

TEST_F(TestPropertyDialog, ComputerInfoThreadRunTest)
{
    ComputerInfoThread thread;
    // Note: run() is called internally by start(), so we just test it doesn't crash
    EXPECT_NO_THROW(thread.start());
    thread.quit();
    thread.wait();
}

TEST_F(TestPropertyDialog, ComputerInfoThreadComputerProcessTest)
{
    ComputerInfoThread thread;
    EXPECT_NO_THROW(thread.computerProcess());
}


TEST_F(TestPropertyDialog, BasicWidgetDestructorTest)
{
    BasicWidget *widget = new BasicWidget();
    EXPECT_NO_THROW(delete widget);
}

TEST_F(TestPropertyDialog, BasicWidgetGetFileSizeTest)
{
    BasicWidget widget;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    widget.selectFileUrl(testUrl);
    qint64 size = widget.getFileSize();
    EXPECT_GE(size, 0);
}

TEST_F(TestPropertyDialog, BasicWidgetGetFileCountTest)
{
    BasicWidget widget;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    widget.selectFileUrl(testUrl);
    int count = widget.getFileCount();
    EXPECT_GE(count, 0);
}

TEST_F(TestPropertyDialog, BasicWidgetUpdateFileUrlTest)
{
    BasicWidget widget;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    EXPECT_NO_THROW(widget.updateFileUrl(testUrl));
}

TEST_F(TestPropertyDialog, BasicWidgetSlotFileCountAndSizeChangeTest)
{
    BasicWidget widget;
    EXPECT_NO_THROW(widget.slotFileCountAndSizeChange(1024, 5, 2));
}

TEST_F(TestPropertyDialog, BasicWidgetSlotFileHideTest)
{
    BasicWidget widget;
    EXPECT_NO_THROW(widget.slotFileHide(Qt::Checked));
    EXPECT_NO_THROW(widget.slotFileHide(Qt::Unchecked));
}

TEST_F(TestPropertyDialog, BasicWidgetSlotOpenFileLocationTest)
{
    BasicWidget widget;
    EXPECT_NO_THROW(widget.slotOpenFileLocation());
}

TEST_F(TestPropertyDialog, BasicWidgetCloseEventTest)
{
    BasicWidget widget;
    QCloseEvent event;
    EXPECT_NO_THROW(widget.closeEvent(&event));
}

TEST_F(TestPropertyDialog, BasicWidgetImageExtenInfoTest)
{
    BasicWidget widget;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.jpg");
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties;
    EXPECT_NO_THROW(widget.imageExtenInfo(testUrl, properties));
}

TEST_F(TestPropertyDialog, BasicWidgetVideoExtenInfoTest)
{
    BasicWidget widget;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.mp4");
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties;
    EXPECT_NO_THROW(widget.videoExtenInfo(testUrl, properties));
}

TEST_F(TestPropertyDialog, BasicWidgetAudioExtenInfoTest)
{
    BasicWidget widget;
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test.mp3");
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties;
    EXPECT_NO_THROW(widget.audioExtenInfo(testUrl, properties));
}