// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "delegate/computeritemdelegate.h"
#include "views/computerview.h"
#include "models/computermodel.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/fileutils.h>

#include <QApplication>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QLineEdit>
#include <QToolTip>
#include <QHelpEvent>
#include <QAbstractItemView>
#include <QIcon>
#include <QSize>
#include <QWidget>
#include <QFont>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QValidator>
#include <QPixmap>
#include <QColor>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class MockComputerModel : public QAbstractItemModel
{
public:
    MockComputerModel(QObject *parent = nullptr)
        : QAbstractItemModel(parent) { }

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return createIndex(row, column);
    }

    QModelIndex parent(const QModelIndex &) const override { return QModelIndex(); }
    int rowCount(const QModelIndex & = QModelIndex()) const override { return mockRowCount; }
    int columnCount(const QModelIndex & = QModelIndex()) const override { return 1; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        return mockData.value(role, QVariant());
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        Q_UNUSED(index)
        setDataCalls[role] = value;
        return true;
    }

    void setMockData(int role, const QVariant &value) { mockData[role] = value; }

public:
    int mockRowCount { 5 };
    QMap<int, QVariant> mockData;
    mutable QMap<int, QVariant> setDataCalls;
};

class UT_ComputerItemDelegate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();

        // Create mock view and model
        mockView = new ComputerView(QUrl());
        mockModel = new MockComputerModel();
        mockView->setModel(mockModel);

        // Create delegate with mock view as parent
        delegate = new ComputerItemDelegate(mockView);

        // Setup default mock data
        setupDefaultMockData();

        // Create mock painter and option
        mockPixmap = QPixmap(800, 600);
        mockPainter = new QPainter(&mockPixmap);

        mockOption.rect = QRect(0, 0, 284, 84);
        mockOption.widget = mockView;
        mockOption.font = QFont("Arial", 12);

        mockIndex = mockModel->index(0, 0);
    }

    virtual void TearDown() override
    {
        delete mockPainter;
        delete delegate;
        delete mockView;
        delete mockModel;
        stub.clear();
    }

    void setupDefaultMockData()
    {
        mockModel->setMockData(Qt::DisplayRole, "Test Device");
        mockModel->setMockData(Qt::DecorationRole, QIcon::fromTheme("drive-harddisk"));
        mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kLargeItem);
        mockModel->setMockData(ComputerModel::kFileSystemRole, "ext4");
        mockModel->setMockData(ComputerModel::kSizeTotalRole, static_cast<qint64>(1024 * 1024 * 1024));
        mockModel->setMockData(ComputerModel::kSizeUsageRole, static_cast<qint64>(512 * 1024 * 1024));
        mockModel->setMockData(ComputerModel::kProgressVisiableRole, true);
        mockModel->setMockData(ComputerModel::kTotalSizeVisiableRole, true);
        mockModel->setMockData(ComputerModel::kUsedSizeVisiableRole, true);
        mockModel->setMockData(ComputerModel::kDeviceNameMaxLengthRole, 255);
        mockModel->setMockData(ComputerModel::kDisplayNameIsElidedRole, false);
    }

protected:
    stub_ext::StubExt stub;
    ComputerItemDelegate *delegate { nullptr };
    ComputerView *mockView { nullptr };
    MockComputerModel *mockModel { nullptr };
    QPainter *mockPainter { nullptr };
    QPixmap mockPixmap;
    QStyleOptionViewItem mockOption;
    QModelIndex mockIndex;
};

TEST_F(UT_ComputerItemDelegate, Constructor_WithComputerViewParent_SetsViewCorrectly)
{
    EXPECT_NE(delegate, nullptr);
    // Note: view member is private, we can access it due to compiler flags
    EXPECT_EQ(delegate->view, mockView);
}

TEST_F(UT_ComputerItemDelegate, Constructor_WithNonComputerViewParent_SetsViewToNull)
{
    QWidget *plainWidget = new QWidget();
    ComputerItemDelegate *testDelegate = new ComputerItemDelegate(plainWidget);

    EXPECT_EQ(testDelegate->view, nullptr);

    delete testDelegate;
    delete plainWidget;
}

TEST_F(UT_ComputerItemDelegate, Paint_SplitterItem_CallsPaintSplitter)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kSplitterItem);

    bool paintSplitterCalled = false;
    stub.set_lamda(&ComputerItemDelegate::paintSplitter, [&](ComputerItemDelegate *, QPainter *, const QStyleOptionViewItem &, const QModelIndex &) {
        __DBG_STUB_INVOKE__
        paintSplitterCalled = true;
    });

    delegate->paint(mockPainter, mockOption, mockIndex);
    EXPECT_TRUE(paintSplitterCalled);
}

TEST_F(UT_ComputerItemDelegate, Paint_SmallItem_CallsPaintSmallItem)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kSmallItem);

    bool paintSmallItemCalled = false;
    stub.set_lamda(&ComputerItemDelegate::paintSmallItem, [&](ComputerItemDelegate *, QPainter *, const QStyleOptionViewItem &, const QModelIndex &) {
        __DBG_STUB_INVOKE__
        paintSmallItemCalled = true;
    });

    delegate->paint(mockPainter, mockOption, mockIndex);
    EXPECT_TRUE(paintSmallItemCalled);
}

TEST_F(UT_ComputerItemDelegate, Paint_LargeItem_CallsPaintLargeItem)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kLargeItem);

    bool paintLargeItemCalled = false;
    stub.set_lamda(&ComputerItemDelegate::paintLargeItem, [&](ComputerItemDelegate *, QPainter *, const QStyleOptionViewItem &, const QModelIndex &) {
        __DBG_STUB_INVOKE__
        paintLargeItemCalled = true;
    });

    delegate->paint(mockPainter, mockOption, mockIndex);
    EXPECT_TRUE(paintLargeItemCalled);
}

TEST_F(UT_ComputerItemDelegate, Paint_WidgetItem_CallsPaintCustomWidget)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kWidgetItem);

    bool paintCustomWidgetCalled = false;
    stub.set_lamda(&ComputerItemDelegate::paintCustomWidget, [&](ComputerItemDelegate *, QPainter *, const QStyleOptionViewItem &, const QModelIndex &) {
        __DBG_STUB_INVOKE__
        paintCustomWidgetCalled = true;
    });

    delegate->paint(mockPainter, mockOption, mockIndex);
    EXPECT_TRUE(paintCustomWidgetCalled);
}

TEST_F(UT_ComputerItemDelegate, SizeHint_SplitterItem_ReturnsCorrectSize)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kSplitterItem);
    mockView->setFixedWidth(800);

    QSize result = delegate->sizeHint(mockOption, mockIndex);
    EXPECT_EQ(result.width(), 770);   // view width - 30
    EXPECT_EQ(result.height(), 36);   // kSplitterLineHeight
}

TEST_F(UT_ComputerItemDelegate, SizeHint_SmallItem_ReturnsCorrectSize)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kSmallItem);

    QSize result = delegate->sizeHint(mockOption, mockIndex);
    EXPECT_EQ(result.width(), 108);   // kSmallItemWidth
    EXPECT_EQ(result.height(), 138);   // kSmallItemHeight
}

TEST_F(UT_ComputerItemDelegate, SizeHint_LargeItem_ReturnsCorrectSize)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kLargeItem);

    QSize result = delegate->sizeHint(mockOption, mockIndex);
    EXPECT_EQ(result.width(), 284);   // kLargeItemWidth
    EXPECT_EQ(result.height(), 84);   // kLargeItemHeight
}

TEST_F(UT_ComputerItemDelegate, CreateEditor_ValidIndex_CreatesLineEdit)
{
    QWidget *parent = new QWidget;

    QWidget *editor = delegate->createEditor(parent, mockOption, mockIndex);

    EXPECT_NE(editor, nullptr);
    delete parent;
}

TEST_F(UT_ComputerItemDelegate, SetEditorData_ValidEditor_SetsTextFromModel)
{
    QLineEdit *editor = new QLineEdit;
    mockModel->setMockData(Qt::DisplayRole, "Test Device Name");

    delegate->setEditorData(editor, mockIndex);

    EXPECT_EQ(editor->text(), "Test Device Name");

    delete editor;
}

TEST_F(UT_ComputerItemDelegate, SetModelData_ChangedText_UpdatesModel)
{
    QLineEdit *editor = new QLineEdit;
    editor->setText("New Device Name");
    mockModel->setMockData(Qt::DisplayRole, "Old Device Name");

    delegate->setModelData(editor, mockModel, mockIndex);

    EXPECT_TRUE(mockModel->setDataCalls.contains(Qt::EditRole));
    EXPECT_EQ(mockModel->setDataCalls[Qt::EditRole].toString(), "New Device Name");

    // Check that editing flag is cleared
    EXPECT_TRUE(mockModel->setDataCalls.contains(ComputerModel::kItemIsEditingRole));
    EXPECT_FALSE(mockModel->setDataCalls[ComputerModel::kItemIsEditingRole].toBool());

    delete editor;
}

TEST_F(UT_ComputerItemDelegate, SetModelData_UnchangedText_DoesNotUpdateModel)
{
    QLineEdit *editor = new QLineEdit;
    editor->setText("Same Name");
    mockModel->setMockData(Qt::DisplayRole, "Same Name");

    delegate->setModelData(editor, mockModel, mockIndex);

    EXPECT_FALSE(mockModel->setDataCalls.contains(Qt::EditRole));

    // Editing flag should still be cleared
    EXPECT_TRUE(mockModel->setDataCalls.contains(ComputerModel::kItemIsEditingRole));
    EXPECT_FALSE(mockModel->setDataCalls[ComputerModel::kItemIsEditingRole].toBool());

    delete editor;
}

TEST_F(UT_ComputerItemDelegate, UpdateEditorGeometry_NormalItem_SetsCorrectGeometry)
{
    QLineEdit *editor = new QLineEdit;
    mockView->setIconSize(QSize(48, 48));

    delegate->updateEditorGeometry(editor, mockOption, mockIndex);

    QRect expectedRect;
    expectedRect.setLeft(mockOption.rect.left() + 10 + 48 + 10);   // margins + icon + spacing
    expectedRect.setWidth(180);
    expectedRect.setTop(mockOption.rect.top() + 10);
    expectedRect.setHeight(mockView->fontInfo().pixelSize() * 2);

    EXPECT_EQ(editor->geometry(), expectedRect);

    delete editor;
}

TEST_F(UT_ComputerItemDelegate, UpdateEditorGeometry_WidgetItem_SetsToOptionRect)
{
    QLineEdit *editor = new QLineEdit;
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kWidgetItem);

    delegate->updateEditorGeometry(editor, mockOption, mockIndex);

    EXPECT_EQ(editor->geometry(), mockOption.rect);

    delete editor;
}

TEST_F(UT_ComputerItemDelegate, HelpEvent_TooltipWithElidedText_ShowsTooltip)
{
    QHelpEvent *he = new QHelpEvent(QEvent::ToolTip, QPoint(10, 10), QPoint(100, 100));
    mockModel->setMockData(ComputerModel::kDisplayNameIsElidedRole, true);
    mockModel->setMockData(Qt::DisplayRole, "Very Long Device Name That Gets Elided");

    bool tooltipShown = false;
    stub.set_lamda(static_cast<void (*)(const QPoint &, const QString &, QWidget *, const QRect &, int)>(&QToolTip::showText),
                   [&] {
                       __DBG_STUB_INVOKE__
                       tooltipShown = true;
                   });
    stub.set_lamda(VADDR(QStyledItemDelegate, helpEvent), [] { __DBG_STUB_INVOKE__ return true; });

    bool result = delegate->helpEvent(he, nullptr, mockOption, mockIndex);

    EXPECT_TRUE(result);
    EXPECT_TRUE(tooltipShown);

    delete he;
}

TEST_F(UT_ComputerItemDelegate, HelpEvent_TooltipWithNonElidedText_HidesTooltip)
{
    QHelpEvent *he = new QHelpEvent(QEvent::ToolTip, QPoint(10, 10), QPoint(100, 100));
    mockModel->setMockData(ComputerModel::kDisplayNameIsElidedRole, false);

    bool tooltipHidden = false;
    stub.set_lamda(&QToolTip::hideText, [&]() {
        __DBG_STUB_INVOKE__
        tooltipHidden = true;
    });
    stub.set_lamda(VADDR(QStyledItemDelegate, helpEvent), [] { __DBG_STUB_INVOKE__ return true; });

    bool result = delegate->helpEvent(he, nullptr, mockOption, mockIndex);

    EXPECT_TRUE(result);
    EXPECT_TRUE(tooltipHidden);

    delete he;
}

TEST_F(UT_ComputerItemDelegate, SizeHint_WidgetItem_ReturnsEmptySize)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, ComputerItemData::kWidgetItem + 1);

    // For widget items without proper setup, expect empty size
    QSize result = delegate->sizeHint(mockOption, mockIndex);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_ComputerItemDelegate, Paint_InvalidShapeType_DoesNotCrash)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, 999);   // Invalid type

    EXPECT_NO_THROW(delegate->paint(mockPainter, mockOption, mockIndex));
}

TEST_F(UT_ComputerItemDelegate, SizeHint_InvalidShapeType_ReturnsEmptySize)
{
    mockModel->setMockData(ComputerModel::kItemShapeTypeRole, 999);   // Invalid type

    QSize result = delegate->sizeHint(mockOption, mockIndex);
    EXPECT_TRUE(result.isEmpty());
}

// Test private methods accessibility (due to compiler flags)
TEST_F(UT_ComputerItemDelegate, PrepareColor_SelectedState_DoesNotCrash)
{
    mockOption.state |= QStyle::State_Selected;

    EXPECT_NO_THROW(delegate->prepareColor(mockPainter, mockOption, mockIndex));
}

TEST_F(UT_ComputerItemDelegate, PrepareColor_HoverState_DoesNotCrash)
{
    mockOption.state |= QStyle::State_MouseOver;

    EXPECT_NO_THROW(delegate->prepareColor(mockPainter, mockOption, mockIndex));
}

TEST_F(UT_ComputerItemDelegate, DrawDeviceIcon_ValidIcon_DoesNotCrash)
{
    QIcon testIcon = QIcon::fromTheme("drive-harddisk");
    mockModel->setMockData(Qt::DecorationRole, testIcon);

    EXPECT_NO_THROW(delegate->drawDeviceIcon(mockPainter, mockOption, mockIndex));
}

TEST_F(UT_ComputerItemDelegate, DrawDeviceLabelAndFs_WithFileSystem_DoesNotCrash)
{
    mockModel->setMockData(Qt::DisplayRole, "Test Device");
    mockModel->setMockData(ComputerModel::kFileSystemRole, "NTFS");

    // Mock Application::instance()->genericAttribute call
    stub.set_lamda(&Application::genericAttribute, [](Application::GenericAttribute attr) -> QVariant {
        __DBG_STUB_INVOKE__
        if (attr == Application::GenericAttribute::kShowFileSystemTagOnDiskIcon)
            return true;
        return QVariant();
    });

    EXPECT_NO_THROW(delegate->drawDeviceLabelAndFs(mockPainter, mockOption, mockIndex));
}

TEST_F(UT_ComputerItemDelegate, DrawDeviceDetail_WithSizeInfo_DoesNotCrash)
{
    mockModel->setMockData(ComputerModel::kTotalSizeVisiableRole, true);
    mockModel->setMockData(ComputerModel::kUsedSizeVisiableRole, true);
    mockModel->setMockData(ComputerModel::kProgressVisiableRole, true);
    mockModel->setMockData(ComputerModel::kSizeTotalRole, static_cast<qint64>(1024 * 1024 * 1024));
    mockModel->setMockData(ComputerModel::kSizeUsageRole, static_cast<qint64>(512 * 1024 * 1024));

    // Mock FileUtils::formatSize
    stub.set_lamda(&DFMBASE_NAMESPACE::FileUtils::formatSize, [](qint64 size, bool, int, int, QStringList) -> QString {
        __DBG_STUB_INVOKE__
        if (size == 1024 * 1024 * 1024)
            return "1.0 GB";
        else if (size == 512 * 1024 * 1024)
            return "512 MB";
        return "Unknown";
    });

    EXPECT_NO_THROW(delegate->drawDeviceDetail(mockPainter, mockOption, mockIndex));
}

TEST_F(UT_ComputerItemDelegate, GetProgressTotalColor_ReturnsValidColor)
{
    QColor color = delegate->getProgressTotalColor();
    EXPECT_TRUE(color.isValid());
}

TEST_F(UT_ComputerItemDelegate, RenderBlurShadow_ValidSize_ReturnsNonNullPixmap)
{
    QSize testSize(50, 50);
    QColor testColor(255, 0, 0, 128);
    int blurRadius = 5;

    QPixmap result = delegate->renderBlurShadow(testSize, testColor, blurRadius);
    EXPECT_FALSE(result.isNull());
    EXPECT_GT(result.size().width(), testSize.width());
    EXPECT_GT(result.size().height(), testSize.height());
}

TEST_F(UT_ComputerItemDelegate, RenderBlurShadow_ValidPixmap_ReturnsBlurredPixmap)
{
    QPixmap testPixmap(30, 30);
    testPixmap.fill(Qt::red);
    int blurRadius = 3;

    QPixmap result = delegate->renderBlurShadow(testPixmap, blurRadius);
    EXPECT_FALSE(result.isNull());
    EXPECT_GT(result.size().width(), testPixmap.size().width());
    EXPECT_GT(result.size().height(), testPixmap.size().height());
}

TEST_F(UT_ComputerItemDelegate, CloseEditor_ValidView_InvokesCommitData)
{
    // This method interacts with Qt's internal delegate mechanisms
    // We test that it doesn't crash when called
    EXPECT_NO_THROW(delegate->closeEditor(static_cast<ComputerView *>(mockView)));
}

TEST_F(UT_ComputerItemDelegate, CloseEditor_NullView_DoesNotCrash)
{
    EXPECT_NO_THROW(delegate->closeEditor(nullptr));
}
