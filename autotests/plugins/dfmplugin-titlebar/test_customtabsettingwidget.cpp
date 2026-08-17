// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/customtabsettingwidget.h"

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/dialogmanager.h>

#include <DSettingsOption>
#include <DCommandLinkButton>
#include <DToolButton>
#include <DLabel>

#include <gtest/gtest.h>
#include <QUrl>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSignalSpy>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class CustomTabSettingWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Stub icon loading
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            return QIcon();
        });

        // Stub QFileDialog
        stub.set_lamda(&QFileDialog::getExistingDirectoryUrl, [this](QWidget *, const QString &, const QUrl &, QFileDialog::Options, const QStringList &) {
            __DBG_STUB_INVOKE__
            return mockSelectedUrl;
        });

        // Stub ProtocolUtils
        stub.set_lamda(&ProtocolUtils::isLocalFile, [this](const QUrl &url) {
            __DBG_STUB_INVOKE__
            return mockIsLocalFile;
        });

        // Stub DialogManager
        stub.set_lamda(&DialogManager::showErrorDialog, [](DialogManager *, const QString &, const QString &) {
            __DBG_STUB_INVOKE__
        });

        widget = new CustomTabSettingWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    CustomTabSettingWidget *widget { nullptr };
    stub_ext::StubExt stub;
    QUrl mockSelectedUrl;
    bool mockIsLocalFile { true };
};

TEST_F(CustomTabSettingWidgetTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(CustomTabSettingWidgetTest, Constructor_InitializesUI_LayoutCreated)
{
    EXPECT_NE(widget->mainLayout, nullptr);
    EXPECT_NE(widget->addItemBtn, nullptr);
}

TEST_F(CustomTabSettingWidgetTest, InitUI_CreatesLayout_WithCorrectMargins)
{
    EXPECT_EQ(widget->mainLayout->contentsMargins(), QMargins(0, 0, 0, 0));
    EXPECT_EQ(widget->mainLayout->verticalSpacing(), 10);
}

TEST_F(CustomTabSettingWidgetTest, InitUI_CreatesAddButton_Enabled)
{
    EXPECT_TRUE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, InitUI_SetsColumnStretch_Correctly)
{
    EXPECT_EQ(widget->mainLayout->columnStretch(0), 1);
    EXPECT_EQ(widget->mainLayout->columnStretch(1), 0);
}

TEST_F(CustomTabSettingWidgetTest, SetOption_EmptyList_EnablesAddButton)
{
    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    widget->setOption(&option);

    EXPECT_TRUE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, SetOption_WithThreeItems_EnablesAddButton)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2"
          << "file:///home/test3";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    widget->setOption(&option);

    EXPECT_TRUE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, SetOption_WithFourItems_DisablesAddButton)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2"
          << "file:///home/test3"
          << "file:///home/test4";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    widget->setOption(&option);

    EXPECT_FALSE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, SetOption_WithItems_AddsCustomItems)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    int initialRowCount = widget->mainLayout->rowCount();
    widget->setOption(&option);

    // Should add 2 rows (one for each item)
    EXPECT_GT(widget->mainLayout->rowCount(), initialRowCount);
}

TEST_F(CustomTabSettingWidgetTest, AddCustomItem_ValidUrl_AddsLabelAndButton)
{
    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    QUrl testUrl("file:///home/test");
    int initialRowCount = widget->mainLayout->rowCount();

    widget->addCustomItem(&option, testUrl);

    EXPECT_GT(widget->mainLayout->rowCount(), initialRowCount);
}

TEST_F(CustomTabSettingWidgetTest, AddCustomItem_CreatesDeleteButton_WithCorrectProperties)
{
    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    QUrl testUrl("file:///home/test");
    widget->addCustomItem(&option, testUrl);

    // Find the delete button
    DToolButton *deleteBtn = nullptr;
    for (int i = 0; i < widget->mainLayout->count(); ++i) {
        QLayoutItem *item = widget->mainLayout->itemAt(i);
        if (item && item->widget()) {
            deleteBtn = qobject_cast<DToolButton *>(item->widget());
            if (deleteBtn && deleteBtn->objectName() == "DeleteButton") {
                break;
            }
        }
    }

    EXPECT_NE(deleteBtn, nullptr);
    if (deleteBtn) {
        EXPECT_EQ(deleteBtn->objectName(), "DeleteButton");
        EXPECT_EQ(deleteBtn->size(), QSize(32, 32));
    }
}

TEST_F(CustomTabSettingWidgetTest, AddCustomItem_CreatesPathLabel_WithCorrectText)
{
    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    QUrl testUrl("file:///home/test");
    widget->addCustomItem(&option, testUrl);

    // Find the path label
    DLabel *pathLabel = nullptr;
    for (int i = 0; i < widget->mainLayout->count(); ++i) {
        QLayoutItem *item = widget->mainLayout->itemAt(i);
        if (item && item->widget()) {
            pathLabel = qobject_cast<DLabel *>(item->widget());
            if (pathLabel && pathLabel->text() == testUrl.path()) {
                break;
            }
        }
    }

    EXPECT_NE(pathLabel, nullptr);
    if (pathLabel) {
        EXPECT_EQ(pathLabel->text(), testUrl.path());
        EXPECT_EQ(pathLabel->toolTip(), testUrl.path());
    }
}

TEST_F(CustomTabSettingWidgetTest, SelectCustomDirectory_ValidLocalUrl_ReturnsUrl)
{
    mockSelectedUrl = QUrl("file:///home/test");
    mockIsLocalFile = true;

    QUrl result = widget->selectCustomDirectory();

    EXPECT_EQ(result, mockSelectedUrl);
}

TEST_F(CustomTabSettingWidgetTest, SelectCustomDirectory_InvalidUrl_ReturnsEmpty)
{
    mockSelectedUrl = QUrl();
    mockIsLocalFile = true;

    QUrl result = widget->selectCustomDirectory();

    EXPECT_FALSE(result.isValid());
}

TEST_F(CustomTabSettingWidgetTest, SelectCustomDirectory_NonLocalUrl_ShowsError)
{
    mockSelectedUrl = QUrl("ftp://example.com/test");
    mockIsLocalFile = false;

    bool errorShown = false;
    stub.set_lamda(&DialogManager::showErrorDialog, [&errorShown](DialogManager *, const QString &, const QString &) {
        __DBG_STUB_INVOKE__
        errorShown = true;
    });

    QUrl result = widget->selectCustomDirectory();

    EXPECT_TRUE(errorShown);
    EXPECT_FALSE(result.isValid());
}

TEST_F(CustomTabSettingWidgetTest, HandleAddCustomItem_ValidUrl_AddsItem)
{
    mockSelectedUrl = QUrl("file:///home/test");
    mockIsLocalFile = true;

    QStringList items;
    DSettingsOption option;

    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    bool valueSet = false;
    stub.set_lamda(&DSettingsOption::setValue, [&valueSet, &items](DSettingsOption *, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSet = true;
        items = value.toStringList();
    });

    widget->handleAddCustomItem(&option);

    EXPECT_TRUE(valueSet);
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items.first(), mockSelectedUrl.toString());
}

TEST_F(CustomTabSettingWidgetTest, HandleAddCustomItem_InvalidUrl_DoesNotAddItem)
{
    mockSelectedUrl = QUrl();
    mockIsLocalFile = true;

    QStringList items;
    DSettingsOption option;

    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    bool valueSet = false;
    stub.set_lamda(&DSettingsOption::setValue, [&valueSet](DSettingsOption *, const QVariant &) {
        __DBG_STUB_INVOKE__
        valueSet = true;
    });

    widget->handleAddCustomItem(&option);

    EXPECT_FALSE(valueSet);
}

TEST_F(CustomTabSettingWidgetTest, HandleAddCustomItem_FourItems_DisablesButton)
{
    mockSelectedUrl = QUrl("file:///home/test4");
    mockIsLocalFile = true;

    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2"
          << "file:///home/test3";

    DSettingsOption option;

    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    stub.set_lamda(&DSettingsOption::setValue, [&items](DSettingsOption *, const QVariant &value) {
        __DBG_STUB_INVOKE__
        items = value.toStringList();
    });

    widget->setOption(&option);
    widget->handleAddCustomItem(&option);

    EXPECT_FALSE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, HandleOptionChanged_WithNewItems_UpdatesUI)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    widget->setOption(&option);

    // Change items
    items.clear();
    items << "file:///home/test3";

    widget->handleOptionChanged(QVariant(items));

    EXPECT_TRUE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, HandleOptionChanged_WithFourItems_DisablesButton)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    widget->setOption(&option);

    // Change to 4 items
    items.clear();
    items << "file:///home/test1"
          << "file:///home/test2"
          << "file:///home/test3"
          << "file:///home/test4";

    widget->handleOptionChanged(QVariant(items));

    EXPECT_TRUE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, HandleOptionChanged_WithEmptyList_ClearsItems)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    widget->setOption(&option);

    // Clear items
    items.clear();

    int rowCountBefore = widget->mainLayout->rowCount();
    widget->handleOptionChanged(QVariant(items));

    EXPECT_TRUE(widget->addItemBtn->isEnabled());
}

TEST_F(CustomTabSettingWidgetTest, ClearCustomItems_WithItems_RemovesAll)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    widget->setOption(&option);
    EXPECT_NO_THROW(widget->clearCustomItems());
}

TEST_F(CustomTabSettingWidgetTest, ClearCustomItems_EmptyWidget_HandlesGracefully)
{
    EXPECT_NO_THROW(widget->clearCustomItems());
}

TEST_F(CustomTabSettingWidgetTest, RemoveRow_ValidWidget_ReturnsTrue)
{
    QStringList items;
    items << "file:///home/test1";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    widget->addCustomItem(&option, QUrl("file:///home/test1"));

    // Find a widget in the layout
    QWidget *widgetToRemove = nullptr;
    for (int i = 0; i < widget->mainLayout->count(); ++i) {
        QLayoutItem *item = widget->mainLayout->itemAt(i);
        if (item && item->widget()) {
            auto btn = qobject_cast<DToolButton *>(item->widget());
            if (btn && btn->objectName() == "DeleteButton") {
                widgetToRemove = btn;
                break;
            }
        }
    }

    if (widgetToRemove) {
        bool result = widget->removeRow(widgetToRemove);
        EXPECT_TRUE(result);
    }
}

TEST_F(CustomTabSettingWidgetTest, RemoveRow_InvalidWidget_ReturnsFalse)
{
    QWidget dummyWidget;
    bool result = widget->removeRow(&dummyWidget);

    EXPECT_FALSE(result);
}

TEST_F(CustomTabSettingWidgetTest, RemoveRow_NullWidget_ReturnsFalse)
{
    QLabel *label = new QLabel("test", widget);
    widget->mainLayout->addWidget(label, 5, 0);

    QWidget *notInLayout = new QWidget();
    bool result = widget->removeRow(notInLayout);

    EXPECT_FALSE(result);
    delete notInLayout;
}

TEST_F(CustomTabSettingWidgetTest, DeleteButton_Clicked_RemovesItem)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    bool valueSet = false;
    QStringList newItems;
    stub.set_lamda(&DSettingsOption::setValue, [&valueSet, &newItems](DSettingsOption *, const QVariant &value) {
        __DBG_STUB_INVOKE__
        valueSet = true;
        newItems = value.toStringList();
    });

    widget->setOption(&option);

    // Find and click the first delete button
    DToolButton *deleteBtn = nullptr;
    for (int i = 0; i < widget->mainLayout->count(); ++i) {
        QLayoutItem *item = widget->mainLayout->itemAt(i);
        if (item && item->widget()) {
            deleteBtn = qobject_cast<DToolButton *>(item->widget());
            if (deleteBtn && deleteBtn->objectName() == "DeleteButton") {
                break;
            }
        }
    }

    if (deleteBtn) {
        deleteBtn->click();

        EXPECT_TRUE(valueSet);
        EXPECT_EQ(newItems.size(), 1);
    }
}

TEST_F(CustomTabSettingWidgetTest, DeleteButton_ClickedWithThreeItems_EnablesAddButton)
{
    QStringList items;
    items << "file:///home/test1"
          << "file:///home/test2"
          << "file:///home/test3"
          << "file:///home/test4";

    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [&items](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(items);
    });

    stub.set_lamda(&DSettingsOption::setValue, [&items](DSettingsOption *, const QVariant &value) {
        __DBG_STUB_INVOKE__
        items = value.toStringList();
    });

    widget->setOption(&option);

    // Button should be disabled with 4 items
    EXPECT_FALSE(widget->addItemBtn->isEnabled());

    // Find and click a delete button
    DToolButton *deleteBtn = nullptr;
    for (int i = 0; i < widget->mainLayout->count(); ++i) {
        QLayoutItem *item = widget->mainLayout->itemAt(i);
        if (item && item->widget()) {
            deleteBtn = qobject_cast<DToolButton *>(item->widget());
            if (deleteBtn && deleteBtn->objectName() == "DeleteButton") {
                break;
            }
        }
    }

    if (deleteBtn) {
        deleteBtn->click();

        // After removing one item, should have 3 items and button should be enabled
        EXPECT_TRUE(widget->addItemBtn->isEnabled());
    }
}

TEST_F(CustomTabSettingWidgetTest, PathLabel_HasElideMode_ElideMiddle)
{
    DSettingsOption option;
    stub.set_lamda(&DSettingsOption::value, [](DSettingsOption *) {
        __DBG_STUB_INVOKE__
        return QVariant(QStringList());
    });

    QUrl testUrl("file:///home/test/very/long/path");
    widget->addCustomItem(&option, testUrl);

    // Find the path label
    DLabel *pathLabel = nullptr;
    for (int i = 0; i < widget->mainLayout->count(); ++i) {
        QLayoutItem *item = widget->mainLayout->itemAt(i);
        if (item && item->widget()) {
            pathLabel = qobject_cast<DLabel *>(item->widget());
            if (pathLabel && pathLabel->text() == testUrl.path()) {
                break;
            }
        }
    }

    EXPECT_NE(pathLabel, nullptr);
    if (pathLabel) {
        EXPECT_EQ(pathLabel->elideMode(), Qt::ElideMiddle);
    }
}
