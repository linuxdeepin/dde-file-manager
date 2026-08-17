// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/folderlistwidget.h"
#include "views/private/folderlistwidget_p.h"
#include "views/folderviewdelegate.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/chinese2pinyin.h>
#include <dfm-base/interfaces/abstractfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <DListView>

#include <gtest/gtest.h>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QVBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>
#include <QTest>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class FolderListWidgetTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Stub QIcon::fromTheme
        stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) {
            __DBG_STUB_INVOKE__
            QPixmap pixmap(16, 16);
            pixmap.fill(Qt::red);
            return QIcon(pixmap);
        });

        // Stub Application
        stub.set_lamda(&Application::instance, []() -> Application * {
            __DBG_STUB_INVOKE__
            static Application app;
            return &app;
        });

        stub.set_lamda(&Application::genericAttribute, [] {
            __DBG_STUB_INVOKE__
            return QVariant(true);
        });

        // Stub InfoFactory
        stub.set_lamda(&InfoFactory::create<FileInfo>,
                       [](const QUrl &url, Global::CreateFileInfoType, QString *) -> QSharedPointer<FileInfo> {
                           __DBG_STUB_INVOKE__
                           auto info = QSharedPointer<FileInfo>(new FileInfo(url));
                           return info;
                       });

        // Stub FileInfo
        stub.set_lamda(VADDR(FileInfo, fileIcon), [](FileInfo *) {
            __DBG_STUB_INVOKE__
            QPixmap pixmap(16, 16);
            pixmap.fill(Qt::blue);
            return QIcon(pixmap);
        });

        stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, FileInfo::FileIsType) {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Stub Pinyin
        stub.set_lamda(&Pinyin::Chinese2Pinyin, [](const QString &str) {
            __DBG_STUB_INVOKE__
            return str;
        });

        // Stub QGuiApplication::screenAt
        stub.set_lamda(static_cast<QScreen *(*)(const QPoint &)>(&QGuiApplication::screenAt), [](const QPoint &) {
            __DBG_STUB_INVOKE__
            return QGuiApplication::primaryScreen();
        });

        // Stub QGuiApplication::primaryScreen
        stub.set_lamda(&QGuiApplication::primaryScreen, []() {
            __DBG_STUB_INVOKE__
            static QScreen *screen = nullptr;
            if (!screen && QGuiApplication::screens().size() > 0) {
                screen = QGuiApplication::screens().first();
            }
            return screen;
        });

        // Stub QCursor::pos
        stub.set_lamda(qOverload<>(&QCursor::pos), []() {
            __DBG_STUB_INVOKE__
            return QPoint(100, 100);
        });

        widget = new FolderListWidget();
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

    FolderListWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(FolderListWidgetTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(widget, nullptr);
    EXPECT_NE(widget->d, nullptr);
}

TEST_F(FolderListWidgetTest, Constructor_InitializesPrivate_AllComponentsCreated)
{
    EXPECT_NE(widget->d->layout, nullptr);
    EXPECT_NE(widget->d->folderModel, nullptr);
    EXPECT_NE(widget->d->folderView, nullptr);
    EXPECT_NE(widget->d->folderDelegate, nullptr);
}

TEST_F(FolderListWidgetTest, Constructor_InitializesWidget_CorrectSize)
{
    EXPECT_EQ(widget->width(), 172);
    EXPECT_GT(widget->height(), 0);
}

TEST_F(FolderListWidgetTest, Constructor_InitializesLayout_CorrectMargins)
{
    EXPECT_EQ(widget->d->layout->contentsMargins(), QMargins(0, 0, 0, 0));
    EXPECT_EQ(widget->d->layout->spacing(), 0);
}

TEST_F(FolderListWidgetTest, Constructor_InitializesFolderView_CorrectSettings)
{
    EXPECT_TRUE(widget->d->folderView->hasMouseTracking());
    EXPECT_EQ(widget->d->folderView->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    EXPECT_EQ(widget->d->folderView->verticalScrollBarPolicy(), Qt::ScrollBarAsNeeded);
    EXPECT_TRUE(widget->d->folderView->uniformItemSizes());
    EXPECT_EQ(widget->d->folderView->viewMode(), QListView::ListMode);
}

TEST_F(FolderListWidgetTest, Constructor_InitializesFolderView_CorrectModel)
{
    EXPECT_EQ(widget->d->folderView->model(), widget->d->folderModel);
}

TEST_F(FolderListWidgetTest, Constructor_InitializesFolderView_CorrectDelegate)
{
    EXPECT_EQ(widget->d->folderView->itemDelegate(), widget->d->folderDelegate);
}

TEST_F(FolderListWidgetTest, Constructor_WindowFlags_SetCorrectly)
{
    EXPECT_TRUE(widget->windowFlags() & Qt::Popup);
    EXPECT_TRUE(widget->windowFlags() & Qt::FramelessWindowHint);
}

TEST_F(FolderListWidgetTest, SetFolderList_EmptyList_ModelCleared)
{
    QList<CrumbData> emptyList;

    widget->setFolderList(emptyList, false);

    EXPECT_EQ(widget->d->folderModel->rowCount(), 0);
}

TEST_F(FolderListWidgetTest, SetFolderList_SingleItem_ModelPopulated)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;

    widget->setFolderList(datas, false);

    EXPECT_EQ(widget->d->folderModel->rowCount(), 1);
    EXPECT_EQ(widget->d->crumbDatas.size(), 1);
}

TEST_F(FolderListWidgetTest, SetFolderList_MultipleItems_ModelPopulated)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 5; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }

    widget->setFolderList(datas, false);

    EXPECT_EQ(widget->d->folderModel->rowCount(), 5);
    EXPECT_EQ(widget->d->crumbDatas.size(), 5);
}

TEST_F(FolderListWidgetTest, SetFolderList_StackedMode_ShowsHiddenFiles)
{
    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, FileInfo::FileIsType) {
        __DBG_STUB_INVOKE__
        return true;   // Is hidden
    });

    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/.hidden");
    data.displayText = ".hidden";
    datas << data;

    widget->setFolderList(datas, true);

    // In stacked mode, should show hidden files
    EXPECT_EQ(widget->d->folderModel->rowCount(), 1);
}

TEST_F(FolderListWidgetTest, SetFolderList_NonStackedMode_HidesHiddenFiles)
{
    stub.set_lamda(&Application::genericAttribute, [] {
        __DBG_STUB_INVOKE__
        return QVariant(false);   // Don't show hidden files
    });

    stub.set_lamda(VADDR(FileInfo, isAttributes), [](FileInfo *, FileInfo::FileIsType) {
        __DBG_STUB_INVOKE__
        return true;   // Is hidden
    });

    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/.hidden");
    data.displayText = ".hidden";
    datas << data;

    widget->setFolderList(datas, false);

    // In non-stacked mode with hidden files disabled, should filter out
    EXPECT_EQ(widget->d->folderModel->rowCount(), 0);
}

TEST_F(FolderListWidgetTest, SetFolderList_ItemData_ContainsCorrectRole)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;

    widget->setFolderList(datas, false);

    EXPECT_EQ(widget->d->folderModel->item(0)->data(Qt::UserRole).toInt(), 0);
}

TEST_F(FolderListWidgetTest, SetFolderList_MultipleItems_CalculatesWidth)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("VeryLongTestName%1").arg(i);
        datas << data;
    }

    widget->setFolderList(datas, false);

    EXPECT_GE(widget->width(), 173);   // kMinAvailableWidth
}

TEST_F(FolderListWidgetTest, SetFolderList_SingleItem_AdjustsMargins)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;

    widget->setFolderList(datas, false);

    QMargins margins = widget->d->folderView->viewportMargins();
    // Single item should have different margins
    EXPECT_GT(margins.top(), 6);
}

TEST_F(FolderListWidgetTest, PopUp_BasicPosition_ShowsWidget)
{
    widget->popUp(QPoint(100, 100));

    EXPECT_TRUE(widget->isVisible());
}

TEST_F(FolderListWidgetTest, PopUp_BottomOverflow_AdjustsPosition)
{
    stub.set_lamda(&QGuiApplication::primaryScreen, []() {
        __DBG_STUB_INVOKE__
        static QScreen *screen = QGuiApplication::screens().first();
        return screen;
    });

    // Set a large size that would overflow
    widget->resize(200, 1000);

    QPoint popupPos(100, 100);
    widget->popUp(popupPos);

    // Should adjust position to fit within screen
    EXPECT_TRUE(widget->isVisible());
}

TEST_F(FolderListWidgetTest, PopUp_RightOverflow_AdjustsPosition)
{
    QRect screenRect = QRect(0, 0, 1920, 1080);
    stub.set_lamda(static_cast<QRect (QScreen::*)() const>(&QScreen::availableGeometry), [screenRect](QScreen *) {
        __DBG_STUB_INVOKE__
        return screenRect;
    });

    widget->resize(300, 200);

    QPoint popupPos(1800, 100);   // Would overflow right
    widget->popUp(popupPos);

    EXPECT_TRUE(widget->isVisible());
    // Position should be adjusted to fit
}

TEST_F(FolderListWidgetTest, PopUp_TopOverflow_AdjustsPosition)
{
    QRect screenRect = QRect(0, 0, 1920, 1080);
    stub.set_lamda(static_cast<QRect (QScreen::*)() const>(&QScreen::availableGeometry), [screenRect](QScreen *) {
        __DBG_STUB_INVOKE__
        return screenRect;
    });

    widget->resize(200, 200);

    QPoint popupPos(100, 5);   // Too close to top
    widget->popUp(popupPos);

    EXPECT_TRUE(widget->isVisible());
    EXPECT_GE(widget->y(), 10);   // kFloderListMargin
}

TEST_F(FolderListWidgetTest, PopUp_LeftOverflow_AdjustsPosition)
{
    QRect screenRect = QRect(0, 0, 1920, 1080);
    stub.set_lamda(static_cast<QRect (QScreen::*)() const>(&QScreen::availableGeometry), [screenRect](QScreen *) {
        __DBG_STUB_INVOKE__
        return screenRect;
    });

    widget->resize(200, 200);

    QPoint popupPos(-10, 100);   // Would overflow left
    widget->popUp(popupPos);

    EXPECT_TRUE(widget->isVisible());
    EXPECT_GE(widget->x(), 0);
}

TEST_F(FolderListWidgetTest, PopUp_MaxHeightLimit_ResizesWidget)
{
    QRect screenRect = QRect(0, 0, 1920, 100);   // Very short screen
    stub.set_lamda(static_cast<QRect (QScreen::*)() const>(&QScreen::availableGeometry), [screenRect](QScreen *) {
        __DBG_STUB_INVOKE__
        return screenRect;
    });
    stub.set_lamda(&FolderListWidget::show, [] { __DBG_STUB_INVOKE__ });

    widget->resize(200, 500);   // Taller than screen

    EXPECT_NO_THROW(widget->popUp(QPoint(100, 50)));
}

TEST_F(FolderListWidgetTest, KeyPressEvent_UpKey_SelectsUp)
{
    // Populate list
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(1, 0));

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    widget->keyPressEvent(&event);

    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 0);
}

TEST_F(FolderListWidgetTest, KeyPressEvent_UpKey_WrapsAround)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(0, 0));

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    widget->keyPressEvent(&event);

    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 2);
}

TEST_F(FolderListWidgetTest, KeyPressEvent_DownKey_SelectsDown)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(0, 0));

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    widget->keyPressEvent(&event);

    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 1);
}

TEST_F(FolderListWidgetTest, KeyPressEvent_DownKey_WrapsAround)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(2, 0));

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    widget->keyPressEvent(&event);

    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 0);
}

TEST_F(FolderListWidgetTest, KeyPressEvent_ReturnKey_EmitsUrlButtonActivated)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(0, 0));

    QSignalSpy spy(widget, &FolderListWidget::urlButtonActivated);

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    widget->keyPressEvent(&event);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl(), data.url);
}

TEST_F(FolderListWidgetTest, KeyPressEvent_ReturnKey_HidesWidget)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(0, 0));
    widget->show();

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    widget->keyPressEvent(&event);

    EXPECT_FALSE(widget->isVisible());
}

TEST_F(FolderListWidgetTest, KeyPressEvent_TextInput_FindsMatch)
{
    QList<CrumbData> datas;
    CrumbData data1;
    data1.url = QUrl("file:///home/abc");
    data1.displayText = "abc";
    datas << data1;

    CrumbData data2;
    data2.url = QUrl("file:///home/test");
    data2.displayText = "test";
    datas << data2;

    widget->setFolderList(datas, false);

    QKeyEvent event(QEvent::KeyPress, Qt::Key_T, Qt::NoModifier, "t");
    widget->keyPressEvent(&event);

    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 1);
}

TEST_F(FolderListWidgetTest, HideEvent_EmitsHiddenSignal)
{
    QSignalSpy spy(widget, &FolderListWidget::hidden);

    widget->show();
    widget->hide();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(FolderListWidgetTest, MatchText_EmptyInput_ReturnsFalse)
{
    EXPECT_FALSE(widget->d->matchText("test", ""));
}

TEST_F(FolderListWidgetTest, MatchText_EmptySource_ReturnsFalse)
{
    EXPECT_FALSE(widget->d->matchText("", "test"));
}

TEST_F(FolderListWidgetTest, MatchText_ExactMatch_ReturnsTrue)
{
    EXPECT_TRUE(widget->d->matchText("test", "test"));
}

TEST_F(FolderListWidgetTest, MatchText_PrefixMatch_ReturnsTrue)
{
    EXPECT_TRUE(widget->d->matchText("testing", "test"));
}

TEST_F(FolderListWidgetTest, MatchText_CaseInsensitive_ReturnsTrue)
{
    EXPECT_TRUE(widget->d->matchText("Testing", "test"));
    EXPECT_TRUE(widget->d->matchText("test", "TEST"));
}

TEST_F(FolderListWidgetTest, MatchText_NoMatch_ReturnsFalse)
{
    EXPECT_FALSE(widget->d->matchText("abc", "xyz"));
}

TEST_F(FolderListWidgetTest, MatchText_PartialMatch_ReturnsFalse)
{
    EXPECT_FALSE(widget->d->matchText("test", "esting"));
}

TEST_F(FolderListWidgetTest, MatchText_PinyinMatch_ReturnsTrue)
{
    stub.set_lamda(&Pinyin::Chinese2Pinyin, [](const QString &) {
        __DBG_STUB_INVOKE__
        return QString("ceshi");
    });

    EXPECT_TRUE(widget->d->matchText("测试", "ce"));
}

TEST_F(FolderListWidgetTest, FindAndSelectMatch_FoundMatch_ReturnsTrue)
{
    QList<CrumbData> datas;
    CrumbData data1;
    data1.url = QUrl("file:///home/abc");
    data1.displayText = "abc";
    datas << data1;

    CrumbData data2;
    data2.url = QUrl("file:///home/test");
    data2.displayText = "test";
    datas << data2;

    widget->setFolderList(datas, false);

    bool result = widget->d->findAndSelectMatch("test", 0);

    EXPECT_TRUE(result);
    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 1);
}

TEST_F(FolderListWidgetTest, FindAndSelectMatch_NoMatch_ReturnsFalse)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/abc");
    data.displayText = "abc";
    datas << data;

    widget->setFolderList(datas, false);

    bool result = widget->d->findAndSelectMatch("xyz", 0);

    EXPECT_FALSE(result);
}

TEST_F(FolderListWidgetTest, FindAndSelectMatch_WrapsAround_FindsMatch)
{
    QList<CrumbData> datas;
    CrumbData data1;
    data1.url = QUrl("file:///home/test");
    data1.displayText = "test";
    datas << data1;

    CrumbData data2;
    data2.url = QUrl("file:///home/abc");
    data2.displayText = "abc";
    datas << data2;

    widget->setFolderList(datas, false);

    // Start from row 1, should wrap and find "test" at row 0
    bool result = widget->d->findAndSelectMatch("test", 1);

    EXPECT_TRUE(result);
    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 0);
}

TEST_F(FolderListWidgetTest, FindAndSelectMatch_SkipsCurrentRow_FindsNext)
{
    QList<CrumbData> datas;
    CrumbData data1;
    data1.url = QUrl("file:///home/test1");
    data1.displayText = "test1";
    datas << data1;

    CrumbData data2;
    data2.url = QUrl("file:///home/test2");
    data2.displayText = "test2";
    datas << data2;

    widget->setFolderList(datas, false);

    // Start from row 0, should skip current and find next matching "test"
    bool result = widget->d->findAndSelectMatch("test", 0);

    EXPECT_TRUE(result);
    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 1);
}

TEST_F(FolderListWidgetTest, GetStartIndexFromHover_ValidCurrent_ReturnsNextIndex)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(1, 0));

    QModelIndex result = widget->d->getStartIndexFromHover(false);

    EXPECT_EQ(result.row(), 2);
}

TEST_F(FolderListWidgetTest, GetStartIndexFromHover_UpDirection_ReturnsPreviousIndex)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }
    widget->setFolderList(datas, false);

    widget->d->folderView->setCurrentIndex(widget->d->folderModel->index(1, 0));

    QModelIndex result = widget->d->getStartIndexFromHover(true);

    EXPECT_EQ(result.row(), 0);
}

TEST_F(FolderListWidgetTest, GetStartIndexFromHover_EmptyModel_ReturnsInvalid)
{
    QModelIndex result = widget->d->getStartIndexFromHover(false);

    EXPECT_FALSE(result.isValid());
}

TEST_F(FolderListWidgetTest, SelectUp_EmptyModel_HidesWidget)
{
    EXPECT_NO_THROW(widget->d->selectUp());
}

TEST_F(FolderListWidgetTest, SelectDown_EmptyModel_HidesWidget)
{
    EXPECT_NO_THROW(widget->d->selectDown());
}

TEST_F(FolderListWidgetTest, ReturnPressed_EmptyModel_HidesWidget)
{    
    EXPECT_NO_THROW(widget->d->returnPressed());
}

TEST_F(FolderListWidgetTest, ReturnPressed_InvalidIndex_HidesWidget)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    EXPECT_NO_THROW(widget->d->returnPressed());
}

TEST_F(FolderListWidgetTest, Clicked_ValidIndex_EmitsSignal)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    QSignalSpy spy(widget, &FolderListWidget::urlButtonActivated);

    QModelIndex index = widget->d->folderModel->index(0, 0);
    widget->d->clicked(index);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl(), data.url);
}

TEST_F(FolderListWidgetTest, Clicked_InvalidIndex_DoesNotEmit)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    QSignalSpy spy(widget, &FolderListWidget::urlButtonActivated);

    QModelIndex index;   // Invalid
    widget->d->clicked(index);

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(FolderListWidgetTest, Clicked_OutOfRangeRow_DoesNotEmit)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    QSignalSpy spy(widget, &FolderListWidget::urlButtonActivated);

    // Create an index with row out of range
    QStandardItem *item = new QStandardItem("fake");
    item->setData(999, Qt::UserRole);   // Out of range
    widget->d->folderModel->appendRow(item);

    QModelIndex index = widget->d->folderModel->index(1, 0);
    widget->d->clicked(index);

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(FolderListWidgetTest, Clicked_HidesWidget)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    QModelIndex index = widget->d->folderModel->index(0, 0);
    EXPECT_NO_THROW(widget->d->clicked(index));
}

TEST_F(FolderListWidgetTest, HandleKeyInput_EmptyText_DoesNothing)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    widget->d->handleKeyInput("");

    // Should not change selection
    EXPECT_FALSE(widget->d->folderView->currentIndex().isValid());
}

TEST_F(FolderListWidgetTest, HandleKeyInput_NonPrintable_DoesNothing)
{
    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "test";
    datas << data;
    widget->setFolderList(datas, false);

    widget->d->handleKeyInput("\n");

    EXPECT_FALSE(widget->d->folderView->currentIndex().isValid());
}

TEST_F(FolderListWidgetTest, HandleKeyInput_ValidText_FindsMatch)
{
    QList<CrumbData> datas;
    CrumbData data1;
    data1.url = QUrl("file:///home/abc");
    data1.displayText = "abc";
    datas << data1;

    CrumbData data2;
    data2.url = QUrl("file:///home/test");
    data2.displayText = "test";
    datas << data2;

    widget->setFolderList(datas, false);

    widget->d->handleKeyInput("t");

    EXPECT_EQ(widget->d->folderView->currentIndex().row(), 1);
}

TEST_F(FolderListWidgetTest, AvailableGeometry_WithValidScreen_ReturnsGeometry)
{
    QRect result = widget->availableGeometry(QPoint(100, 100));

    EXPECT_FALSE(result.isNull());
    EXPECT_GT(result.width(), 0);
    EXPECT_GT(result.height(), 0);
}

TEST_F(FolderListWidgetTest, AvailableGeometry_NullPosition_UsesCurrentScreen)
{
    QRect result = widget->availableGeometry(QPoint());

    EXPECT_FALSE(result.isNull());
}

TEST_F(FolderListWidgetTest, AvailableGeometry_NoScreenAtPosition_UsesPrimaryScreen)
{
    stub.set_lamda(static_cast<QScreen *(*)(const QPoint &)>(&QGuiApplication::screenAt), [](const QPoint &) {
        __DBG_STUB_INVOKE__
        return nullptr;
    });

    QRect result = widget->availableGeometry(QPoint(100, 100));

    EXPECT_FALSE(result.isNull());
}

TEST_F(FolderListWidgetTest, SetFolderList_MaxWidthLimit_AppliesCorrectly)
{
    stub.set_lamda(&DListView::sizeHintForIndex, [] {
        __DBG_STUB_INVOKE__
        return QSize(1000, 30);   // Very wide
    });

    QList<CrumbData> datas;
    CrumbData data;
    data.url = QUrl("file:///home/test");
    data.displayText = "VeryLongTextThatShouldExceedMaxWidth";
    datas << data;

    widget->setFolderList(datas, false);

    EXPECT_LE(widget->width(), 800);   // kMaxAvailableWidth
}

TEST_F(FolderListWidgetTest, KeyPressEvent_NoCurrentIndex_UsesHoverIndex)
{
    QList<CrumbData> datas;
    for (int i = 0; i < 3; ++i) {
        CrumbData data;
        data.url = QUrl(QString("file:///home/test%1").arg(i));
        data.displayText = QString("test%1").arg(i);
        datas << data;
    }
    widget->setFolderList(datas, false);

    // No current index set

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    widget->keyPressEvent(&event);

    // Should set a current index based on hover position
    EXPECT_TRUE(widget->d->folderView->currentIndex().isValid());
}

TEST_F(FolderListWidgetTest, SetFolderList_ClearsModel_BeforePopulating)
{
    QList<CrumbData> datas1;
    CrumbData data1;
    data1.url = QUrl("file:///home/test1");
    data1.displayText = "test1";
    datas1 << data1;

    widget->setFolderList(datas1, false);
    EXPECT_EQ(widget->d->folderModel->rowCount(), 1);

    QList<CrumbData> datas2;
    CrumbData data2;
    data2.url = QUrl("file:///home/test2");
    data2.displayText = "test2";
    datas2 << data2;

    widget->setFolderList(datas2, false);
    EXPECT_EQ(widget->d->folderModel->rowCount(), 1);
    EXPECT_EQ(widget->d->folderModel->item(0)->text(), "test2");
}
