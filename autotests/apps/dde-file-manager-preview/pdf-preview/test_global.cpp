// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global.h"
#include "ut_common.h"

#include "stubext.h"
#include <dfm-base/mimetype/dmimedatabase.h>

#include <gtest/gtest.h>

#include <QMimeDatabase>
#include <QMimeType>
#include <QWidget>

using namespace plugin_filepreview;

static QMimeType pdfMime()
{
    QMimeDatabase db;
    return db.mimeTypeForName("application/pdf");
}

TEST(UT_Global, FileType_PdfContent_ReturnsKPDF)
{
    stub_ext::StubExt stub;
    stub.set_lamda(static_cast<QMimeType (DFMBASE_NAMESPACE::DMimeDatabase::*)(const QUrl &, QMimeDatabase::MatchMode) const>(&DFMBASE_NAMESPACE::DMimeDatabase::mimeTypeForFile),
                   [](DFMBASE_NAMESPACE::DMimeDatabase *, const QUrl &, QMimeDatabase::MatchMode) {
                       __DBG_STUB_INVOKE__
                       return pdfMime();
                   });

    EXPECT_EQ(kPDF, fileType(ut_utils::prepareFile(ut_utils::kSinglePagePdfSrc)));
}

TEST(UT_Global, FileType_TextContent_ReturnsKUnknown)
{
    EXPECT_EQ(kUnknown, fileType(ut_utils::prepareFile("/etc/hostname")));
}

TEST(UT_Global, FileType_NonExistentFile_ReturnsKUnknown)
{
    EXPECT_EQ(kUnknown, fileType("/nonexistent-dir/nonexistent-file.pdf"));
}

TEST(UT_Global, SetMainWidget_GetMainDialog_ReturnsSameWidget)
{
    QWidget widget;
    setMainWidget(&widget);
    EXPECT_EQ(&widget, getMainDialog());
}

TEST(UT_Global, SetMainWidgetNull_GetMainDialog_ReturnsNull)
{
    setMainWidget(nullptr);
    EXPECT_EQ(nullptr, getMainDialog());
}
