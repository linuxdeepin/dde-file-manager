// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKGLOBAL_H
#define DISKGLOBAL_H
#include <QObject>
#include <dfmglobal.h>

class DiskGlobal : public QObject
{
public:
    explicit DiskGlobal(QObject *parent = nullptr);

    static bool isWayLand();
};

#endif // DFMGLOBAL_H
