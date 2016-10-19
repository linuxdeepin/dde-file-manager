/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DFILEINFO_P_H
#define DFILEINFO_P_H

#include "dabstractfileinfo_p.h"

#include <QFileInfo>

class DFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    QFileInfo fileInfo;
    mutable QMimeType mimeType;

protected:
    DFileInfoPrivate(const DUrl &url);

    friend class DFileInfo;
};

#endif // DFILEINFO_P_H
