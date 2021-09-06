/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef LOCALFILEINFO_P_H
#define LOCALFILEINFO_P_H

#include "base/private/abstractfileinfo_p.h"
#include "localfile/localfileinfo.h"
#include "shutil/mimedatabase.h"

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>
#include <QMimeType>
#include <QReadWriteLock>

DFMBASE_BEGIN_NAMESPACE
class LocalFileInfoPrivate : public AbstractFileInfoPrivate
{
    Q_DECLARE_PUBLIC(LocalFileInfo)
    LocalFileInfo *q_ptr;
public:
    LocalFileInfoPrivate(LocalFileInfo *qq);
    virtual ~LocalFileInfoPrivate();
    QString sizeString(const QString &str) const
    {
        int begin_pos = str.indexOf('.');

        if (begin_pos < 0)
            return str;

        QString size = str;

        while (size.count() - 1 > begin_pos) {
            if (!size.endsWith('0'))
                return size;

            size = size.left(size.count() - 1);
        }

        return size.left(size.count() - 1);
    }
    static QIcon standardFileIcon(LocalFileInfo::Icon type);
    void readStandardFlagsIcon();
    bool isLowSpeedFile() const;
    virtual QMimeType readMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const;

private:
    mutable QIcon icon; // 文件的icon
    mutable LocalFileInfo::DFMEmblemInfos EmblemsInfo;   // 文件的扩展属性
    mutable bool iconFromTheme = false; // icon是否是来自缩略图
    mutable bool requestingThumbnail = false;   // 请求缩略图
    mutable bool needThumbnail = false; // 是否需要缩略图
    mutable qint8 hasThumbnail = -1; // 是否拥有缩略图
    mutable qint8 lowSpeedFile = -1; // 是否是低速设备
    mutable bool epInitialized = false; // 扩展属性是否初始化
    mutable QMimeType mimeType; // 文件的mimetype
    mutable quint64 inode = 0;  // 文件的inode，唯一表示符
    mutable QVariantHash extraProperties; // 扩展属性列表
    mutable QPointer<QTimer> getEPTimer; // 获取扩展属性的计时器
    mutable QPointer<QTimer> getIconTimer; // 获取icon的计时器
    mutable MimeDatabase::FileType m_cacheFileType = MimeDatabase::FileType::Unknown; // 缓存文件的FileType
};

LocalFileInfoPrivate::LocalFileInfoPrivate(LocalFileInfo *qq)
    :AbstractFileInfoPrivate(qq)
{

}

LocalFileInfoPrivate::~LocalFileInfoPrivate()
{
    if (getIconTimer) {
        getIconTimer->disconnect(SIGNAL(timeout()));
        getIconTimer->stop();
        getIconTimer->deleteLater();
    }
}

QIcon LocalFileInfoPrivate::standardFileIcon(LocalFileInfo::Icon iconType)
{
    static QIcon linkIcon(QIcon::fromTheme("emblem-symbolic-link"));
    static QIcon lockIcon(QIcon::fromTheme("emblem-locked"));
    static QIcon unreadableIcon(QIcon::fromTheme("emblem-unreadable"));
    static QIcon shareIcon(QIcon::fromTheme("emblem-shared"));

    switch (iconType) {
    case LocalFileInfo::LinkIcon:
        return linkIcon;
    case LocalFileInfo::LockIcon:
        return lockIcon;
    case LocalFileInfo::UnreadableIcon:
        return unreadableIcon;
    case LocalFileInfo::ShareIcon:
        return shareIcon;
    }

    return QIcon();
}

void LocalFileInfoPrivate::readStandardFlagsIcon(){

    Q_Q(LocalFileInfo);
    if (q->isSymLink()) {
        QIcon::fromTheme("emblem-symbolic-link", standardFileIcon(LocalFileInfo::LinkIcon));
    }

    if (!q->isWritable()) {
        QIcon::fromTheme("emblem-readonly", standardFileIcon(LocalFileInfo::LockIcon));
    }

    if (!q->isReadable()) {
        QIcon::fromTheme("emblem-unreadable", standardFileIcon(LocalFileInfo::UnreadableIcon));
    }
}

bool LocalFileInfoPrivate::isLowSpeedFile() const
{
    return lowSpeedFile;
}

QMimeType LocalFileInfoPrivate::readMimeType(QMimeDatabase::MatchMode mode) const
{
    Q_Q(const LocalFileInfo);

    QUrl url = q->url();
    if (url.isLocalFile())
        return MimeDatabase::mimeTypeForUrl(url);
    else
        return MimeDatabase::mimeTypeForFile(UrlRoute::urlToPath(url),
                                              mode);
}
DFMBASE_END_NAMESPACE
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::LocalFileInfoPrivate*)

#endif // LOCALFILEINFO_P_H
