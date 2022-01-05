/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILETREATER_P_H
#define FILETREATER_P_H

#include "filetreater.h"

DSB_D_BEGIN_NAMESPACE

class FileTreaterPrivate
{
public:
    explicit FileTreaterPrivate(FileTreater *q_ptr)
        : q(q_ptr) {}

public:
    QList<DFMLocalFileInfoPointer> fileList;
    QMap<QString, DFMLocalFileInfoPointer> fileHashTable;
    QString homePath;
    bool isDone { false };
    FileTreater *q { nullptr };
};

DSB_D_END_NAMESPACE
#endif   // FILETREATER_P_H
