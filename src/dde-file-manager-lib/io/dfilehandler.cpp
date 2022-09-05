// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
