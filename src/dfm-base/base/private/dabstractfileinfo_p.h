/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DABSTRACTFILEINFO_P_H
#define DABSTRACTFILEINFO_P_H

#include "base/dabstractfileinfo.h"

#include <QPointer>

class DAbstractFileInfoPrivate
{
    Q_DECLARE_PUBLIC(DAbstractFileInfo)

public:
    DAbstractFileInfoPrivate(DAbstractFileInfo *qq);
    virtual ~DAbstractFileInfoPrivate();

protected:
    QFileInfo fileInfo;
    QUrl url;
    DAbstractFileInfo * const q_ptr;
};





#endif // DABSTRACTFILEINFO_P_H
