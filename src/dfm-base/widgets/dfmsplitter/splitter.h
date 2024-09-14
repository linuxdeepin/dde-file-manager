// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef Splitter_H
#define Splitter_H

#include <dfm-base/dfm_base_global.h>

#include <QSplitter>
#include <QSplitterHandle>

namespace dfmbase {
class SplitterHandle : public QSplitterHandle
{
    Q_OBJECT
public:
    explicit SplitterHandle(Qt::Orientation orientation, QSplitter *parent);

protected:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent *) override;
#else
    void enterEvent(QEvent *) override;
#endif
    void leaveEvent(QEvent *) override;
};

// ---------- Item Get Border Line ----------

class Splitter : public QSplitter
{
    Q_OBJECT
public:
    explicit Splitter(Qt::Orientation orientation, QWidget *parent = nullptr);
    void moveSplitter(int pos, int index);

protected:
    QSplitterHandle *createHandle() override;
};
}

#endif   // Splitter_H
