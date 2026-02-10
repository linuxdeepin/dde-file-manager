// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSWIDGET_H
#define TIPSWIDGET_H

#include <QFrame>

class QPaintEvent;

class TipsWidget : public QFrame
{
    Q_OBJECT

public:
    explicit TipsWidget(QWidget *parent = nullptr);

    void setText(const QString &text);
    void refreshFont();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString curText;
};

#endif   // TIPSWIDGET_H
