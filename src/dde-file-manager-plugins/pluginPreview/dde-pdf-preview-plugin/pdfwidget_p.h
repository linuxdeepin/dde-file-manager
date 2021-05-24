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
    //! 正在执行的线程数量
    QAtomicInt m_threadRunningCount = 0;
    //! 对象是否要被删除
    QAtomicInteger<bool> m_needRelease = false;

    PdfWidget* q_ptr = nullptr;
    Q_DECLARE_PUBLIC(PdfWidget)
};

#endif // PDFWIDGET_P_H
