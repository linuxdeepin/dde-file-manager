// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMACTIONBUTTON_H
#define DFMACTIONBUTTON_H

#include <QToolButton>


class DFMActionButton : public QToolButton
{
    Q_OBJECT

public:
    explicit DFMActionButton(QWidget *parent = nullptr);

    void setAction(QAction *action);
    QAction *action() const;
};

#endif // DFMACTIONBUTTON_H
