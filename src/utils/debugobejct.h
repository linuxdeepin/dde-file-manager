// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGOBEJCT_H
#define DEBUGOBEJCT_H

#include <QObject>
#include <QDebug>

class DebugObejct : public QObject
{

public:
    explicit DebugObejct(QObject *parent = 0);

private:
    friend QDebug operator<<(QDebug dbg, const QObject &obj);
};

QDebug operator<<(QDebug dbg, const QObject &obj);

#endif // DEBUGOBEJCT_H
