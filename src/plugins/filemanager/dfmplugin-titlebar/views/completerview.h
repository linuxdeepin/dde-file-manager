// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMCOMPLETERVIEW_H
#define DFMCOMPLETERVIEW_H

#include "dfmplugin_titlebar_global.h"

#include <DListView>
#include <QCompleter>

namespace dfmplugin_titlebar {
class CompleterView : public DTK_WIDGET_NAMESPACE::DListView
{
    Q_OBJECT
public:
    explicit CompleterView(QWidget *parent = nullptr);
    void keyPressEvent(QKeyEvent *e) override;

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

signals:
    void listCurrentChanged(const QModelIndex &current);
    void listSelectionChanged(const QItemSelection &selected);
};
}

#endif   //DFMCOMPLETERVIEW_H
