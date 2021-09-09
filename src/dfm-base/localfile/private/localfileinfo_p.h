/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
    mutable QIcon icon; // 文件的icon
    mutable LocalFileInfo::DFMEmblemInfos EmblemsInfo;   // 文件的扩展属性
    mutable QAtomicInteger<bool> iconFromTheme { false }; // icon是否是来自缩略图
    mutable QAtomicInteger<bool> requestingThumbnail { false };   // 请求缩略图
    mutable QAtomicInteger<bool> needThumbnail { false }; // 是否需要缩略图
    mutable QAtomicInteger<qint8> hasThumbnail { -1 }; // 是否拥有缩略图
    mutable QAtomicInteger<qint8> lowSpeedFile { -1 }; // 是否是低速设备
    mutable QAtomicInteger<bool> epInitialized { false }; // 扩展属性是否初始化
    mutable QMimeType mimeType; // 文件的mimetype
    mutable QAtomicInteger<quint64> inode { 0 };  // 文件的inode，唯一表示符
    mutable QVariantHash extraProperties; // 扩展属性列表
    mutable QPointer<QTimer> getEPTimer; // 获取扩展属性的计时器
    mutable QPointer<QTimer> getIconTimer; // 获取icon的计时器
    mutable MimeDatabase::FileType cacheFileType { MimeDatabase::FileType::Unknown }; // 缓存文件的FileType
public:
    explicit LocalFileInfoPrivate(LocalFileInfo *qq);
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
