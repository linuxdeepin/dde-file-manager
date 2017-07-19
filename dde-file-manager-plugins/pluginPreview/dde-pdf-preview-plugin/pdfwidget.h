#ifndef PDFWIDGET_H
#define PDFWIDGET_H

#include <QWidget>
#include <QSharedPointer>
#include <QListWidget>
#include <QLabel>

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
    explicit PdfWidget(const QString &file, QWidget *parent = 0);
    ~PdfWidget();

    void initDoc(const QString &file);

    void initUI();
    void initConnections();

    void showBadPage();

public slots:
    void onThumbAdded(int index, QImage img);
    void onpageAdded(int index, QImage img);
    void onThumbScrollBarValueChanged(const int& val);
    void onPageScrollBarvalueChanged(const int& val);
    void startLoadCurrentPages();
    void startLoadCurrentThumbs();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:

    void renderBorder(QImage& img);
    void emptyBorder(QImage& img);

    void loadPageSync(const int& index);
    void loadThumbSync(const int& index);
    void initEmptyPages();

    void resizeCurrentPage();

    QSharedPointer<PdfWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), PdfWidget)
};

class PdfInitWorker: public QObject{
    Q_OBJECT
public:
    explicit PdfInitWorker(QSharedPointer<poppler::document> doc, QObject* parent = 0);

    void startGetPageThumb(int index);
    void startGetPageImage(int index);

signals:
    void pageAdded(const int& index, const QImage& img);
    void thumbAdded(const int& index, const QImage& img);

private:
    QImage getPageThumb(const QImage& img) const;
    QImage getRenderedPageImage(const int& index) const;

    QList<int> m_gotThumbIndexes;
    QList<int> m_gotPageIndexes;

    QSharedPointer<poppler::document> m_doc;
};

class DListWidget: public QListWidget{
    Q_OBJECT
public:
    explicit DListWidget(QWidget* parent = 0);
protected:
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

};


#endif // PDFWIDGET_H
