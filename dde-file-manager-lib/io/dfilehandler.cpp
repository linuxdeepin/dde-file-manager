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
#include "dfilehandler.h"
#include "private/dfilehandler_p.h"

DFM_BEGIN_NAMESPACE

DFileHandlerPrivate::DFileHandlerPrivate(DFileHandler *qq)
    : q_ptr(qq)
{

}

void DFileHandlerPrivate::setErrorString(const QString &es)
{
    errorString = es;
}

DFileHandler::~DFileHandler()
{

}

QString DFileHandler::errorString() const
{
    Q_D(const DFileHandler);

    return d->errorString;
}

DFileHandler::DFileHandler()
    : DFileHandler(*new DFileHandlerPrivate(this))
{

}

DFileHandler::DFileHandler(DFileHandlerPrivate &dd)
    : d_ptr(&dd)
{

}

DFM_END_NAMESPACE
