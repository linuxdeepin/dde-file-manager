// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage leftovers for the pdf-preview plugin: DocSheet::operationRef()
// plus the pure-virtual Page / Document destructors declared in model.h.

#include "docsheet.h"
#include "model.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QEvent>

using namespace plugin_filepreview;

// ---------------------------------------------------------------------------
// DocSheet::operationRef()
// ---------------------------------------------------------------------------
class PdfLeftoversCovTest : public testing::Test
{
protected:
    void SetUp() override
    {
        sheet = new DocSheet(kUnknown, "/tmp/cov-no-such-document.pdf");
    }
    void TearDown() override
    {
        delete sheet;
        sheet = nullptr;
        qApp->processEvents();
        qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    DocSheet *sheet { nullptr };
};

TEST_F(PdfLeftoversCovTest, OperationRef_ExposesTheMutableOperationState)
{
    // Arrange: the sheet starts with the default operation values
    EXPECT_EQ(sheet->operation().scaleFactor, 0.9);
    EXPECT_EQ(sheet->operation().rotation, kRotateBy0);

    // Act: mutate the state through the reference
    SheetOperation &ref = sheet->operationRef();
    ref.scaleFactor = 1.5;
    ref.rotation = kRotateBy90;
    ref.currentPage = 3;

    // Assert: operation() reflects the very same struct
    EXPECT_EQ(sheet->operation().scaleFactor, 1.5);
    EXPECT_EQ(sheet->operation().rotation, kRotateBy90);
    EXPECT_EQ(sheet->operation().currentPage, 3);
}

// ---------------------------------------------------------------------------
// model.h Page / Document destructors
// ---------------------------------------------------------------------------
namespace {
class CovPage final : public Page
{
public:
    QSizeF sizeF() const override { return QSizeF(10, 20); }
    QImage render(int, int, const QRect & = QRect()) const override { return QImage(); }
};

class CovDocument final : public Document
{
public:
    int pageCount() const override { return 0; }
    Page *page(int) const override { return nullptr; }
};
}   // namespace

TEST_F(PdfLeftoversCovTest, ModelDestructors_VirtualDispatchThroughBasePointer)
{
    // Arrange: two heap objects held through their abstract base pointers
    Page *page = new CovPage();
    Document *document = new CovDocument();

    // Assert (arranged state): the derived implementations answer through the
    // base interface before destruction
    EXPECT_EQ(page->sizeF(), QSizeF(10, 20));
    EXPECT_EQ(document->pageCount(), 0);

    // Act: destroying through the base pointer must enter the (virtual)
    // base-class destructors declared in model.h
    EXPECT_NO_FATAL_FAILURE(delete page);
    EXPECT_NO_FATAL_FAILURE(delete document);
}
