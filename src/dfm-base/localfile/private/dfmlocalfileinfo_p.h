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

#include "base/private/dabstractfileinfo_p.h"
#include "localfile/dfmlocalfileinfo.h"
#include "shutil/dmimedatabase.h"

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>
#include <QMimeType>
#include <QReadWriteLock>

class DFMLocalFileInfo;
class DFMLocalFileInfoPrivate;

class DFMLocalFileInfoPrivate : public DAbstractFileInfoPrivate
{
    Q_DECLARE_PUBLIC(DFMLocalFileInfo)

public: //method
    DFMLocalFileInfoPrivate(DFMLocalFileInfo *qq);

    virtual ~DFMLocalFileInfoPrivate();

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

    static QIcon standardFileIcon(DFMLocalFileInfo::Icon type);

    void readStandardFlagsIcon();

    bool isLowSpeedFile() const;

    virtual QMimeType readMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const;

private: //member
    mutable QIcon icon;
    mutable DFMLocalFileInfo::DFMEmblemInfos EmblemsInfo;
    mutable bool iconFromTheme = false;
    mutable QPointer<QTimer> getIconTimer;

    mutable bool requestingThumbnail = false;
    mutable bool needThumbnail = false;

    mutable QMimeType mimeType;

    mutable qint8 hasThumbnail = -1;
    mutable qint8 lowSpeedFile = -1;
    mutable quint64 inode = 0;

    mutable QVariantHash extraProperties;
    mutable bool epInitialized = false;
    mutable QPointer<QTimer> getEPTimer;
};

DFMLocalFileInfoPrivate::DFMLocalFileInfoPrivate(DFMLocalFileInfo *qq)
    :DAbstractFileInfoPrivate(qq)
{

}

DFMLocalFileInfoPrivate::~DFMLocalFileInfoPrivate()
{
    if (getIconTimer) {
        getIconTimer->disconnect(SIGNAL(timeout()));
        getIconTimer->stop();
        getIconTimer->deleteLater();
    }
}



QIcon DFMLocalFileInfoPrivate::standardFileIcon(DFMLocalFileInfo::Icon iconType)
{
    static QIcon linkIcon(QIcon::fromTheme("emblem-symbolic-link"));
    static QIcon lockIcon(QIcon::fromTheme("emblem-locked"));
    static QIcon unreadableIcon(QIcon::fromTheme("emblem-unreadable"));
    static QIcon shareIcon(QIcon::fromTheme("emblem-shared"));

    switch (iconType) {
    case DFMLocalFileInfo::LinkIcon:
        return linkIcon;
    case DFMLocalFileInfo::LockIcon:
        return lockIcon;
    case DFMLocalFileInfo::UnreadableIcon:
        return unreadableIcon;
    case DFMLocalFileInfo::ShareIcon:
        return shareIcon;
    }

    return QIcon();
}

void DFMLocalFileInfoPrivate::readStandardFlagsIcon(){

    Q_Q(DFMLocalFileInfo);
    if (q->isSymLink()) {
        QIcon::fromTheme("emblem-symbolic-link", standardFileIcon(DFMLocalFileInfo::LinkIcon));
    }

    if (!q->isWritable()) {
        QIcon::fromTheme("emblem-readonly", standardFileIcon(DFMLocalFileInfo::LockIcon));
    }

    if (!q->isReadable()) {
        QIcon::fromTheme("emblem-unreadable", standardFileIcon(DFMLocalFileInfo::UnreadableIcon));
    }
}

bool DFMLocalFileInfoPrivate::isLowSpeedFile() const
{
    return lowSpeedFile;
}

QMimeType DFMLocalFileInfoPrivate::readMimeType(QMimeDatabase::MatchMode mode) const
{
    Q_Q(const DFMLocalFileInfo);

    QUrl url = q->url();
    if (url.isLocalFile())
        return DMimeDatabase::mimeTypeForUrl(url);
    else
        return DMimeDatabase::mimeTypeForFile(DFMUrlRoute::urlToPath(url),
                                              mode);
}

Q_DECLARE_METATYPE(DFMLocalFileInfoPrivate*)

#endif // DFILEINFO_P_H
