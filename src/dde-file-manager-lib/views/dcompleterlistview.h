// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCOMPLETERLISTVIEW_H
#define DCOMPLETERLISTVIEW_H

#include <QListView>
#include <QTimer>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DCompleterListView : public QListView
{
    Q_OBJECT

public:
    explicit DCompleterListView(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *e) override;

    void showMe();
public slots:
    void hideMe();
protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

signals:
    void listCurrentChanged(const QModelIndex &current);
    void listSelectionChanged(const QItemSelection &selected);

};

DFM_END_NAMESPACE

#endif // DCOMPLETERLISTVIEW_H
