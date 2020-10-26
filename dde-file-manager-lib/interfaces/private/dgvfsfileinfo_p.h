/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DGVFSFILEINFO_P_H
#define DGVFSFILEINFO_P_H

#include "dfileinfo_p.h"

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>

class DGvfsFileInfo;
class RequestGvfsEP;
class DGvfsFileInfoPrivate : public DFileInfoPrivate
{
public:
    DGvfsFileInfoPrivate(const DUrl &url, DGvfsFileInfo *qq, bool hasCache = true);
    ~DGvfsFileInfoPrivate();

    mutable qint8 cacheFileExists = -1;
    mutable qint8 cacheCanRename = -1;
    mutable qint8 cacheIsSymLink = -1;
    mutable qint8 cacheCanWrite = -1;//fix bug 27828 打开挂载文件（有很多的文件夹和文件）在断网的情况下，滑动鼠标或者滚动鼠标滚轮时文管卡死，做缓存
    mutable qint8 cacheIsDir = -1;// 缓存是否是dir
    mutable long cacheModifyTime = -1;// bug 27247 远程连接FTP,使用大小排序反应很慢
    mutable long cacheReadTime = -1;// bug 27247 远程连接FTP,使用大小排序反应很慢
    mutable qint32 cacheFileCount = -1;// bug 27247 远程连接FTP,使用大小排序反应很慢
    mutable qint64 cacheFileSize = -1; // bug 27247 远程连接FTP,使用大小排序反应很慢


    mutable int ownerid = -1;//当前文件的所以
};

Q_DECLARE_METATYPE(DGvfsFileInfoPrivate *)

#endif // DGVFSFILEINFO_P_H
