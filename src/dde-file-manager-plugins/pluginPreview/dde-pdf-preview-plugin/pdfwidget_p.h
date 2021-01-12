#ifndef PDFWIDGET_P_H
#define PDFWIDGET_P_H

#include <QHBoxLayout>
#include <QButtonGroup>

#include "pdfwidget.h"

class PdfWidgetPrivate{
public:
    explicit PdfWidgetPrivate(PdfWidget* qq):
        q_ptr(qq){}

    DListWidget* thumbListWidget = nullptr;
    DListWidget* pageListWidget = nullptr;
    QHBoxLayout* mainLayout = nullptr;
    QScrollBar* thumbScrollBar = nullptr;
    QScrollBar* pageScrollBar = nullptr;
    QButtonGroup* thumbButtonGroup = nullptr;

    QTimer* pageWorkTimer = nullptr;
    QTimer* thumbWorkTimer = nullptr;
    bool isBadDoc = false;

    QSharedPointer<poppler::document> doc;

    PdfInitWorker* pdfInitWorker = nullptr;
    QMap<int, QImage> pageMap;

    PdfWidget* q_ptr = nullptr;
    Q_DECLARE_PUBLIC(PdfWidget)
};

#endif // PDFWIDGET_P_H
