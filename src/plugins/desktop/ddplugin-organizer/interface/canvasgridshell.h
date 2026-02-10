// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASGRIDSHELL_H
#define CANVASGRIDSHELL_H

#include <QObject>

namespace ddplugin_organizer {

class CanvasGridShell : public QObject
{
    Q_OBJECT
public:
    explicit CanvasGridShell(QObject *parent = nullptr);
    ~CanvasGridShell();
    bool initialize();

public:
    QString item(int index, const QPoint &gridPos);
    void tryAppendAfter(const QStringList &items, int index, const QPoint &begin);
    int point(const QString &item, QPoint *pos);
};

}

#endif // CANVASGRIDSHELL_H
