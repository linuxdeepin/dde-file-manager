/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-27

 */


#include <gtest/gtest.h>

#ifdef GTEST
#define private public
#define protected public
#endif

#include "pdfwidget.h"
#include "durl.h"

#include <QDebug>

namespace  {
class TestPdfInitWorker : public testing::Test
{
public:
    void SetUp() override
    {
        QString file = "../../../../dde-file-manager/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/tests/test.pdf";
        QSharedPointer<poppler::document> doc = QSharedPointer<poppler::document>(poppler::document::load_from_file(file.toStdString()));

        if (!doc || doc->is_locked()) {
            qDebug () << "Cannot read this pdf file: " << file;
        }

        m_pdfInitworker = new PdfInitWorker(doc);
    }

    void TearDown() override
    {
        delete m_pdfInitworker;
        m_pdfInitworker = nullptr;
    }

public:
    PdfInitWorker * m_pdfInitworker;
    DUrl m_url;
};
}

TEST_F(TestPdfInitWorker, get_page_thumb)
{
    QImage img = m_pdfInitworker->getRenderedPageImage(0);

    if(!img.isNull()){
       EXPECT_TRUE(!m_pdfInitworker->getPageThumb(img).isNull());
    }
}

TEST_F(TestPdfInitWorker, get_rendered_page_image)
{
    EXPECT_TRUE(!m_pdfInitworker->getRenderedPageImage(0).isNull());
}
