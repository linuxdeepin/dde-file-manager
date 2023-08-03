// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/filepreviewdialog.h"
#include "views/unknowfilepreview.h"
#include "utils/previewfileoperation.h"

#include <gtest/gtest.h>

#include <QShowEvent>

#include <DGuiApplicationHelper>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>

DPFILEPREVIEW_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_FilePreviewDialog, updatePreviewList_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST");
    FilePreviewDialog dlg(list);
    dlg.playingVideo = true;
    dlg.updatePreviewList(list);

    EXPECT_TRUE(dlg.playingVideo);
}

TEST(UT_FilePreviewDialog, updatePreviewList_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST");
    FilePreviewDialog dlg(list);
    dlg.updatePreviewList(list);

    EXPECT_FALSE(dlg.statusBar->preButton()->isVisible());
}

TEST(UT_FilePreviewDialog, updatePreviewList_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&QWidget::show, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.updatePreviewList(list);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, setEntryUrlList_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.setEntryUrlList(QList<QUrl>());

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, setEntryUrlList_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.fileList.push_back(QUrl("file:///UT_TEST1"));
    dlg.currentPageIndex = 0;
    dlg.setEntryUrlList(list);

    EXPECT_TRUE(dlg.previewDir);
}

TEST(UT_FilePreviewDialog, done)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(VADDR(QDialog, done), [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.preview = new UnknowFilePreview;
    dlg.done(0);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, setCurrentWinID)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.setCurrentWinID(0);

    EXPECT_TRUE(dlg.currentWinID == 0);
}

TEST(UT_FilePreviewDialog, playCurrentPreviewFile)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&QMetaObject::className, []{
        return "VideoPreview";
    });
    typedef  void(*FuncType)(int, Qt::TimerType,
                             const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType>(QTimer::singleShotImpl), [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    UnknowFilePreview view;
    FilePreviewDialog dlg(list);
    dlg.preview = &view;
    dlg.playCurrentPreviewFile();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, openFile)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&PreviewFileOperation::openFileHandle, [ &isOk ]{
        isOk = true;
        return true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.fileList.push_back(QUrl("file:///UT_TEST1"));
    dlg.currentPageIndex = 0;
    dlg.openFile();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, childEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&QChildEvent::added, []{
        return true;
    });
    stub.set_lamda(&QWidget::raise, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QChildEvent event(QEvent::None, nullptr);
    dlg.childEvent(&event);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, showEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    typedef void(*FuncType)(QShowEvent *);
    stub.set_lamda((FuncType)(void(DAbstractDialog::*)(QShowEvent *))(&DAbstractDialog::showEvent), [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QShowEvent event;
    dlg.showEvent(&event);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, closeEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    typedef void(*FuncType)(QCloseEvent *);
    stub.set_lamda((FuncType)(void(QDialog::*)(QCloseEvent *))(&QDialog::closeEvent), [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    UnknowFilePreview view;
    dlg.preview = &view;
    QCloseEvent event;
    dlg.closeEvent(&event);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, resizeEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    typedef void(*FuncType)(QResizeEvent *);
    stub.set_lamda((FuncType)(void(DAbstractDialog::*)(QResizeEvent *))(&DAbstractDialog::resizeEvent), [ &isOk ]{
        isOk = true;
    });
    typedef  void(*FuncType2)(int, Qt::TimerType,
                             const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType2>(QTimer::singleShotImpl), []{});

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QResizeEvent event(QSize(2, 2), QSize(1, 1));
    dlg.resizeEvent(&event);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, eventFilter_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&FilePreviewDialog::previousPage, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    dlg.eventFilter(nullptr, &event);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, eventFilter_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&FilePreviewDialog::nextPage, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    dlg.eventFilter(nullptr, &event);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, eventFilter_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&QWidget::close, [ &isOk ]{
        isOk = true;
        return true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    dlg.eventFilter(nullptr, &event);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, initUI_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    typedef DGuiApplicationHelper::ColorType(*FuncType)(const QColor &);
    stub.set_lamda(static_cast<FuncType>(&DGuiApplicationHelper::toColorType), [ &isOk ]{
        isOk = true;
        return DGuiApplicationHelper::LightType;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, initUI_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    typedef DGuiApplicationHelper::ColorType(*FuncType)(const QColor &);
    stub.set_lamda(static_cast<FuncType>(&DGuiApplicationHelper::toColorType), [ &isOk ]{
        isOk = true;
        return DGuiApplicationHelper::DarkType;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, switchToPage_one)
{
    bool isOk { false };

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);

    stub_ext::StubExt st;
    st.set_lamda(&InfoFactory::create<FileInfo>, [ &isOk ] {
        isOk = true;
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl("file:///UT_TEST")));
    });
    dlg.switchToPage(0);

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, previousPage)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.currentPageIndex = 0;
    dlg.previousPage();

    dlg.currentPageIndex = 1;
    dlg.playingVideo = true;
    dlg.previousPage();

    dlg.currentPageIndex = 1;
    dlg.playingVideo = false;
    dlg.previousPage();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, nextPage)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.currentPageIndex = 1;
    dlg.fileList.push_back(QUrl("file:///UT_TEST1"));
    dlg.nextPage();

    dlg.currentPageIndex = 0;
    dlg.fileList.push_back(QUrl("file:///UT_TEST1"));
    dlg.fileList.push_back(QUrl("file:///UT_TEST2"));
    dlg.fileList.push_back(QUrl("file:///UT_TEST3"));
    dlg.playingVideo = true;
    dlg.nextPage();

    dlg.playingVideo = false;
    dlg.nextPage();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, updateTitle_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, [ &isOk ]{
        isOk = true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    dlg.preview =  nullptr;
    dlg.updateTitle();

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, updateTitle_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&QWidget::isVisible, [ &isOk ]{
        isOk = true;
        return false;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    UnknowFilePreview view;
    dlg.preview = &view;
    dlg.updateTitle();
    dlg.preview = nullptr;

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, updateTitle_three)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});
    stub.set_lamda(&QWidget::isVisible, [ &isOk ]{
        isOk = true;
        return true;
    });

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    UnknowFilePreview view;
    dlg.preview = &view;
    dlg.updateTitle();
    dlg.preview = nullptr;

    EXPECT_TRUE(isOk);
}

TEST(UT_FilePreviewDialog, generalKey_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QString result = dlg.generalKey("/UT/UT_TEST");

    EXPECT_TRUE(result == "/*");
}

TEST(UT_FilePreviewDialog, generalKey_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FilePreviewDialog::switchToPage, []{});

    QList<QUrl> list;
    list << QUrl("file:///UT_TEST1") << QUrl("file:///UT_TEST2");
    FilePreviewDialog dlg(list);
    QString result = dlg.generalKey("/UT");

    EXPECT_TRUE(result == "/*");
}
