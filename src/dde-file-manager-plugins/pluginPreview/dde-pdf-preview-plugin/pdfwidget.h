// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDFWIDGET_H
#define PDFWIDGET_H

#include <QWidget>
#include <QSharedPointer>
#include <QListWidget>
#include <QLabel>
#include <QFuture>

#include "poppler-document.h"
#include "poppler-page.h"
#include "poppler-page-renderer.h"

#define DEFAULT_VIEW_SIZE QSize(700, 800)
#define DEFAULT_THUMB_SIZE QSize(55, 74)
#define DEFAULT_PAGE_SIZE QSize(800, 1200)
#define DISPLAY_THUMB_NUM 10
#define DISPLAT_PAGE_NUM 5

class PdfWidgetPrivate;
class PdfInitWorker;
class PdfWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PdfWidget(const QString &file, QWidget *parent = nullptr);
    ~PdfWidget() override;

    void initDoc(const QString &file);

    void initUI();
    void initConnections();

    void showBadPage();

    /**
     * @brief getCanRelease 对象是否可以被删除
     * @return 如果返回true对象可删除，反之亦然
     */
    bool getCanRelease();

    /**
     * @brief setNeedRelease 设置对象是否要被删除
     * @param need true对象要被删除，反之亦然
     */
    void setNeedRelease(const bool & need);

public slots:
    void onThumbAdded(int index, QImage img);
    void onpageAdded(int index, QImage img);
    void onThumbScrollBarValueChanged(const int &val);
    void onPageScrollBarvalueChanged(const int &val);
    void startLoadCurrentPages();
    void startLoadCurrentThumbs();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:

    void renderBorder(QImage &img);
    QPixmap renderRadius(const QImage &img,int radius);
    void emptyBorder(QImage &img);

    void loadPageAsync(const int &index);
    void loadThumbAsync(const int &index);
    void initEmptyPages();

    int resizeCurrentPage();

    QSharedPointer<PdfWidgetPrivate> d_ptr;
    QFuture<void> threadPage;
    QFuture<void> threadThumb;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), PdfWidget)
};

class PdfInitWorker: public QObject
{
    Q_OBJECT
public:
    explicit PdfInitWorker(QSharedPointer<poppler::document> doc, QObject *parent = nullptr);

    void startGetPageThumb(int index);
    void startGetPageImage(int index);

signals:
    void pageAdded(const int &index, const QImage &img);
    void thumbAdded(const int &index, const QImage &img);

private:
    QImage getPageThumb(const QImage &img) const;
    QImage getRenderedPageImage(const int &index) const;

    QList<int> m_gotThumbIndexes;
    QList<int> m_gotPageIndexes;

    QSharedPointer<poppler::document> m_doc;
};

class DListWidget: public QListWidget
{
    Q_OBJECT
public:
    explicit DListWidget(QWidget *parent = nullptr);
protected:
    void mouseMoveEvent(QMouseEvent *e) override;
};


#endif // PDFWIDGET_H
