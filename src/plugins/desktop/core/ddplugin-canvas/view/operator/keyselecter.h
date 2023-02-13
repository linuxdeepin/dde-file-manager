// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYSELECTER_H
#define KEYSELECTER_H

#include "ddplugin_canvas_global.h"
#include "clickselecter.h"

#include <QObject>
#include <QModelIndex>

class QTimer;
class QKeyEvent;
namespace ddplugin_canvas {
class CanvasView;
class KeySelecter : public ClickSelecter
{
    Q_OBJECT
public:
    explicit KeySelecter(CanvasView *parent);
    void keyPressed(QKeyEvent *event);
    QList<Qt::Key> filterKeys() const;
    void keyboardSearch(const QString &search);
protected:
    QPersistentModelIndex moveCursor(QKeyEvent *event) const;
    void singleSelect(const QModelIndex &index);
    void incrementSelect(const QModelIndex &index);
signals:

public slots:
    void clearSearchKey();
protected:
    QString searchKeys;
    QTimer *searchTimer;
};

}
#endif // KEYSELECTER_H
