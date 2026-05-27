// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QTimer>
#include <QApplication>
#include <QScreen>

#include "stubext.h"
#include "utils/propertydialogutil.h"
#include "utils/propertydialogmanager.h"
#include "views/filepropertydialog.h"
#include "views/closealldialog.h"
#include "dfmplugin_propertydialog_global.h"

// Include DPF framework headers
#include <dfm-framework/dpf.h>
#include <dfm-framework/event/eventsequence.h>
#include <dfm-base/utils/windowutils.h>

using namespace dfmplugin_propertydialog;
using namespace dfmbase;

class TestPropertyDialogUtil : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TestPropertyDialogUtil, instance)
{
    PropertyDialogUtil *util1 = PropertyDialogUtil::instance();
    PropertyDialogUtil *util2 = PropertyDialogUtil::instance();
    
    EXPECT_NE(util1, nullptr);
    EXPECT_EQ(util1, util2);
}

TEST_F(TestPropertyDialogUtil, constructor)
{
    PropertyDialogUtil util;
    
    EXPECT_NE(util.closeIndicatorTimer, nullptr);
    EXPECT_NE(util.closeAllDialog, nullptr);
    EXPECT_EQ(util.closeIndicatorTimer->interval(), 1000);
}

TEST_F(TestPropertyDialogUtil, showPropertyDialog_SingleFile)
{
    PropertyDialogUtil util;
    
    // Use correct signature for hook sequence run
    typedef bool (DPF_NAMESPACE::EventSequenceManager::*RunFunc)(const QString &, const QString &, const QUrl &);
    auto run = static_cast<RunFunc>(&DPF_NAMESPACE::EventSequenceManager::run);
    
    bool hookSequenceRunCalled = false;
    stub.set_lamda(run,
                   [&hookSequenceRunCalled](DPF_NAMESPACE::EventSequenceManager *self, const QString &ns, const QString &hook, const QUrl &url) -> bool {
                        hookSequenceRunCalled = true;
                        return false; // Don't disable property dialog
                   });
    
    bool showCustomDialogCalled = false;
    stub.set_lamda(&PropertyDialogUtil::showCustomDialog,
                   [&showCustomDialogCalled](PropertyDialogUtil *self, const QUrl &url) -> bool {
                        showCustomDialogCalled = true;
                        return false; // Don't show custom dialog
                   });
    
    bool showFilePropertyDialogCalled = false;
    stub.set_lamda(&PropertyDialogUtil::showFilePropertyDialog,
                   [&showFilePropertyDialogCalled](PropertyDialogUtil *self, const QList<QUrl> &urls, const QVariantHash &option) {
                        showFilePropertyDialogCalled = true;
                   });
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    util.showPropertyDialog(urls, option);
    
    EXPECT_TRUE(hookSequenceRunCalled);
    EXPECT_TRUE(showCustomDialogCalled);
    EXPECT_TRUE(showFilePropertyDialogCalled);
}

TEST_F(TestPropertyDialogUtil, showFilePropertyDialog_SingleFile)
{
    PropertyDialogUtil util;
    
    bool filterControlViewCalled = false;
    bool setBasicInfoExpandCalled = false;
    bool moveCalled = false;
    bool showCalled = false;
    
    // Stub FilePropertyDialog methods with QWidget* as base type
    stub.set_lamda(&FilePropertyDialog::selectFileUrl,
                   [](QWidget *self, const QUrl &url) {
                        Q_UNUSED(self)
                        Q_UNUSED(url)
                        // Mock implementation
                   });
    stub.set_lamda(&FilePropertyDialog::filterControlView,
                   [&filterControlViewCalled](QWidget *self) {
                        Q_UNUSED(self)
                        filterControlViewCalled = true;
                   });
    stub.set_lamda(&FilePropertyDialog::setBasicInfoExpand,
                   [&setBasicInfoExpandCalled](QWidget *self, bool expand) {
                        Q_UNUSED(self)
                        setBasicInfoExpandCalled = true;
                   });
    stub.set_lamda(&FilePropertyDialog::size,
                   [](QWidget *self) -> QSize {
                        Q_UNUSED(self)
                        return QSize(400, 300);
                   });
    stub.set_lamda(&FilePropertyDialog::initalHeightOfView,
                   [](QWidget *self) -> int {
                        Q_UNUSED(self)
                        return 250;
                   });
    
    // Fix overloaded function issue by using static_cast
    typedef void (QWidget::*MoveFunc)(const QPoint &);
    stub.set_lamda(static_cast<MoveFunc>(&FilePropertyDialog::move),
                   [&moveCalled](QWidget *self, const QPoint &pos) {
                        Q_UNUSED(self)
                        Q_UNUSED(pos)
                        moveCalled = true;
                   });
    
    stub.set_lamda(&FilePropertyDialog::show,
                   [&showCalled](QWidget *self) {
                        Q_UNUSED(self)
                        showCalled = true;
                   });
    stub.set_lamda(&FilePropertyDialog::activateWindow,
                   [](QWidget *self) {
                        Q_UNUSED(self)
                        // Mock implementation
                   });
    
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    util.showFilePropertyDialog(urls, option);
    
    EXPECT_TRUE(filterControlViewCalled);
    EXPECT_TRUE(setBasicInfoExpandCalled);
    EXPECT_TRUE(moveCalled);
    EXPECT_TRUE(showCalled);
}

TEST_F(TestPropertyDialogUtil, showCustomDialog_NotInCache)
{
    PropertyDialogUtil util;
    
    // Initially no custom dialog for this URL
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    
    bool createCustomizeViewCalled = false;
    stub.set_lamda(&PropertyDialogUtil::createCustomizeView,
                   [&createCustomizeViewCalled, &testUrl](PropertyDialogUtil *self, const QUrl &url) -> QWidget* {
                        Q_UNUSED(self)
                        createCustomizeViewCalled = (url == testUrl);
                        if (createCustomizeViewCalled) {
                            QWidget *mockWidget = new QWidget();
                            return mockWidget;
                        }
                        return nullptr;
                   });
    
    bool showCalled = false;
    bool activateWindowCalled = false;
    stub.set_lamda(&QWidget::show,
                   [&showCalled](QWidget *self) {
                        Q_UNUSED(self)
                        showCalled = true;
                   });
    stub.set_lamda(&QWidget::activateWindow,
                   [&activateWindowCalled](QWidget *self) {
                        Q_UNUSED(self)
                        activateWindowCalled = true;
                   });
    
    bool result = util.showCustomDialog(testUrl);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(createCustomizeViewCalled);
    EXPECT_TRUE(showCalled);
    EXPECT_TRUE(activateWindowCalled);
}

TEST_F(TestPropertyDialogUtil, closeFilePropertyDialog)
{
    PropertyDialogUtil util;
    
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    
    // This test checks that the function can be called without crashing
    EXPECT_NO_THROW(util.closeFilePropertyDialog(testUrl));
}

TEST_F(TestPropertyDialogUtil, closeCustomPropertyDialog)
{
    PropertyDialogUtil util;
    
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    
    // This test checks that the function can be called without crashing
    EXPECT_NO_THROW(util.closeCustomPropertyDialog(testUrl));
}

TEST_F(TestPropertyDialogUtil, closeAllFilePropertyDialog)
{
    PropertyDialogUtil util;
    
    bool closeAllDialogCloseCalled = false;
    bool closeIndicatorTimerStopCalled = false;
    
    // Fix the return type issue - QWidget::close returns bool
    stub.set_lamda(&QWidget::close,
                   [&closeAllDialogCloseCalled](QWidget *self) -> bool {
                        Q_UNUSED(self)
                        closeAllDialogCloseCalled = true;
                        return true;
                   });
    
    // Fix overloaded function issue by using static_cast
    typedef void (QTimer::*StopFunc)();
    stub.set_lamda(static_cast<StopFunc>(&QTimer::stop),
                   [&closeIndicatorTimerStopCalled](QTimer *self) {
                        Q_UNUSED(self)
                        closeIndicatorTimerStopCalled = true;
                   });
    
    util.closeAllFilePropertyDialog();
    
    EXPECT_TRUE(closeIndicatorTimerStopCalled);
    EXPECT_TRUE(closeAllDialogCloseCalled);
}

TEST_F(TestPropertyDialogUtil, createControlView)
{
    PropertyDialogUtil util;
    
    bool createViewCalled = false;
    stub.set_lamda(&PropertyDialogUtil::createView,
                   [&createViewCalled](PropertyDialogUtil *self, const QUrl &url, const QVariantHash &option) -> QMap<int, QWidget*> {
                        Q_UNUSED(self)
                        Q_UNUSED(url)
                        Q_UNUSED(option)
                        createViewCalled = true;
                        QMap<int, QWidget*> result;
                        result.insert(-1, new QWidget());
                        return result;
                   });
    
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    util.createControlView(testUrl, option);
    
    EXPECT_TRUE(createViewCalled);
}

TEST_F(TestPropertyDialogUtil, createView)
{
    PropertyDialogUtil util;
    
    bool createExtensionViewCalled = false;
    stub.set_lamda(&PropertyDialogManager::createExtensionView,
                   [&createExtensionViewCalled](PropertyDialogManager *self, const QUrl &url, const QVariantHash &option) -> QMap<int, QWidget*> {
                        Q_UNUSED(self)
                        Q_UNUSED(url)
                        Q_UNUSED(option)
                        createExtensionViewCalled = true;
                        return QMap<int, QWidget*>();
                   });
    
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    auto result = util.createView(testUrl, option);
    
    EXPECT_TRUE(createExtensionViewCalled);
}

TEST_F(TestPropertyDialogUtil, createCustomizeView)
{
    PropertyDialogUtil util;
    
    bool createCustomViewCalled = false;
    stub.set_lamda(&PropertyDialogManager::createCustomView,
                   [&createCustomViewCalled](PropertyDialogManager *self, const QUrl &url) -> QWidget* {
                        Q_UNUSED(self)
                        Q_UNUSED(url)
                        createCustomViewCalled = true;
                        return nullptr;
                   });
    
    QUrl testUrl = QUrl::fromLocalFile("/tmp/test");
    
    QWidget *result = util.createCustomizeView(testUrl);
    
    EXPECT_TRUE(createCustomViewCalled);
    EXPECT_EQ(result, nullptr);
}

TEST_F(TestPropertyDialogUtil, insertExtendedControlFileProperty)
{
    PropertyDialogUtil util;
    
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QWidget *widget = new QWidget();
    
    // This test checks that the function can be called without crashing
    EXPECT_NO_THROW(util.insertExtendedControlFileProperty(url, 0, widget));
    
    delete widget;
}

TEST_F(TestPropertyDialogUtil, addExtendedControlFileProperty)
{
    PropertyDialogUtil util;
    
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QWidget *widget = new QWidget();
    
    // This test checks that the function can be called without crashing
    EXPECT_NO_THROW(util.addExtendedControlFileProperty(url, widget));
    
    delete widget;
}

TEST_F(TestPropertyDialogUtil, closeAllPropertyDialog)
{
    PropertyDialogUtil util;
    
    // This test checks that the function can be called without crashing
    EXPECT_NO_THROW(util.closeAllPropertyDialog());
}

TEST_F(TestPropertyDialogUtil, updateCloseIndicator)
{
    PropertyDialogUtil util;
    
    // This test checks that the function can be called without crashing
    EXPECT_NO_THROW(util.updateCloseIndicator());
}

TEST_F(TestPropertyDialogUtil, createViewWithUrl)
{
    PropertyDialogUtil util;
    QUrl url = QUrl::fromLocalFile("/tmp/test");
    QVariantHash option;
    
    auto result = util.createView(url, option);
    EXPECT_TRUE(result.isEmpty()); // Should be empty since no extensions registered
}