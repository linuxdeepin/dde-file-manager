// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractfileinfo.h>
#include <dfm-base/utils/chinese2pinyin.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QMetaType>
#include <QDir>

USING_IO_NAMESPACE

namespace dfmbase {
Q_GLOBAL_STATIC_WITH_ARGS(int, type_id, { qRegisterMetaType<AbstractFileInfoPointer>("AbstractFileInfo") })

/*!
 * \class DAbstractFileInfo 抽象文件信息类
 *
 * \brief 内部实现Url到真实路径的信息关联，设置的真实本地路径总是指向虚拟路径Url
 *
 * 在这之前你应该明确你的路径是否被DFMUrlRoute注册，如果没有注册那么Url会指向QUrl标准定义：
 *
 * QUrl(file:///root) 标识/root路径
 */

/*!
 * \brief DAbstractFileInfo 构造函数
 *
 * \param QUrl & 文件的URL
 */
AbstractFileInfo::AbstractFileInfo(const QUrl &url)
    : url(url)
{
    if (url.path().endsWith(QDir::separator()) && url.path() != QDir::separator()) {
        auto path = url.path();
        path.chop(1);
        this->url.setPath(path);
    }
    Q_UNUSED(type_id)
}

AbstractFileInfo::~AbstractFileInfo()
{
}

QUrl dfmbase::AbstractFileInfo::fileUrl() const
{
    return url;
}

/*!
 * \brief exists 文件是否存在
 *
 * \param
 *
 * \return 返回文件是否存在
 */
bool AbstractFileInfo::exists() const
{
    return false;
}
/*!
 * \brief refresh 跟新文件信息，清理掉缓存的所有的文件信息
 *
 * \param
 *
 * \return
 */
void AbstractFileInfo::refresh()
{
}

QString dfmbase::AbstractFileInfo::filePath() const
{
    return url.path();
}

QString dfmbase::AbstractFileInfo::absoluteFilePath() const
{
    return filePath();
}

QString dfmbase::AbstractFileInfo::fileName() const
{
    QString filePath = this->filePath();

    int index = filePath.lastIndexOf(QDir::separator());

    if (index >= 0) {
        return filePath.mid(index + 1);
    }

    return filePath;
}

QString dfmbase::AbstractFileInfo::baseName() const
{
    const QString &fileName = this->fileName();
    const QString &suffix = this->suffix();

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.length() - suffix.length() - 1);
}

QString dfmbase::AbstractFileInfo::completeBaseName() const
{
    return fileName();
}

QString dfmbase::AbstractFileInfo::suffix() const
{
    if (isDir()) {
        return QString();
    }
    // xushitong 20200424 修改后缀名获取策略为小数点后非空字符串
    const QString &strFileName = this->fileName();
    QString tmpName = strFileName;
    int nIdx = 0;
    QString strSuffix;
    while (strSuffix.isEmpty()) {
        nIdx = tmpName.lastIndexOf(".");
        if (nIdx == -1 || nIdx == 0)
            return QString();
        strSuffix = tmpName.mid(nIdx + 1);
        tmpName = tmpName.mid(0, nIdx);
    }
    return strFileName.mid(nIdx + 1);
}

QString dfmbase::AbstractFileInfo::completeSuffix() const
{
    if (isDir()) {
        return QString();
    }

    const QString &fileName = this->fileName();

    int index = fileName.indexOf('.');

    if (index >= 0) {
        return fileName.mid(index + 1);
    }

    return QString();
}

QString dfmbase::AbstractFileInfo::path() const
{
    const QString &filePath = this->filePath();

    int index = filePath.lastIndexOf(QDir::separator());

    if (index > 0) {
        return filePath.left(index);
    }

    return filePath;
}

QString dfmbase::AbstractFileInfo::absolutePath() const
{
    return path();
}

bool dfmbase::AbstractFileInfo::isReadable() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isWritable() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isExecutable() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isHidden() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isNativePath() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isFile() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isDir() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isSymLink() const
{
    return false;
}

bool dfmbase::AbstractFileInfo::isRoot() const
{
    return filePath() == QDir::separator();
}

bool dfmbase::AbstractFileInfo::isBundle() const
{
    return false;
}

QString dfmbase::AbstractFileInfo::symLinkTarget() const
{
    return QString();
}

QString dfmbase::AbstractFileInfo::owner() const
{
    return QString();
}

uint dfmbase::AbstractFileInfo::ownerId() const
{
    return 0;
}

QString dfmbase::AbstractFileInfo::group() const
{
    return QString();
}

uint dfmbase::AbstractFileInfo::groupId() const
{
    return 0;
}

/*!
 * \brief permission 判断文件是否有传入的权限
 *
 * Tests for file permissions. The permissions argument can be several flags
 *
 * of type QFile::Permissions OR-ed together to check for permission combinations.
 *
 * On systems where files do not have permissions this function always returns true.
 *
 * \param QFile::Permissions permissions 文件的权限
 *
 * \return bool 是否有传入的权限
 */
bool AbstractFileInfo::permission(QFileDevice::Permissions permissions) const
{
    return this->permissions() & permissions;
}
/*!
 * \brief permissions 获取文件的全部权限
 *
 * \param
 *
 * \return QFile::Permissions 文件的全部权限
 */
QFileDevice::Permissions AbstractFileInfo::permissions() const
{
    return QFileDevice::Permissions();
}

int dfmbase::AbstractFileInfo::countChildFile() const
{
    return 0;
}
/*!
 * \brief size 获取文件的大小
 *
 * Returns the file size in bytes.
 *
 * If the file does not exist or cannot be fetched, 0 is returned.
 *
 * \param
 *
 * \return qint64 文件的大小
 */
qint64 AbstractFileInfo::size() const
{
    return 0;
}

quint32 dfmbase::AbstractFileInfo::birthTime() const
{
    return 0;
}

quint32 dfmbase::AbstractFileInfo::metadataChangeTime() const
{
    return 0;
}

quint32 dfmbase::AbstractFileInfo::lastModified() const
{
    return 0;
}

quint32 dfmbase::AbstractFileInfo::lastRead() const
{
    return 0;
}

}
