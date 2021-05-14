/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <gtest/gtest.h>
#include <QPrinter>
#include <QPainter>
#include <QFile>
#include <QThread>

#include "pdfwidget.h"
#include "pdfwidget_p.h"
#include "durl.h"

#include "stub.h"
#include "addr_pri.h"

#include <QDebug>
#include <QEventLoop>
#include <QTimer>
namespace  {
class TestPdfWidget : public testing::Test
{
public:
    void SetUp() override
    {
        QPrinter text_printer;
        text_printer.setOutputFormat(QPrinter::PdfFormat);
        text_printer.setOutputFileName("./test.pdf");
        QPainter painter_pixmap;
        painter_pixmap.begin(&text_printer);
        painter_pixmap.drawText(10, 30, "hello world");
        painter_pixmap.end();
        QString file = "./test.pdf";
        m_pdfWidget = new PdfWidget(file);
    }

    void TearDown() override
    {
	QThread::sleep(5);	
	delete m_pdfWidget;
        m_pdfWidget = nullptr;

        QFile file("./test.pdf");
        file.remove();

        QEventLoop loop;
        QTimer::singleShot(10, &loop, [&loop]{
            loop.exit();
        });
        loop.exec();
    }

public:
    PdfWidget * m_pdfWidget;
    DUrl m_url;
};
}
#ifndef __arm__
TEST_F(TestPdfWidget, use_showBadPage)
{
    EXPECT_NO_FATAL_FAILURE(m_pdfWidget->showBadPage());
}
#endif
TEST_F(TestPdfWidget, slot_onThumbScrollBarValueChanged)
{
    EXPECT_NO_FATAL_FAILURE(m_pdfWidget->onThumbScrollBarValueChanged(10));
}

TEST_F(TestPdfWidget, slot_onPageScrollBarvalueChanged)
{
    EXPECT_NO_FATAL_FAILURE(m_pdfWidget->onPageScrollBarvalueChanged(10));
}

TEST_F(TestPdfWidget, slot_startLoadCurrentPages)
{
    EXPECT_NO_FATAL_FAILURE(m_pdfWidget->startLoadCurrentPages());
}

TEST_F(TestPdfWidget, slot_startLoadCurrentThumbs)
{
    EXPECT_NO_FATAL_FAILURE(m_pdfWidget->startLoadCurrentThumbs());
}

ACCESS_PRIVATE_FIELD(PdfWidget, QSharedPointer<PdfWidgetPrivate>, d_ptr)
TEST_F(TestPdfWidget, get_startGetPageThumb)
{
    QSharedPointer<PdfWidgetPrivate> temp = access_private_field::PdfWidgetd_ptr(*m_pdfWidget);
    EXPECT_NO_FATAL_FAILURE(temp->pdfInitWorker->startGetPageThumb(0));
}

TEST_F(TestPdfWidget, get_startGetPageImage)
{
    QSharedPointer<PdfWidgetPrivate> temp = access_private_field::PdfWidgetd_ptr(*m_pdfWidget);
    EXPECT_NO_FATAL_FAILURE(temp->pdfInitWorker->startGetPageImage(0));
}


ACCESS_PRIVATE_FUN(PdfInitWorker, QImage(const int &index)const, getRenderedPageImage)
QImage PrivateGetRenderedPageImage(PdfInitWorker * initWorker, const int &index)
{
    return call_private_fun::PdfInitWorkergetRenderedPageImage(*initWorker, index);
}
TEST_F(TestPdfWidget, used_onpageAdded)
{
    QSharedPointer<PdfWidgetPrivate> temp = access_private_field::PdfWidgetd_ptr(*m_pdfWidget);
    QImage img = PrivateGetRenderedPageImage(temp->pdfInitWorker,0);

    if(!img.isNull()){

        EXPECT_NO_FATAL_FAILURE(m_pdfWidget->onpageAdded(0, img));
    }
}

ACCESS_PRIVATE_FUN(PdfWidget, void(QImage &img), renderBorder)
void PrivateReaderBorder(PdfWidget * pdfWidget, QImage &img)
{
    call_private_fun::PdfWidgetrenderBorder(*pdfWidget, img);
}
TEST_F(TestPdfWidget, used_renderBorder)
{
    QSharedPointer<PdfWidgetPrivate> temp = access_private_field::PdfWidgetd_ptr(*m_pdfWidget);
    QImage img = PrivateGetRenderedPageImage(temp->pdfInitWorker,0);
    PrivateReaderBorder(m_pdfWidget, img);
}

ACCESS_PRIVATE_FUN(PdfWidget, void(QImage &img), emptyBorder)
void PrivateEmptyBorder(PdfWidget * pdfWidget, QImage &img)
{
    call_private_fun::PdfWidgetemptyBorder(*pdfWidget, img);
}
TEST_F(TestPdfWidget, used_emptyBorder)
{
    QSharedPointer<PdfWidgetPrivate> temp = access_private_field::PdfWidgetd_ptr(*m_pdfWidget);
    QImage img = PrivateGetRenderedPageImage(temp->pdfInitWorker,0);
    PrivateEmptyBorder(m_pdfWidget, img);
}
