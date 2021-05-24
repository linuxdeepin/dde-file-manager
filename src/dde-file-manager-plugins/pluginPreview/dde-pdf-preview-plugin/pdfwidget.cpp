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

#include "pdfwidget.h"

#include <QImage>
#include <QHBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QtMath>
#include <QUrl>
#include <QLabel>
#include <QListWidgetItem>
#include <QThread>
#include <QtConcurrent>
#include <QScrollBar>
#include <QResizeEvent>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QPushButton>

#include "pdfwidget_p.h"

PdfWidget::PdfWidget(const QString &file, QWidget *parent) :
    QWidget(parent),
    d_ptr(new PdfWidgetPrivate(this))
{
    Q_D(PdfWidget);

    d->pageWorkTimer = new QTimer(this);
    d->pageWorkTimer->setSingleShot(true);
    d->pageWorkTimer->setInterval(50);
    d->thumbWorkTimer = new QTimer(this);
    d->thumbWorkTimer->setSingleShot(true);
    d->thumbWorkTimer->setInterval(100);

    d->thumbButtonGroup = new QButtonGroup(this);


    initDoc(file);
    initUI();

    if(d->isBadDoc){
        return;
    }

    initConnections();
}

PdfWidget::~PdfWidget()
{
    Q_D(PdfWidget);

    disconnect(d->pdfInitWorker, &PdfInitWorker::thumbAdded, this, &PdfWidget::onThumbAdded);
    disconnect(d->pdfInitWorker, &PdfInitWorker::pageAdded, this, &PdfWidget::onpageAdded);
    this->hide();

    d->pdfInitWorker->deleteLater();
}

void PdfWidget::initDoc(const QString& file)
{
    Q_D(PdfWidget);
    d->doc = QSharedPointer<poppler::document>(poppler::document::load_from_file(file.toStdString()));

    if (!d->doc || d->doc->is_locked()) {
        qDebug () << "Cannot read this pdf file: " << file;
        d->isBadDoc = true;
    }

    d->pdfInitWorker = new PdfInitWorker(d->doc);
}

void PdfWidget::initUI()
{
    Q_D(PdfWidget);

    if(d->isBadDoc){
        showBadPage();
        return;
    }

    setContentsMargins(0, 0, 0, 0);
    setFixedSize(qMin(DEFAULT_VIEW_SIZE.width(), static_cast<int>(qApp->desktop()->width() * 0.8)),
                 qMin(DEFAULT_VIEW_SIZE.height(), static_cast<int>(qApp->desktop()->height() * 0.8)));

    d->thumbListWidget = new DListWidget(this);
    d->thumbListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->thumbListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->thumbScrollBar = d->thumbListWidget->verticalScrollBar();
    d->thumbScrollBar->setParent(this);
    d->thumbListWidget->setFixedWidth(96);
    d->thumbListWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    d->thumbListWidget->setAttribute(Qt::WA_MouseTracking);
    d->thumbListWidget->setStyleSheet("QListWidget{"
                                        "border: none;"
                                        "background: white;"
                                        "border-right: 1px solid rgba(0, 0, 0, 0.1);"
                                      "}"
                                      "QListWidget::item{"
                                        "border: none;"
                                      "}");

    d->thumbListWidget->setSpacing(18);

    d->pageListWidget = new DListWidget(this);
    d->pageListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->pageListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->pageListWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    d->pageListWidget->setStyleSheet("QListWidget::item:selected{"
                                        "background: white;"
                                     "}");
    d->pageScrollBar = d->pageListWidget->verticalScrollBar();
    d->pageScrollBar->setParent(this);

    d->mainLayout = new QHBoxLayout;
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);
    d->mainLayout->addWidget(d->thumbListWidget);
    d->mainLayout->addWidget(d->pageListWidget);

    setLayout(d->mainLayout);

    initEmptyPages();

    loadThumbSync(0);
    loadPageSync(0);
}

void PdfWidget::initConnections()
{
    Q_D(PdfWidget);

    connect(d->pdfInitWorker, &PdfInitWorker::thumbAdded, this, &PdfWidget::onThumbAdded);
    connect(d->pdfInitWorker, &PdfInitWorker::pageAdded, this, &PdfWidget::onpageAdded);

    connect(d->thumbScrollBar, &QScrollBar::valueChanged, this, &PdfWidget::onThumbScrollBarValueChanged);
    connect(d->pageScrollBar, &QScrollBar::valueChanged, this, &PdfWidget::onPageScrollBarvalueChanged);

    connect(d->pageWorkTimer, &QTimer::timeout, this, &PdfWidget::startLoadCurrentPages);
    connect(d->thumbWorkTimer, &QTimer::timeout, this, &PdfWidget::startLoadCurrentThumbs);
}

void PdfWidget::showBadPage()
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setParent(this);
    QLabel* badLabel = new QLabel(this);
    badLabel->setStyleSheet("QLabel{"
                                "font-size: 20px;"
                            "}");
    badLabel->setText(tr("Cannot preview this file!"));

    layout->addStretch();
    layout->addWidget(badLabel, 0, Qt::AlignHCenter);
    layout->addStretch();
    setLayout(layout);
}

bool PdfWidget::getCanRelease()
{
    Q_D(PdfWidget);

    return d->m_threadRunningCount <= 0;
}

void PdfWidget::setNeedRelease(const bool &need)
{
    Q_D(PdfWidget);

    d->m_needRelease = need;
}

void PdfWidget::onThumbAdded(int index, QImage img)
{
    Q_D(PdfWidget);
    QListWidgetItem* item = d->thumbListWidget->item(index);
    QWidget* w = d->thumbListWidget->itemWidget(item);

    if(!w){
        QPushButton* bnt = new QPushButton(this);
        d->thumbButtonGroup->addButton(bnt);
        bnt->setIcon(QIcon(QPixmap::fromImage(img)));
        bnt->setFixedSize(img.size());
        bnt->setIconSize(QSize(img.width() - 4, img.height()));
        bnt->setCheckable(true);
        bnt->setStyleSheet("QPushButton{"
                            "border: 1px solid rgba(0, 0, 0, 0.2);"
                           "}"
                           "QPushButton:checked{"
                            "border: 2px solid #2ca7f8;"
                           "}");

        if(index == 0){
            bnt->setChecked(true);
        }

        connect(bnt, &QPushButton::clicked, [=]{
            bnt->setChecked(true);
            int row = d->thumbListWidget->row(item);
            d->pageListWidget->setCurrentRow(row);

        });

        d->thumbListWidget->setItemWidget(item, bnt);
        item->setSizeHint(img.size());
    }

    if(d->thumbScrollBar->maximum() == 0){
        d->thumbScrollBar->hide();
    } else {
        d->thumbScrollBar->show();
    }
}

void PdfWidget::onpageAdded(int index, QImage img)
{
    Q_D(PdfWidget);

    d->pageMap.insert(index, img);

    QListWidgetItem* item = d->pageListWidget->item(index);
    QWidget* w = d->pageListWidget->itemWidget(item);

    if(!w){
        img = img.scaled(d->pageListWidget->width(), img.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QImage page(d->pageListWidget->width(), img.height() + 4, QImage::Format_ARGB32_Premultiplied);
        page.fill(Qt::white);
        QPainter p(&page);
        p.drawImage((page.width() - img.width())/2, 2, img);
        if(index < (d->doc->pages() - 1)){
            QPen pen(QColor(0, 0, 0 , 20));
            p.setPen(pen);
            p.drawLine(0, page.height() - 1, page.width(), page.height() - 1);
        }

        QLabel* pageLabel = new QLabel(this);
        pageLabel->setPixmap(QPixmap::fromImage(page));

        d->pageListWidget->setItemWidget(item, pageLabel);
        item->setSizeHint(page.size());
    }

    if(d->pageScrollBar->maximum() == 0){
        d->pageScrollBar->hide();
    } else {
        d->pageScrollBar->show();
    }

}

void PdfWidget::onThumbScrollBarValueChanged(const int &val)
{
    Q_UNUSED(val)
    Q_D(const PdfWidget);

    d->thumbWorkTimer->stop();
    d->thumbWorkTimer->start();
}

void PdfWidget::onPageScrollBarvalueChanged(const int &val)
{
    Q_UNUSED(val)
    Q_D(const PdfWidget);

    d->pageWorkTimer->stop();
    d->pageWorkTimer->start();

    resizeCurrentPage();

    QListWidgetItem* item = d->pageListWidget->itemAt(d->pageListWidget->width() /2 , 20);
    if(!item) {
        return;
    }

    int row = d->pageListWidget->row(item);
    d->thumbListWidget->setCurrentRow(row);
    QListWidgetItem* thumbItem = d->thumbListWidget->item(row);
    if(!thumbItem){
        return;
    }

    QWidget* w = d->thumbListWidget->itemWidget(thumbItem);
    if(!w){
        return;
    }

    QPushButton* bnt = qobject_cast<QPushButton*>(w);
    bnt->setChecked(true);
}

void PdfWidget::startLoadCurrentPages()
{
    Q_D(const PdfWidget);
    QListWidgetItem* item = d->pageListWidget->itemAt(d->pageListWidget->width() / 2, 0);
    if(!item){
        item = d->pageListWidget->itemAt(d->pageListWidget->width() / 2, d->pageListWidget->spacing() * 2 + 1);
    }
    if(item)
    {
        int row = d->pageListWidget->row(item);
        loadPageSync(row);
    }
}

void PdfWidget::startLoadCurrentThumbs()
{
    Q_D(const PdfWidget);
    QListWidgetItem* item = d->thumbListWidget->itemAt(d->thumbListWidget->width() / 2, 0);
    //To prevent this point is int empty area, we get another point again with next pixcel that lager than it spacing
    if(!item){
        item = d->thumbListWidget->itemAt(d->thumbListWidget->width() / 2, d->thumbListWidget->spacing() * 2  + 1);
    }

    if(item)
    {
        int row = d->thumbListWidget->row(item);
        loadThumbSync(row);
    }
}

void PdfWidget::resizeEvent(QResizeEvent *event)
{
    Q_D(PdfWidget);

    QWidget::resizeEvent(event);

    if(d->isBadDoc){
        return;
    }

    if(d->pageScrollBar->maximum() == 0){
        d->pageScrollBar->hide();
    } else {
        d->pageScrollBar->show();
    }

    if(d->thumbScrollBar->maximum() == 0){
        d->thumbScrollBar->hide();
    } else {
        d->thumbScrollBar->show();
    }

    d->thumbScrollBar->setFixedSize(d->thumbScrollBar->sizeHint().width(), event->size().height() - 10);
    d->thumbScrollBar->move(d->thumbListWidget->width() - d->thumbScrollBar->width(), 10);

    d->pageScrollBar->setFixedSize(d->pageScrollBar->sizeHint().width(), event->size().height() - 30);
    d->pageScrollBar->move(event->size().width() - d->pageScrollBar->width(), 30);
    d->pageListWidget->setFixedWidth(width() - d->thumbListWidget->width());

    resizeCurrentPage();
}

void PdfWidget::renderBorder(QImage &img)
{
    QColor color(0, 0, 0, 30);

    QPainter painter(&img);
    QPen pen;
    pen.setColor(color);
    pen.setWidth(1);
    painter.setPen(pen);

    painter.drawRect(0, 0, img.width() - 1, img.height() -1);
}

void PdfWidget::emptyBorder(QImage &img)
{
    QColor color(255, 255, 255);

    QPainter painter(&img);
    QPen pen;
    pen.setColor(color);
    pen.setWidth(2);
    painter.setPen(pen);

    painter.drawRect(0, 0, img.width() - 2, img.height() - 2);
}

void PdfWidget::loadPageSync(const int &index)
{
    Q_D(PdfWidget);

    QPointer<PdfWidget> mePtr = this;

    QtConcurrent::run([=]{
        if (mePtr.isNull() || d->m_needRelease)
            return;
        d->m_threadRunningCount++;
        d->pdfInitWorker->startGetPageImage(index);
        d->m_threadRunningCount--;
        if (d->m_needRelease && d->m_threadRunningCount <= 0)
            this->deleteLater();
    });
}

void PdfWidget::loadThumbSync(const int &index)
{
    Q_D(PdfWidget);

    QPointer<PdfWidget> mePtr = this;

    QtConcurrent::run([=]{
        if (mePtr.isNull() || d->m_needRelease)
            return;
        d->m_threadRunningCount++;
        d->pdfInitWorker->startGetPageThumb(index);
        d->m_threadRunningCount--;
        if (d->m_needRelease && d->m_threadRunningCount <= 0)
            this->deleteLater();
    });
}

void PdfWidget::initEmptyPages()
{
    Q_D(PdfWidget);

    for(int i = 0; i < d->doc->pages(); i ++ ){
        QListWidgetItem* pageItem = new QListWidgetItem;
        pageItem->setSizeHint(DEFAULT_PAGE_SIZE);

        QListWidgetItem* thumbItem = new QListWidgetItem;
        thumbItem->setSizeHint(DEFAULT_THUMB_SIZE);

        d->pageListWidget->addItem(pageItem);
        d->thumbListWidget->addItem(thumbItem);
    }
}

void PdfWidget::resizeCurrentPage()
{
    Q_D(PdfWidget);

    QListWidgetItem* currentItem = d->pageListWidget->itemAt(d->pageListWidget->width() / 2, d->pageListWidget->height() / 2);

    if(!currentItem)
        return;

    int currentRow = d->pageListWidget->row(currentItem);
    int index = currentRow - DISPLAT_PAGE_NUM/2;
    if(index < 0){
        index = 0;
    }
    int counter = 0;
    while(counter < DISPLAT_PAGE_NUM){
        counter ++;

        if(d->pageMap.contains(index)){

            QListWidgetItem* item = d->pageListWidget->item(index);
            if(!item){
                continue;
            }

            QWidget* w = d->pageListWidget->itemWidget(item);
            if(!w){
                continue;
            }

            QLabel* label = qobject_cast<QLabel*>(w);

            QImage img = d->pageMap.value(index);
            img = img.scaled(d->pageListWidget->width(), img.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QImage page(d->pageListWidget->width(), img.height() + 4, QImage::Format_ARGB32_Premultiplied);
            page.fill(Qt::white);

            QPainter p(&page);
            p.drawImage((page.width() - img.width()) / 2, 2, img);

            if(index < (d->doc->pages() - 1)){
                QPen pen(QColor(0, 0, 0 , 20));
                p.setPen(pen);
                p.drawLine(0, page.height() - 1, page.width(), page.height() - 1);
            }

            label->setPixmap(QPixmap::fromImage(page));
            item->setSizeHint(page.size());

            index ++;
        } else {
            index ++;
            continue;
        }
    }
}

PdfInitWorker::PdfInitWorker(QSharedPointer<poppler::document> doc, QObject *parent):
    QObject(parent),
    m_doc(doc)
{

}

void PdfInitWorker::startGetPageThumb(int index)
{
    int counter = 0;
    while(counter < DISPLAY_THUMB_NUM){
        counter++;

        //Skip for indexed thumb we got
        if(m_gotThumbIndexes.contains(index)){
            index ++;
            continue;
        }

        QImage img = getRenderedPageImage(index);

        if(img.isNull()){
            break;
        } else{
            QImage thumb = getPageThumb(img);
            emit thumbAdded(index, thumb);
            m_gotThumbIndexes << index;
            index ++;
        }

    }
}

void PdfInitWorker::startGetPageImage(int index)
{
    int counter = 0;
    while(counter < DISPLAT_PAGE_NUM){
        counter++;

        //Skip for indexed page we got
        if(m_gotPageIndexes.contains(index)){
            index ++;
            continue;
        }

        QImage img = getRenderedPageImage(index);

        if(img.isNull()){
            break;
        } else {
            emit pageAdded(index, img);
            m_gotPageIndexes << index;
            index ++;
        }
    }
}

QImage PdfInitWorker::getPageThumb(const QImage &img) const
{
    QImage newImg = img;
    return newImg.scaled(DEFAULT_THUMB_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QImage PdfInitWorker::getRenderedPageImage(const int &index) const
{
    QImage img;

    QSharedPointer<poppler::page> page = QSharedPointer<poppler::page>(m_doc->create_page(index));

    if (!page) {
        return img;
    }

    poppler::page_renderer pr;
    pr.set_render_hint(poppler::page_renderer::antialiasing, true);
    pr.set_render_hint(poppler::page_renderer::text_antialiasing, true);

    if(!pr.can_render()){
        qDebug () << "Cannot render page";
        return img;
    }

    if(page->page_rect().width() * page->page_rect().height() > 1920 * 1080 * 3){
        qDebug () << "This pdf page is tool large, ignore...";
        return img;
    }

    poppler::image imageData = pr.render_page(page.data());

    if (!imageData.is_valid()) {
        qDebug () << "Render error";
        return img;
    }

    poppler::image::format_enum format = imageData.format();

    switch (format) {
    case poppler::image::format_invalid:
        qDebug ()  << "Image format is invalid";
        return img;
    case poppler::image::format_mono:
        img = QImage((uchar*)imageData.data(), imageData.width(), imageData.height(), QImage::Format_Mono);
        break;
    case poppler::image::format_rgb24:
        img = QImage((uchar*)imageData.data(),imageData.width(),imageData.height(),QImage::Format_ARGB6666_Premultiplied);
        break;
    case poppler::image::format_argb32:
    {
        //Note that this format has problem that QImage cannot parse it normally
        img = QImage(imageData.width(), imageData.height(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        int w = imageData.width();
        int h = imageData.height();
        int offset = imageData.bytes_per_row() / imageData.width();

        char* pdata = imageData.data();

        for(int y = 0; y < h; y ++){
            for(int x = 0; x < w; x++){
                uchar b = static_cast<uchar> (pdata[(x * offset) + (y * w * offset)]);
                uchar g = static_cast<uchar> (pdata[(x * offset) + (y * w * offset) + 1]);
                uchar r = static_cast<uchar> (pdata[(x * offset) + (y * w * offset) + 2]);
                uchar a = static_cast<uchar> (pdata[(x * offset) + (y * w * offset) + 3]);

                img.setPixelColor(x, y, QColor((int)r, (int)g, (int)b, (int)a));
            }
        }
        break;
    }
    default:
        break;
    }

    return img;
}

DListWidget::DListWidget(QWidget *parent):
    QListWidget(parent)
{

}

void DListWidget::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
}
