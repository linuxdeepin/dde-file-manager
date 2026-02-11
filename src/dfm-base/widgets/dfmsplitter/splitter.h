// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
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
    Q_PROPERTY(int splitPosition READ splitPosition WRITE setSplitPosition)

public:
    explicit Splitter(Qt::Orientation orientation, QWidget *parent = nullptr);

    int splitPosition() const;
    void setSplitPosition(int position);

protected:
    QSplitterHandle *createHandle() override;

private:
    int curSplitPosition { 0 };
};
}

#endif   // Splitter_H
