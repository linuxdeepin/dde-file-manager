// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYSELECTOR_H
#define KEYSELECTOR_H

#include "ddplugin_canvas_global.h"
#include "clickselector.h"

#include <QObject>
#include <QModelIndex>

class QTimer;
class QKeyEvent;
namespace ddplugin_canvas {
class CanvasView;
class KeySelector : public ClickSelector
{
    Q_OBJECT
public:
    explicit KeySelector(CanvasView *parent);
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
    void toggleSelect();
protected:
    QString searchKeys;
    QTimer *searchTimer;
};

}
#endif // KEYSELECTOR_H
