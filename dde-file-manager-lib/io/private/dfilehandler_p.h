/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DFILEHANDLER_P_H
#define DFILEHANDLER_P_H

#include "dfilehandler.h"

DFM_BEGIN_NAMESPACE

class DFileHandlerPrivate
{
public:
    explicit DFileHandlerPrivate(DFileHandler *qq);

    void setErrorString(const QString &es);

    DFileHandler *q_ptr;
    QString errorString;
};

DFM_END_NAMESPACE

#endif // DFILEHANDLER_P_H
