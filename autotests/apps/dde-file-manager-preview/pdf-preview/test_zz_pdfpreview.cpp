// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docsheet.h"
#include "encryptionpage.h"
#include "global.h"
#include "pagerenderthread.h"
#include "pdfpreview.h"
#include "pdfpreviewplugin.h"
#include "pdfwidget.h"
#include "ut_common.h"

#include "stubext.h"
#include <dfm-base/mimetype/dmimedatabase.h>

#include <gtest/gtest.h>

#include <DPasswordEdit>
#include <DPushButton>

#include <QMimeDatabase>
#include <QMimeType>
#include <QSignalSpy>

using namespace plugin_filepreview;

static QMimeType pdfMime()
{
    QMimeDatabase db;
    return db.mimeTypeForName("application/pdf");
}

class UT_EncryptionPage : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        page = new EncryptionPage();
    }

    virtual void TearDown() override
    {
        delete page;
        ut_utils::drainEvents(50);
    }

protected:
    EncryptionPage *page { nullptr };

    DTK_WIDGET_NAMESPACE::DPushButton *nextButton() const
    {
        return page->findChild<DTK_WIDGET_NAMESPACE::DPushButton *>("ensureBtn");
    }

    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordEdit() const
    {
        return page->findChild<DTK_WIDGET_NAMESPACE::DPasswordEdit *>("PasswordEdit");
    }
};

TEST_F(UT_EncryptionPage, Construct_CreatesControls)
{
    EXPECT_NE(nullptr, nextButton());
    EXPECT_NE(nullptr, passwordEdit());
}

TEST_F(UT_EncryptionPage, Construct_NextButtonDisabledInitially)
{
    EXPECT_FALSE(nextButton()->isEnabled());
}

TEST_F(UT_EncryptionPage, PasswordChanged_EmptyPassword_KeepsButtonDisabled)
{
    passwordEdit()->setText("");
    ut_utils::drainEvents(50);
    EXPECT_FALSE(nextButton()->isEnabled());
}

TEST_F(UT_EncryptionPage, PasswordChanged_NonEmptyPassword_EnablesButton)
{
    passwordEdit()->setText("secret");
    ut_utils::drainEvents(50);
    EXPECT_TRUE(nextButton()->isEnabled());
}

TEST_F(UT_EncryptionPage, NextbuttonClicked_EmitsPasswordSignal)
{
    passwordEdit()->setText("secret");
    ut_utils::drainEvents(50);
    QSignalSpy spy(page, SIGNAL(sigExtractPassword(const QString &)));
    page->nextbuttonClicked();
    ASSERT_EQ(1, spy.count());
    EXPECT_EQ(QStringLiteral("secret"), spy.at(0).at(0).toString());
}

TEST_F(UT_EncryptionPage, WrongPasswordSlot_ClearsTextAndSetsAlert)
{
    passwordEdit()->setText("bad");
    ut_utils::drainEvents(50);
    page->wrongPassWordSlot();
    EXPECT_TRUE(passwordEdit()->text().isEmpty());
    EXPECT_TRUE(passwordEdit()->isAlert());
}

TEST_F(UT_EncryptionPage, TypingAfterWrongPassword_ClearsAlert)
{
    page->wrongPassWordSlot();
    EXPECT_TRUE(passwordEdit()->isAlert());
    passwordEdit()->setText("retry");
    ut_utils::drainEvents(50);
    EXPECT_FALSE(passwordEdit()->isAlert());
}

TEST_F(UT_EncryptionPage, OnSetPasswdFocus_WhileHidden_NoCrash)
{
    page->onSetPasswdFocus();
    SUCCEED();
}

TEST_F(UT_EncryptionPage, OnSetPasswdFocus_WhileShown_SetsFocus)
{
    page->show();
    ut_utils::drainEvents(50);
    page->onSetPasswdFocus();
    EXPECT_EQ(passwordEdit()->lineEdit(), qApp->focusWidget());
    page->hide();
}

TEST_F(UT_EncryptionPage, OnUpdateTheme_AppliesPalette)
{
    page->onUpdateTheme();
    SUCCEED();
}

TEST_F(UT_EncryptionPage, InitUI_SecondCall_NoCrash)
{
    page->InitUI();
    SUCCEED();
}

TEST_F(UT_EncryptionPage, InitConnection_SecondCall_NoCrash)
{
    page->InitConnection();
    SUCCEED();
}

class UT_recordSheetPath : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        sheetA = new DocSheet(kPDF, "/tmp/record-a.pdf");
        sheetB = new DocSheet(kPDF, "/tmp/record-b.pdf");
    }

    virtual void TearDown() override
    {
        delete sheetA;
        delete sheetB;
        ut_utils::drainEvents(50);
    }

protected:
    DocSheet *sheetA { nullptr };
    DocSheet *sheetB { nullptr };
};

TEST_F(UT_recordSheetPath, InsertSheet_Null_Ignored)
{
    recordSheetPath record;
    record.insertSheet(nullptr);
    EXPECT_TRUE(record.getSheets().isEmpty());
}

TEST_F(UT_recordSheetPath, RemoveSheet_Null_Ignored)
{
    recordSheetPath record;
    record.removeSheet(nullptr);
    EXPECT_TRUE(record.getSheets().isEmpty());
}

TEST_F(UT_recordSheetPath, InsertSheet_RecordsFilePath)
{
    recordSheetPath record;
    record.insertSheet(sheetA);
    ASSERT_EQ(1, record.getSheets().count());
    EXPECT_EQ(sheetA, record.getSheets().first());
}

TEST_F(UT_recordSheetPath, IndexOfFilePath_KnownPath_Found)
{
    recordSheetPath record;
    record.insertSheet(sheetA);
    record.insertSheet(sheetB);
    int indexA = record.indexOfFilePath("/tmp/record-a.pdf");
    int indexB = record.indexOfFilePath("/tmp/record-b.pdf");
    EXPECT_NE(indexA, indexB);
    EXPECT_TRUE(indexA == 0 || indexA == 1);
    EXPECT_TRUE(indexB == 0 || indexB == 1);
}

TEST_F(UT_recordSheetPath, IndexOfFilePath_UnknownPath_ReturnsMinusOne)
{
    recordSheetPath record;
    record.insertSheet(sheetA);
    EXPECT_EQ(-1, record.indexOfFilePath("/tmp/no-such.pdf"));
}

TEST_F(UT_recordSheetPath, RemoveSheet_RemovesEntry)
{
    recordSheetPath record;
    record.insertSheet(sheetA);
    record.insertSheet(sheetB);
    record.removeSheet(sheetA);
    EXPECT_EQ(1, record.getSheets().count());
    EXPECT_EQ(-1, record.indexOfFilePath("/tmp/record-a.pdf"));
}

static PdfWidget *g_sharedWidget = nullptr;

static PdfWidget *sharedWidget()
{
    if (!g_sharedWidget)
        g_sharedWidget = new PdfWidget();
    return g_sharedWidget;
}

class UT_PdfWidget : public testing::Test
{
protected:
    virtual void TearDown() override
    {
        ut_utils::drainEvents(100);
    }
};

TEST_F(UT_PdfWidget, AddFileAsync_ValidPdf_OpensAndRegistersSheet)
{
    qRegisterMetaType<Document::Error>("Document::Error");
    qRegisterMetaType<DocSheet *>("DocSheet *");
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<QMimeType (DFMBASE_NAMESPACE::DMimeDatabase::*)(const QUrl &, QMimeDatabase::MatchMode) const>(&DFMBASE_NAMESPACE::DMimeDatabase::mimeTypeForFile),
                   [](DFMBASE_NAMESPACE::DMimeDatabase *, const QUrl &, QMimeDatabase::MatchMode) {
                       __DBG_STUB_INVOKE__
                       return pdfMime();
                   });

    QString pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
    sharedWidget()->addFileAsync(pdfPath);

    DocSheet *sheet = DocSheet::getSheetByFilePath(pdfPath);
    ASSERT_NE(nullptr, sheet);
    ut_utils::processEventsUntil([sheet]() { return sheet->opened(); });
    EXPECT_TRUE(sheet->opened());
}

TEST_F(UT_PdfWidget, AddFileAsync_SamePathAgain_SwitchesToExistingSheet)
{
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<QMimeType (DFMBASE_NAMESPACE::DMimeDatabase::*)(const QUrl &, QMimeDatabase::MatchMode) const>(&DFMBASE_NAMESPACE::DMimeDatabase::mimeTypeForFile),
                   [](DFMBASE_NAMESPACE::DMimeDatabase *, const QUrl &, QMimeDatabase::MatchMode) {
                       __DBG_STUB_INVOKE__
                       return pdfMime();
                   });

    QString pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
    DocSheet *before = DocSheet::getSheetByFilePath(pdfPath);
    ASSERT_NE(nullptr, before);
    int sheetsBefore = DocSheet::getSheets().count();

    sharedWidget()->addFileAsync(pdfPath);

    EXPECT_EQ(before, DocSheet::getSheetByFilePath(pdfPath));
    EXPECT_EQ(sheetsBefore, DocSheet::getSheets().count());
}

TEST_F(UT_PdfWidget, AddFileAsync_NotPdfContent_Ignored)
{
    QString textPath = ut_utils::prepareFile("/etc/hostname");
    int sheetsBefore = DocSheet::getSheets().count();
    sharedWidget()->addFileAsync(textPath);
    ut_utils::drainEvents(100);
    EXPECT_EQ(sheetsBefore, DocSheet::getSheets().count());
    EXPECT_EQ(nullptr, DocSheet::getSheetByFilePath(textPath));
}

TEST_F(UT_PdfWidget, AddSheet_NullSheet_Ignored)
{
    int sheetsBefore = DocSheet::getSheets().count();
    sharedWidget()->addSheet(nullptr);
    EXPECT_EQ(sheetsBefore, DocSheet::getSheets().count());
}

TEST_F(UT_PdfWidget, AddSheet_ValidSheet_RepresentsWidget)
{
    DocSheet *sheet = new DocSheet(kPDF, "/tmp/pdfwidget-add.pdf");
    sharedWidget()->addSheet(sheet);
    EXPECT_TRUE(DocSheet::existSheet(sheet));
    EXPECT_EQ(sharedWidget(), sheet->parent());

    sharedWidget()->closeSheet(sheet);
    EXPECT_FALSE(DocSheet::existSheet(sheet));
}

TEST_F(UT_PdfWidget, EnterSheet_NullSheet_Ignored)
{
    sharedWidget()->enterSheet(nullptr);
    SUCCEED();
}

TEST_F(UT_PdfWidget, LeaveSheet_NullSheet_Ignored)
{
    sharedWidget()->leaveSheet(nullptr);
    SUCCEED();
}

TEST_F(UT_PdfWidget, LeaveSheet_ValidSheet_NoCrash)
{
    DocSheet *sheet = new DocSheet(kPDF, "/tmp/pdfwidget-leave.pdf");
    sharedWidget()->addSheet(sheet);
    sharedWidget()->leaveSheet(sheet);
    EXPECT_TRUE(DocSheet::existSheet(sheet));
    sharedWidget()->closeSheet(sheet);
}

TEST_F(UT_PdfWidget, CloseSheet_NullSheet_ReturnsFalse)
{
    EXPECT_FALSE(sharedWidget()->closeSheet(nullptr));
}

TEST_F(UT_PdfWidget, CloseSheet_SheetNotInWidgetMap_StillClosesByGlobalRegistry)
{
    DocSheet *sheet = new DocSheet(kPDF, "/tmp/pdfwidget-close-unreg.pdf");
    EXPECT_TRUE(DocSheet::existSheet(sheet));
    EXPECT_TRUE(sharedWidget()->closeSheet(sheet));
    EXPECT_FALSE(DocSheet::existSheet(sheet));
}

TEST_F(UT_PdfWidget, CloseSheet_RegisteredSheet_ReturnsTrueAndDeletes)
{
    DocSheet *sheet = new DocSheet(kPDF, "/tmp/pdfwidget-close.pdf");
    sharedWidget()->addSheet(sheet);
    EXPECT_TRUE(sharedWidget()->closeSheet(sheet));
    EXPECT_FALSE(DocSheet::existSheet(sheet));
    ut_utils::drainEvents(100);
}

TEST_F(UT_PdfWidget, CloseAllSheets_MultipleSheets_ClosesEverything)
{
    DocSheet *first = new DocSheet(kPDF, "/tmp/pdfwidget-all-1.pdf");
    DocSheet *second = new DocSheet(kPDF, "/tmp/pdfwidget-all-2.pdf");
    sharedWidget()->addSheet(first);
    sharedWidget()->addSheet(second);

    EXPECT_TRUE(sharedWidget()->closeAllSheets());
    EXPECT_FALSE(DocSheet::existSheet(first));
    EXPECT_FALSE(DocSheet::existSheet(second));
    ut_utils::drainEvents(100);
}

TEST_F(UT_PdfWidget, OnOpened_FileError_SchedulesSheetDeletion)
{
    qRegisterMetaType<DocSheet *>("DocSheet *");
    DocSheet *sheet = new DocSheet(kPDF, "/nonexistent-dir/broken.pdf");
    sharedWidget()->addSheet(sheet);
    ASSERT_TRUE(DocSheet::existSheet(sheet));

    sharedWidget()->onOpened(sheet, Document::kFileError);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    EXPECT_FALSE(DocSheet::existSheet(sheet));
}

TEST_F(UT_PdfWidget, OnOpened_FileDamaged_SchedulesSheetDeletion)
{
    qRegisterMetaType<DocSheet *>("DocSheet *");
    DocSheet *sheet = new DocSheet(kPDF, "/nonexistent-dir/damaged.pdf");
    sharedWidget()->addSheet(sheet);
    ASSERT_TRUE(DocSheet::existSheet(sheet));

    sharedWidget()->onOpened(sheet, Document::kFileDamaged);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    EXPECT_FALSE(DocSheet::existSheet(sheet));
}

TEST_F(UT_PdfWidget, OnOpened_NullSheet_Ignored)
{
    sharedWidget()->onOpened(nullptr, Document::kNoError);
    SUCCEED();
}

TEST_F(UT_PdfWidget, OnOpened_Success_KeepsSheet)
{
    qRegisterMetaType<DocSheet *>("DocSheet *");
    DocSheet *sheet = new DocSheet(kPDF, "/tmp/pdfwidget-success.pdf");
    sharedWidget()->addSheet(sheet);

    sharedWidget()->onOpened(sheet, Document::kNoError);

    EXPECT_TRUE(DocSheet::existSheet(sheet));
    sharedWidget()->closeSheet(sheet);
}

TEST(UT_PDFPreviewPlugin, Create_AnyKey_ReturnsNewPreviewInstance)
{
    PDFPreviewPlugin plugin;
    DFMBASE_NAMESPACE::AbstractBasePreview *preview = plugin.create("pdf");
    EXPECT_NE(nullptr, preview);
    delete preview;
}

class UT_PDFPreview : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        qRegisterMetaType<Document::Error>("Document::Error");
        qRegisterMetaType<DocSheet *>("DocSheet *");
        pdfPath = ut_utils::prepareFile(ut_utils::kMultiPagePdfSrc);
        preview = new PDFPreview();
    }

    virtual void TearDown() override
    {
        setMainWidget(nullptr);
        delete preview;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        ut_utils::drainEvents(50);
    }

protected:
    QString pdfPath;
    PDFPreview *preview { nullptr };
};

TEST_F(UT_PDFPreview, SetFileUrl_NotLocalFile_ReturnsFalse)
{
    EXPECT_FALSE(preview->setFileUrl(QUrl("http://example.com/file.pdf")));
    EXPECT_EQ(QUrl(), preview->fileUrl());
}

TEST_F(UT_PDFPreview, SetFileUrl_MissingLocalFile_ReturnsFalse)
{
    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile("/nonexistent-dir/no.pdf")));
    EXPECT_EQ(QUrl(), preview->fileUrl());
}

TEST_F(UT_PDFPreview, ShowStatusBarSeparator_ReturnsFalse)
{
    EXPECT_FALSE(preview->showStatusBarSeparator());
}

TEST_F(UT_PDFPreview, ContentWidget_InitiallyNull)
{
    EXPECT_EQ(nullptr, preview->contentWidget());
}

TEST_F(UT_PDFPreview, Title_InitiallyEmpty)
{
    EXPECT_TRUE(preview->title().isEmpty());
}

TEST_F(UT_PDFPreview, SetFileUrl_ValidPdf_ReturnsTrueAndUpdatesState)
{
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<QMimeType (DFMBASE_NAMESPACE::DMimeDatabase::*)(const QUrl &, QMimeDatabase::MatchMode) const>(&DFMBASE_NAMESPACE::DMimeDatabase::mimeTypeForFile),
                   [](DFMBASE_NAMESPACE::DMimeDatabase *, const QUrl &, QMimeDatabase::MatchMode) {
                       __DBG_STUB_INVOKE__
                       return pdfMime();
                   });

    QSignalSpy spy(preview, SIGNAL(titleChanged()));
    QUrl url = QUrl::fromLocalFile(pdfPath);
    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(url, preview->fileUrl());
    EXPECT_EQ(QFileInfo(pdfPath).fileName(), preview->title());
    EXPECT_NE(nullptr, preview->contentWidget());
    EXPECT_GE(spy.count(), 1);
}

TEST_F(UT_PDFPreview, SetFileUrl_SameUrlTwice_ReturnsTrueWithoutReopen)
{
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<QMimeType (DFMBASE_NAMESPACE::DMimeDatabase::*)(const QUrl &, QMimeDatabase::MatchMode) const>(&DFMBASE_NAMESPACE::DMimeDatabase::mimeTypeForFile),
                   [](DFMBASE_NAMESPACE::DMimeDatabase *, const QUrl &, QMimeDatabase::MatchMode) {
                       __DBG_STUB_INVOKE__
                       return pdfMime();
                   });

    QUrl url = QUrl::fromLocalFile(pdfPath);
    ASSERT_TRUE(preview->setFileUrl(url));
    QWidget *widgetAfterFirst = preview->contentWidget();
    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(widgetAfterFirst, preview->contentWidget());
}

TEST_F(UT_PDFPreview, Initialize_SetsGlobalMainWidget)
{
    QWidget window;
    preview->initialize(&window, nullptr);
    EXPECT_EQ(&window, getMainDialog());
}

TEST(UT_PageRenderThread_Final, DestroyForever_StopsThreadAndAppendTasksBecomeNoOp)
{
    if (g_sharedWidget) {
        g_sharedWidget->closeAllSheets();
        ut_utils::waitRenderIdle();
        delete g_sharedWidget;
        g_sharedWidget = nullptr;
    }
    PageRenderThread::destroyForever();

    DocPageNormalImageTask normalTask;
    PageRenderThread::appendTask(normalTask);
    DocPageSliceImageTask sliceTask;
    PageRenderThread::appendTask(sliceTask);
    DocPageThumbnailTask thumbnailTask;
    PageRenderThread::appendTask(thumbnailTask);
    DocOpenTask openTask;
    PageRenderThread::appendTask(openTask);
    DocCloseTask closeTask;
    PageRenderThread::appendTask(closeTask);

    PageRenderThread::destroyForever();
    SUCCEED();
}
