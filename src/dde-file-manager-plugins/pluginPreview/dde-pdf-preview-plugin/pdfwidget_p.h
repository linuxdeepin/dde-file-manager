// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    QTimer* pageWorkTimer = nullptr;
    QTimer* thumbWorkTimer = nullptr;
    bool isBadDoc = false;

    QSharedPointer<poppler::document> doc;

    PdfInitWorker* pdfInitWorker = nullptr;
    QMap<int, QImage> pageMap;
    //! 正在执行的线程数量
    QAtomicInt m_threadRunningCount = 0;
    //! 对象是否要被删除
    QAtomicInteger<bool> m_needRelease = false;

    PdfWidget* q_ptr = nullptr;
    Q_DECLARE_PUBLIC(PdfWidget)
};

#endif // PDFWIDGET_P_H
