// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QTimer>
#include <QApplication>

#include "stubext.h"
#include "dfmplugin_propertydialog_global.h"
#include "utils/propertydialogutil.h"
#include "utils/propertydialogmanager.h"
#include "views/filepropertydialog.h"
#include "views/closealldialog.h"

#include <dfm-framework/event/eventsequence.h>
#include <dfm-base/utils/windowutils.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

using namespace DPF_NAMESPACE;

class PropertyDialogUtilImpl : public testing::Test
{
protected:
    void SetUp() override { stub.clear(); }
    void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(PropertyDialogUtilImpl, Instance)
{
    PropertyDialogUtil *u1 = PropertyDialogUtil::instance();
    PropertyDialogUtil *u2 = PropertyDialogUtil::instance();
    EXPECT_NE(u1, nullptr);
    EXPECT_EQ(u1, u2);
}

TEST_F(PropertyDialogUtilImpl, ConstructorDestructor)
{
    PropertyDialogUtil *util = new PropertyDialogUtil();
    EXPECT_NE(util, nullptr);
    EXPECT_NE(util->closeIndicatorTimer, nullptr);
    EXPECT_EQ(util->closeIndicatorTimer->interval(), 1000);
    delete util;
}

TEST_F(PropertyDialogUtilImpl, ShowPropertyDialogSingle)
{
    PropertyDialogUtil util;

    // The product calls run(space, topic, url) with T deduced as QUrl (by
    // value); casting to a const QUrl& signature would instantiate a
    // *different* function and the stub would never take effect.
    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl);
    auto run = static_cast<RunFunc>(&EventSequenceManager::run);
    bool hookCalled = false;
    stub.set_lamda(run, [&hookCalled](EventSequenceManager *, const QString &, const QString &, QUrl) -> bool {
        hookCalled = true;
        return false;
    });

    bool customCalled = false;
    stub.set_lamda(&PropertyDialogUtil::showCustomDialog, [&customCalled](PropertyDialogUtil *, const QUrl &) -> bool {
        customCalled = true;
        return false;
    });

    bool fileCalled = false;
    stub.set_lamda(&PropertyDialogUtil::showFilePropertyDialog, [&fileCalled](PropertyDialogUtil *, const QList<QUrl> &, const QVariantHash &) {
        fileCalled = true;
    });

    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/ut-propertydialogutil-a") };
    util.showPropertyDialog(urls, QVariantHash());

    EXPECT_TRUE(hookCalled);
    EXPECT_TRUE(customCalled);
    EXPECT_TRUE(fileCalled);
}

TEST_F(PropertyDialogUtilImpl, ShowFilePropertyDialogSingle)
{
    PropertyDialogUtil util;

    stub.set_lamda(&FilePropertyDialog::selectFileUrl, [](FilePropertyDialog *, const QUrl &) {});
    stub.set_lamda(&FilePropertyDialog::filterControlView, [](FilePropertyDialog *) {});
    stub.set_lamda(&FilePropertyDialog::setBasicInfoExpand, [](FilePropertyDialog *, bool) {});
    stub.set_lamda(&FilePropertyDialog::size, [](QWidget *) -> QSize { return QSize(380, 400); });
    stub.set_lamda(&FilePropertyDialog::initalHeightOfView, [](FilePropertyDialog *) -> int { return 300; });
    stub.set_lamda(static_cast<void (FilePropertyDialog::*)(const QPoint &)>(&FilePropertyDialog::move),
                   [](QWidget *, const QPoint &) {});
    stub.set_lamda(&FilePropertyDialog::show, [](QWidget *) {});
    stub.set_lamda(&FilePropertyDialog::activateWindow, [](QWidget *) {});

    bool createViewCalled = false;
    stub.set_lamda(&PropertyDialogUtil::createView, [&createViewCalled](PropertyDialogUtil *, const QUrl &, const QVariantHash &) -> QMap<int, QWidget *> {
        createViewCalled = true;
        return {};
    });

    QList<QUrl> urls { QUrl::fromLocalFile("/tmp/ut-propertydialogutil-b") };
    QVariantHash option;
    option.insert(kOption_Key_BasicInfoExpand, true);
    util.showFilePropertyDialog(urls, option);

    EXPECT_TRUE(createViewCalled);
}

TEST_F(PropertyDialogUtilImpl, ShowFilePropertyDialogExisting)
{
    PropertyDialogUtil util;

    bool showCalled = false;
    bool activateCalled = false;
    stub.set_lamda(&FilePropertyDialog::show, [&showCalled](QWidget *) { showCalled = true; });
    stub.set_lamda(&FilePropertyDialog::activateWindow, [&activateCalled](QWidget *) { activateCalled = true; });
    stub.set_lamda(&FilePropertyDialog::selectFileUrl, [](FilePropertyDialog *, const QUrl &) {});
    stub.set_lamda(&FilePropertyDialog::filterControlView, [](FilePropertyDialog *) {});
    stub.set_lamda(&FilePropertyDialog::setBasicInfoExpand, [](FilePropertyDialog *, bool) {});
    stub.set_lamda(&FilePropertyDialog::size, [](QWidget *) -> QSize { return QSize(380, 400); });
    stub.set_lamda(&FilePropertyDialog::initalHeightOfView, [](FilePropertyDialog *) -> int { return 300; });
    stub.set_lamda(static_cast<void (FilePropertyDialog::*)(const QPoint &)>(&FilePropertyDialog::move),
                   [](QWidget *, const QPoint &) {});

    QUrl url = QUrl::fromLocalFile("/tmp/ut-propertydialogutil-c");
    util.showFilePropertyDialog({ url }, QVariantHash());
    showCalled = false;
    activateCalled = false;
    util.showFilePropertyDialog({ url }, QVariantHash());

    EXPECT_TRUE(showCalled);
    EXPECT_TRUE(activateCalled);
}

TEST_F(PropertyDialogUtilImpl, InsertExtendedControlFileProperty)
{
    PropertyDialogUtil util;

    QUrl url = QUrl::fromLocalFile("/tmp/ut-propertydialogutil-d");
    QWidget *w1 = new QWidget();
    QWidget *w2 = new QWidget();
    QWidget *w3 = new QWidget();

    EXPECT_NO_THROW(util.insertExtendedControlFileProperty(url, 0, w1));
    EXPECT_NO_THROW(util.addExtendedControlFileProperty(url, w2));
    EXPECT_NO_THROW(util.addExtendedControlFileProperty(url, w3, [](QWidget *, const QUrl &) {}));

    util.closeAllFilePropertyDialog();
}

TEST_F(PropertyDialogUtilImpl, RenameFilePropertyDialog)
{
    PropertyDialogUtil util;

    stub.set_lamda(&FilePropertyDialog::selectFileUrl, [](FilePropertyDialog *, const QUrl &) {});
    stub.set_lamda(&FilePropertyDialog::filterControlView, [](FilePropertyDialog *) {});
    stub.set_lamda(&FilePropertyDialog::setBasicInfoExpand, [](FilePropertyDialog *, bool) {});
    stub.set_lamda(&FilePropertyDialog::size, [](QWidget *) -> QSize { return QSize(380, 400); });
    stub.set_lamda(&FilePropertyDialog::initalHeightOfView, [](FilePropertyDialog *) -> int { return 300; });
    stub.set_lamda(static_cast<void (FilePropertyDialog::*)(const QPoint &)>(&FilePropertyDialog::move),
                   [](QWidget *, const QPoint &) {});
    stub.set_lamda(&FilePropertyDialog::show, [](QWidget *) {});
    stub.set_lamda(&FilePropertyDialog::activateWindow, [](QWidget *) {});
    stub.set_lamda(&PropertyDialogUtil::createView, [](PropertyDialogUtil *, const QUrl &, const QVariantHash &) -> QMap<int, QWidget *> {
        return {};
    });

    QUrl oldUrl = QUrl::fromLocalFile("/tmp/ut-propertydialogutil-old");
    QUrl newUrl = QUrl::fromLocalFile("/tmp/ut-propertydialogutil-new");
    util.showFilePropertyDialog({ oldUrl }, QVariantHash());
    util.renameFilePropertyDialog(oldUrl, newUrl);

    EXPECT_NO_THROW(util.closeFilePropertyDialog(newUrl));
}

TEST_F(PropertyDialogUtilImpl, CloseCustomPropertyDialog)
{
    PropertyDialogUtil util;

    QUrl url = QUrl::fromLocalFile("/tmp/ut-propertydialogutil-custom");
    QWidget *widget = new QWidget();
    widget->setProperty("ForecastDisplayHeight", 200);

    stub.set_lamda(&PropertyDialogUtil::createCustomizeView, [widget](PropertyDialogUtil *, const QUrl &) -> QWidget * { return widget; });
    stub.set_lamda(&QWidget::show, [](QWidget *) {});
    stub.set_lamda(&QWidget::activateWindow, [](QWidget *) {});
    stub.set_lamda(static_cast<void (QWidget::*)(const QPoint &)>(&QWidget::move),
                   [](QWidget *, const QPoint &) {});

    EXPECT_TRUE(util.showCustomDialog(url));
    EXPECT_NO_THROW(util.closeCustomPropertyDialog(url));
    delete widget;
}

TEST_F(PropertyDialogUtilImpl, CloseAllFilePropertyDialog)
{
    PropertyDialogUtil util;

    stub.set_lamda(&FilePropertyDialog::selectFileUrl, [](FilePropertyDialog *, const QUrl &) {});
    stub.set_lamda(&FilePropertyDialog::filterControlView, [](FilePropertyDialog *) {});
    stub.set_lamda(&FilePropertyDialog::setBasicInfoExpand, [](FilePropertyDialog *, bool) {});
    stub.set_lamda(&FilePropertyDialog::size, [](QWidget *) -> QSize { return QSize(380, 400); });
    stub.set_lamda(&FilePropertyDialog::initalHeightOfView, [](FilePropertyDialog *) -> int { return 300; });
    stub.set_lamda(static_cast<void (FilePropertyDialog::*)(const QPoint &)>(&FilePropertyDialog::move),
                   [](QWidget *, const QPoint &) {});
    stub.set_lamda(&FilePropertyDialog::show, [](QWidget *) {});
    stub.set_lamda(&FilePropertyDialog::activateWindow, [](QWidget *) {});
    stub.set_lamda(&PropertyDialogUtil::createView, [](PropertyDialogUtil *, const QUrl &, const QVariantHash &) -> QMap<int, QWidget *> {
        return {};
    });

    util.showFilePropertyDialog({ QUrl::fromLocalFile("/tmp/ut-propertydialogutil-e1") }, QVariantHash());
    util.showFilePropertyDialog({ QUrl::fromLocalFile("/tmp/ut-propertydialogutil-e2") }, QVariantHash());
    EXPECT_NO_THROW(util.closeAllFilePropertyDialog());
}

TEST_F(PropertyDialogUtilImpl, UpdateCloseIndicator)
{
    PropertyDialogUtil util;

    stub.set_lamda(&FilePropertyDialog::selectFileUrl, [](FilePropertyDialog *, const QUrl &) {});
    stub.set_lamda(&FilePropertyDialog::filterControlView, [](FilePropertyDialog *) {});
    stub.set_lamda(&FilePropertyDialog::setBasicInfoExpand, [](FilePropertyDialog *, bool) {});
    stub.set_lamda(&FilePropertyDialog::size, [](QWidget *) -> QSize { return QSize(380, 400); });
    stub.set_lamda(&FilePropertyDialog::initalHeightOfView, [](FilePropertyDialog *) -> int { return 300; });
    stub.set_lamda(static_cast<void (FilePropertyDialog::*)(const QPoint &)>(&FilePropertyDialog::move),
                   [](QWidget *, const QPoint &) {});
    stub.set_lamda(&FilePropertyDialog::show, [](QWidget *) {});
    stub.set_lamda(&FilePropertyDialog::activateWindow, [](QWidget *) {});
    stub.set_lamda(&PropertyDialogUtil::createView, [](PropertyDialogUtil *, const QUrl &, const QVariantHash &) -> QMap<int, QWidget *> {
        return {};
    });

    stub.set_lamda(&FilePropertyDialog::getFileSize, [](FilePropertyDialog *) -> qint64 { return 100; });
    stub.set_lamda(&FilePropertyDialog::getFileCount, [](FilePropertyDialog *) -> int { return 2; });

    bool setTotal = false;
    stub.set_lamda(&CloseAllDialog::setTotalMessage, [&setTotal](CloseAllDialog *, qint64, int) { setTotal = true; });

    util.showFilePropertyDialog({ QUrl::fromLocalFile("/tmp/ut-propertydialogutil-f") }, QVariantHash());
    util.updateCloseIndicator();
    EXPECT_TRUE(setTotal);
}
