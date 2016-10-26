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

#include "dabstractfileinfo.h"

#include <QTimer>
#include <QPointer>

class DAbstractFileInfoPrivate : public DAbstractFileInfoPrivateBase
{
public:
    ~DAbstractFileInfoPrivate() Q_DECL_OVERRIDE;

    DUrl url;
    mutable QString pinyinName;
//    mutable QIcon icon;
    mutable QPointer<QTimer> getIconTimer;
    bool requestingThumbnail = false;

    DAbstractFileInfoPointer proxy;

    static QSet<QString> hasThumbnailMimeHash;

protected:
    DAbstractFileInfoPrivate(const DUrl &url);

private:
    static QMap<DUrl, DAbstractFileInfoPrivate*> urlToFileInfoMap;

    friend class DAbstractFileInfo;
};

#endif // DABSTRACTFILEINFO_P_H
