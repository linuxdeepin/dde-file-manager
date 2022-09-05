// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSPLITTER_H
#define DFMSPLITTER_H


#include <QSplitter>
#include <QSplitterHandle>

class DFMSplitterHandle: public QSplitterHandle
{
    Q_OBJECT

public:
    explicit DFMSplitterHandle(Qt::Orientation orientation, QSplitter* parent);

protected:
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
};

// ---------- Item Get Border Line ----------

class DFMSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit DFMSplitter(Qt::Orientation orientation, QWidget *parent = nullptr);
    void moveSplitter(int pos, int index);

protected:
    QSplitterHandle* createHandle() override;
};

#endif // DFMSPLITTER_H
