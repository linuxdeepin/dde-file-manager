// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAGMONITER_H
#define DRAGMONITER_H

#include <QObject>
#include <QDBusContext>
namespace dfm_drag {
// NOTE: temp code!!!!!!!!!!!
class DragMoniter : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit DragMoniter(QObject *parent = nullptr);
    void start();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

signals:
    Q_SCRIPTABLE void dragEnter(const QStringList &urls);
};
}

#endif   // DRAGMONITER_H
