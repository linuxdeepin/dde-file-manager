/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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

#include "interfaces/private/abstractfileinfo_p.h"
#include "file/local/localfileinfo.h"
#include "mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/systempathutil.h"

#include <QFileInfo>
#include <QIcon>
#include <QTimer>
#include <QFuture>
#include <QQueue>
#include <QMimeType>
#include <QReadWriteLock>
#include <QReadLocker>

namespace dfmbase {
class LocalFileInfoPrivate : public AbstractFileInfoPrivate
{
    friend class LocalFileInfo;
    QSharedPointer<DFileInfo> dfmFileInfo { nullptr };   // dfm文件的信息

    QAtomicInteger<quint64> inode { 0 };   // 文件的inode，唯一表示符
    QVariantHash extraProperties;   // 扩展属性列表
    MimeDatabase::FileType fileType { MimeDatabase::FileType::kUnknown };   // 缓存文件的FileType
    QMap<DFileInfo::AttributeID, QVariant> attributes;   // 缓存的fileinfo信息
    QMap<DFileInfo::AttributeExtendID, QVariant> attributesExtend;   // 缓存的fileinfo 扩展信息
    QList<DFileInfo::AttributeExtendID> extendIDs;

    QMimeType mimeType;
    QMimeDatabase::MatchMode mimeTypeMode;

    QReadWriteLock lock;
    QMutex mutex;

    enum IconType {
        kDefaultIcon,
        kThumbIcon,
    };
    QReadWriteLock iconLock;
    QMap<IconType, QIcon> icons;
    std::atomic_bool loadingThumbnail = { false };
    std::atomic_int enableThumbnail = { -1 };   // 小于0时表示此值未初始化，0表示不支持，1表示支持
    QPointer<QTimer> getIconTimer = nullptr;

    std::atomic_int enableEmblems = { -1 };
    QMap<int, QIcon> gioEmblemsMap;   // 缓存的角标数据

    // permission
    std::atomic_bool permissionsInited { false };
    QFileDevice::Permissions permissions;

    // device
    QVariant isLocalDevice;
    QVariant isCdRomDevice;

public:
    explicit LocalFileInfoPrivate(const QUrl &url, LocalFileInfo *qq);
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
    virtual QMimeType readMimeType(QMimeDatabase::MatchMode mode = QMimeDatabase::MatchDefault) const;

    void clearIcon()
    {
        icons.clear();
        loadingThumbnail = false;
        enableThumbnail = -1;
        getIconTimer = nullptr;
    }

    QIcon thumbIcon();
    QIcon defaultIcon();
    void onRequestThumbFinished(const QString &path);

    void attributesExtendCallback(bool ok, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> values)
    {
        if (ok) {
            auto it = values.constBegin();
            while (it != values.constEnd()) {
                const QVariant &value = it.value();
                if (value.isValid() && !value.toString().isEmpty())
                    attributesExtend.insert(it.key(), value);
                ++it;
            }

            QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> ret;
            for (const DFileInfo::AttributeExtendID &id : extendIDs) {
                if (attributesExtend.count(id) > 0) {
                    ret.insert(id, attributesExtend.value(id));
                }
            }

            q->mediaDataFinished(true, ret);
        } else {
            q->mediaDataFinished(false, {});
        }
    }

private:
    QString fileName() const;
    QString baseName() const;
    QString completeBaseName() const;
    QString suffix() const;
    QString completeSuffix();
    QString iconName();
    QString mimeTypeName();
};

LocalFileInfoPrivate::LocalFileInfoPrivate(const QUrl &url, LocalFileInfo *qq)
    : AbstractFileInfoPrivate(url, qq)
{
}

LocalFileInfoPrivate::~LocalFileInfoPrivate()
{
}

QMimeType LocalFileInfoPrivate::readMimeType(QMimeDatabase::MatchMode mode) const
{
    QUrl url = q->url();
    if (url.isLocalFile())
        return MimeDatabase::mimeTypeForUrl(url);
    else
        return MimeDatabase::mimeTypeForFile(UrlRoute::urlToPath(url),
                                             mode);
}
/*!
 * \brief fileName 文件名称，全名称
 *
 * url = file:///tmp/archive.tar.gz
 *
 * fileName = archive.tar.gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfoPrivate::fileName() const
{
    QString fileName;

    QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
    if (attributes.count(DFileInfo::AttributeID::kStandardName) == 0) {
        locker.unlock();

        QWriteLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        bool success = false;
        if (dfmFileInfo) {
            fileName = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardName, &success).toString();

            // trans "/" to "smb-share:server=xxx,share=xxx"
            if (fileName == R"(/)" && FileUtils::isGvfsFile(url)) {
                fileName = dfmFileInfo->attribute(DFileInfo::AttributeID::kIdFilesystem, &success).toString();
            }
        }
        if (!success)
            fileName = QFileInfo(url.path()).fileName();

        const_cast<LocalFileInfoPrivate *>(this)->attributes.insert(DFileInfo::AttributeID::kStandardName, fileName);
    } else {
        fileName = attributes.value(DFileInfo::AttributeID::kStandardName).toString();
    }

    return fileName;
}
/*!
 * \brief baseName 文件的基本名称
 *
 * url = file:///tmp/archive.tar.gz
 *
 * baseName = archive
 *
 * \param
 *
 * \return
 */
QString LocalFileInfoPrivate::baseName() const
{
    QString baseName;
    QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);

    if (const_cast<LocalFileInfoPrivate *>(this)->attributes.count(DFileInfo::AttributeID::kStandardBaseName) == 0) {
        locker.unlock();

        QWriteLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        bool success = false;
        if (dfmFileInfo) {
            baseName = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardBaseName, &success).toString();
        }
        if (!success)
            baseName = QFileInfo(url.path()).baseName();

        const_cast<LocalFileInfoPrivate *>(this)->attributes.insert(DFileInfo::AttributeID::kStandardBaseName, baseName);
    } else {
        baseName = attributes.value(DFileInfo::AttributeID::kStandardBaseName).toString();
    }

    return baseName;
}
/*!
 * \brief completeBaseName 文件的完整基本名称
 *
 * url = file:///tmp/archive.tar.gz
 *
 * completeBaseName = archive.tar
 *
 * \param
 *
 * \return
 */
QString LocalFileInfoPrivate::completeBaseName() const
{
    QString completeBaseName;

    QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
    if (attributes.count(DFileInfo::AttributeID::kStandardCompleteBaseName) == 0) {
        locker.unlock();

        QWriteLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        bool success = false;
        if (dfmFileInfo) {
            completeBaseName = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardCompleteBaseName, &success).toString();
        }
        if (!success)
            completeBaseName = QFileInfo(url.path()).completeBaseName();

        const_cast<LocalFileInfoPrivate *>(this)->attributes.insert(DFileInfo::AttributeID::kStandardCompleteBaseName, completeBaseName);
    } else {
        completeBaseName = attributes.value(DFileInfo::AttributeID::kStandardCompleteBaseName).toString();
    }

    return completeBaseName;
}
/*!
 * \brief suffix 文件的suffix
 *
 * url = file:///tmp/archive.tar.gz
 *
 * suffix = gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfoPrivate::suffix() const
{
    QString suffix;

    QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
    if (attributes.count(DFileInfo::AttributeID::kStandardSuffix) == 0) {
        locker.unlock();

        QWriteLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        bool success = false;
        if (dfmFileInfo) {
            suffix = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardSuffix, &success).toString();
        }
        if (!success)
            suffix = QFileInfo(url.path()).suffix();

        const_cast<LocalFileInfoPrivate *>(this)->attributes.insert(DFileInfo::AttributeID::kStandardSuffix, suffix);
    } else {
        suffix = attributes.value(DFileInfo::AttributeID::kStandardSuffix).toString();
    }

    return suffix;
}
/*!
 * \brief suffix 文件的完整suffix
 *
 * url = file:///tmp/archive.tar.gz
 *
 * suffix = tar.gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfoPrivate::completeSuffix()
{
    QString completeSuffix;

    QReadLocker locker(&lock);
    if (attributes.count(DFileInfo::AttributeID::kStandardCompleteSuffix) == 0) {
        locker.unlock();

        QWriteLocker locker(&lock);
        bool success = false;
        if (dfmFileInfo) {
            completeSuffix = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardCompleteSuffix, &success).toString();
        }
        if (!success)
            completeSuffix = QFileInfo(url.path()).completeSuffix();

        attributes.insert(DFileInfo::AttributeID::kStandardCompleteSuffix, completeSuffix);
    } else {
        completeSuffix = attributes.value(DFileInfo::AttributeID::kStandardCompleteSuffix).toString();
    }

    return completeSuffix;
}
QString LocalFileInfoPrivate::iconName()
{
    QString iconNameValue;
    QReadLocker locker(&lock);

    if (attributes.count(DFileInfo::AttributeID::kStandardIcon) == 0) {
        locker.unlock();

        if (SystemPathUtil::instance()->isSystemPath(q->absoluteFilePath()))
            iconNameValue = SystemPathUtil::instance()->systemPathIconNameByPath(q->absoluteFilePath());
        if (iconNameValue.isEmpty()) {
            if (dfmFileInfo) {
                const QStringList &list = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardIcon, nullptr).toStringList();
                if (!list.isEmpty())
                    iconNameValue = list.first();
            }
        }

        QWriteLocker locker(&lock);
        attributes.insert(DFileInfo::AttributeID::kStandardIcon, iconNameValue);
    } else {
        iconNameValue = attributes.value(DFileInfo::AttributeID::kStandardIcon).toString();
    }

    return iconNameValue;
}
QString LocalFileInfoPrivate::mimeTypeName()
{
    QString type;

    QReadLocker locker(&lock);
    if (attributes.count(DFileInfo::AttributeID::kStandardContentType) == 0) {
        locker.unlock();

        QWriteLocker locker(&lock);
        bool success = false;
        if (dfmFileInfo) {
            type = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardContentType, &success).toString();

            if (success)
                attributes.insert(DFileInfo::AttributeID::kStandardContentType, type);
        }
    } else {
        type = attributes.value(DFileInfo::AttributeID::kStandardContentType).toString();
    }
    return type;
}

}
Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::LocalFileInfoPrivate *)

#endif   // LOCALFILEINFO_P_H
