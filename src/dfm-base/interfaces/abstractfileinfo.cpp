/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "private/abstractfileinfo_p.h"
#include "abstractfileinfo.h"
#include "utils/chinese2pinyin.h"
#include "dfm-base/mimetype/mimetypedisplaymanager.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"

#include <QMetaType>
#include <QDateTime>
#include <QVariant>
#include <QDir>
#include <QCollator>

USING_IO_NAMESPACE

namespace FileSortFunction {
//fix 多线程排序时，该处的全局变量在compareByString函数中可能导致软件崩溃
//QCollator sortCollator;
class DCollator : public QCollator
{
public:
    DCollator()
        : QCollator()
    {
        setNumericMode(true);
        setCaseSensitivity(Qt::CaseInsensitive);
    }
};

bool startWithSymbol(const QString &text)
{
    if (text.isEmpty())
        return false;

    //匹配字母、数字和中文开头的字符串
    QRegExp regExp("^[a-zA-Z0-9\u4e00-\u9fa5].*$");
    return !regExp.exactMatch(text);
}

bool startWithCNChar(const QString &text)
{
    if (text.isEmpty())
        return false;

    return text.at(0).script() == QChar::Script_Han;
}

bool compareByString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    thread_local static DCollator sortCollator;
    //其他符号要排在最后，需要在中文前先做判断
    if (startWithSymbol(str1)) {
        if (!startWithSymbol(str2))
            return order == Qt::DescendingOrder;
    } else if (startWithSymbol(str2))
        return order != Qt::DescendingOrder;

    if (startWithCNChar(str1)) {
        if (!startWithCNChar(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithCNChar(str2)) {
        return order != Qt::DescendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

COMPARE_FUN_DEFINE(fileName, FileName, DFMBASE_NAMESPACE::AbstractFileInfo)
COMPARE_FUN_DEFINE(size, Size, DFMBASE_NAMESPACE::AbstractFileInfo)
COMPARE_FUN_DEFINE(lastModified, Modified, DFMBASE_NAMESPACE::AbstractFileInfo)
COMPARE_FUN_DEFINE(created, Created, DFMBASE_NAMESPACE::AbstractFileInfo)
COMPARE_FUN_DEFINE(lastRead, LastRead, DFMBASE_NAMESPACE::AbstractFileInfo)
COMPARE_FUN_DEFINE(fileTypeDisplayName, MimeType, DFMBASE_NAMESPACE::AbstractFileInfo)
}   /// end namespace FileSortFunction

#define CALL_PROXY(Fun) \
    if (dptr->proxy) return dptr->proxy->Fun;
DFMBASE_BEGIN_NAMESPACE
Q_GLOBAL_STATIC_WITH_ARGS(int, type_id, { qRegisterMetaType<AbstractFileInfoPointer>("AbstractFileInfo") });

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
AbstractFileInfo::AbstractFileInfo(const QUrl &url, DFMBASE_NAMESPACE::AbstractFileInfoPrivate *d)
    : dptr(d)
{
    Q_UNUSED(type_id)
    dptr->url = url;
}

void DFMBASE_NAMESPACE::AbstractFileInfo::setProxy(const AbstractFileInfoPointer &proxy)
{
    dptr->proxy = proxy;
}

AbstractFileInfo::~AbstractFileInfo()
{
}
/*!
 * \brief = 重载操作符=
 *
 * \param const DAbstractFileInfo & DAbstractFileInfo实例对象的引用
 *
 * \return DAbstractFileInfo & 新DAbstractFileInfo实例对象的引用
 */
AbstractFileInfo &AbstractFileInfo::operator=(const AbstractFileInfo &fileinfo)
{
    dptr->url = fileinfo.dptr->url;
    return *this;
}
/*!
 * \brief == 重载操作符==
 *
 * \param const DAbstractFileInfo & DAbstractFileInfo实例对象的引用
 *
 * \return bool 传入的DAbstractFileInfo实例对象和自己是否相等
 */
bool AbstractFileInfo::operator==(const AbstractFileInfo &fileinfo) const
{
    return dptr->url == fileinfo.dptr->url;
}
/*!
 * \brief != 重载操作符!=
 *
 * \param const DAbstractFileInfo & DAbstractFileInfo实例对象的引用
 *
 * \return bool 传入的DAbstractFileInfo实例对象和自己是否不相等
 */
bool AbstractFileInfo::operator!=(const AbstractFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

/*!
 * \brief setFile 设置文件的File，跟新当前的fileinfo
 *
 * \param const QUrl &url 新文件的URL
 *
 * \return
 */
void AbstractFileInfo::setFile(const QUrl &url)
{
    dptr->url = url;
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
    CALL_PROXY(exists());

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
    CALL_PROXY(refresh());
}

void dfmbase::AbstractFileInfo::refresh(DFileInfo::AttributeID id, const QVariant &value)
{
    CALL_PROXY(refresh(id, value));
}
/*!
 * \brief filePath 获取文件的绝对路径，含文件的名称，相当于文件的全路径
 *
 * url = file:///tmp/archive.tar.gz
 *
 * filePath = /tmp/archive.tar.gz
 *
 * \param
 *
 * \return
 */
QString AbstractFileInfo::filePath() const
{
    CALL_PROXY(filePath());
    return QString();
}

/*!
 * \brief absoluteFilePath 获取文件的绝对路径，含文件的名称，相当于文件的全路径，事例如下：
 *
 * url = file:///tmp/archive.tar.gz
 *
 * absoluteFilePath = /tmp/archive.tar.gz
 *
 * \param
 *
 * \return
 */
QString AbstractFileInfo::absoluteFilePath() const
{
    CALL_PROXY(absoluteFilePath());
    return filePath();
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
QString AbstractFileInfo::fileName() const
{
    CALL_PROXY(fileName());

    QString filePath = this->filePath();

    if (filePath.endsWith(QDir::separator())) {
        filePath.chop(1);
    }

    int index = filePath.lastIndexOf(QDir::separator());

    if (index >= 0) {
        return filePath.mid(index + 1);
    }

    return filePath;
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
QString AbstractFileInfo::baseName() const
{
    CALL_PROXY(baseName());

    const QString &fileName = this->fileName();
    const QString &suffix = this->suffix();

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.length() - suffix.length() - 1);
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
QString AbstractFileInfo::completeBaseName() const
{
    CALL_PROXY(completeBaseName());

    return QString();
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::fileNameOfRename() const
{
    CALL_PROXY(fileNameOfRename());

    return fileName();
}

QString AbstractFileInfo::baseNameOfRename() const
{
    CALL_PROXY(baseNameOfRename());

    return baseName();
}

QString AbstractFileInfo::suffixOfRename() const
{
    CALL_PROXY(suffixOfRename());

    return suffix();
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
QString AbstractFileInfo::suffix() const
{
    CALL_PROXY(suffix());

    return QString();
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
QString AbstractFileInfo::completeSuffix()
{
    CALL_PROXY(completeSuffix());

    return QString();
}
/*!
 * \brief path 获取文件路径，不包含文件的名称，相当于是父目录
 *
 * url = file:///tmp/archive.tar.gz
 *
 * path = /tmp
 *
 * \param
 *
 * \return
 */
QString AbstractFileInfo::path() const
{
    CALL_PROXY(path());

    return QString();
}
/*!
 * \brief path 获取文件路径，不包含文件的名称，相当于是父目录
 *
 * url = file:///tmp/archive.tar.gz
 *
 * absolutePath = /tmp
 *
 * \param
 *
 * \return
 */
QString AbstractFileInfo::absolutePath() const
{
    CALL_PROXY(absolutePath());

    return path();
}
/*!
 * \brief canonicalPath 获取文件canonical路径，包含文件的名称，相当于文件的全路径
 *
 * url = file:///tmp/archive.tar.gz
 *
 * canonicalPath = /tmp/archive.tar.gz
 *
 * \param
 *
 * \return QString 返回没有符号链接或冗余“.”或“..”元素的绝对路径
 */
QString AbstractFileInfo::canonicalPath() const
{

    CALL_PROXY(canonicalPath());

    return filePath();
}
/*!
 * \brief dir 获取文件的父母目录的QDir
 *
 * Returns the path of the object's parent directory as a QDir object.
 *
 * url = file:///tmp/archive.tar.gz
 *
 * dirpath = /tmp
 *
 * \param
 *
 * \return QDir 父母目录的QDir实例对象
 */
QDir AbstractFileInfo::dir() const
{
    CALL_PROXY(dir());

    return QDir(path());
}
/*!
 * \brief absoluteDir 获取文件的父母目录的QDir
 *
 * Returns the file's absolute path as a QDir object.
 *
 * url = file:///tmp/archive.tar.gz
 *
 * absolute path = /tmp
 *
 * \param
 *
 * \return QDir 父母目录的QDir实例对象
 */
QDir AbstractFileInfo::absoluteDir() const
{
    CALL_PROXY(absoluteDir());

    return dir();
}
/*!
 * \brief url 获取文件的url
 *
 * \param
 *
 * \return QUrl 返回设置的url
 */
QUrl AbstractFileInfo::url() const
{
    return dptr->url;
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canRename() const
{

    CALL_PROXY(canRename());

    return false;
}

/*!
 * \brief isReadable 获取文件是否可读
 *
 * Returns the file can Read
 *
 * url = file:///tmp/archive.tar.gz
 *
 * \param
 *
 * \return bool 返回文件是否可读
 */
bool AbstractFileInfo::isReadable() const
{
    CALL_PROXY(isReadable());

    return false;
}
/*!
 * \brief isWritable 获取文件是否可写
 *
 * Returns the file can write
 *
 * url = file:///tmp/archive.tar.gz
 *
 * \param
 *
 * \return bool 返回文件是否可写
 */
bool AbstractFileInfo::isWritable() const
{
    CALL_PROXY(isWritable());

    return false;
}
/*!
 * \brief isExecutable 获取文件是否可执行
 *
 * \param
 *
 * \return bool 返回文件是否可执行
 */
bool AbstractFileInfo::isExecutable() const
{
    CALL_PROXY(isExecutable());

    return false;
}
/*!
 * \brief isHidden 获取文件是否是隐藏
 *
 * \param
 *
 * \return bool 返回文件是否隐藏
 */
bool AbstractFileInfo::isHidden() const
{
    CALL_PROXY(isHidden());

    return false;
}
/*!
 * \brief isFile 获取文件是否是文件
 *
 * Returns true if this object points to a file or to a symbolic link to a file.
 *
 * Returns false if the object points to something which isn't a file,
 *
 * such as a directory.
 *
 * \param
 *
 * \return bool 返回文件是否文件
 */
bool AbstractFileInfo::isFile() const
{
    CALL_PROXY(isFile());

    return false;
}
/*!
 * \brief isDir 获取文件是否是目录
 *
 * Returns true if this object points to a directory or to a symbolic link to a directory;
 *
 * otherwise returns false.
 *
 * \param
 *
 * \return bool 返回文件是否目录
 */
bool AbstractFileInfo::isDir() const
{
    CALL_PROXY(isDir());

    return false;
}
/*!
 * \brief isSymLink 获取文件是否是链接文件
 *
 * Returns true if this object points to a symbolic link;
 *
 * otherwise returns false.Symbolic links exist on Unix (including macOS and iOS)
 *
 * and Windows and are typically created by the ln -s or mklink commands, respectively.
 *
 * Opening a symbolic link effectively opens the link's target.
 *
 * In addition, true will be returned for shortcuts (*.lnk files) on Windows.
 *
 * Opening those will open the .lnk file itself.
 *
 * \param
 *
 * \return bool 返回文件是否是链接文件
 */
bool AbstractFileInfo::isSymLink() const
{
    CALL_PROXY(isSymLink());

    return false;
}
/*!
 * \brief isRoot 获取文件是否是根目录
 *
 * Returns true if the object points to a directory or to a symbolic link to a directory,
 *
 * and that directory is the root directory; otherwise returns false.
 *
 * \param
 *
 * \return bool 返回文件是否是根目录
 */
bool AbstractFileInfo::isRoot() const
{
    CALL_PROXY(isRoot());

    return filePath() == "/";
}
/*!
 * \brief isBundle 获取文件是否是二进制文件
 *
 * Returns true if this object points to a bundle or to a symbolic
 *
 * link to a bundle on macOS and iOS; otherwise returns false.
 *
 * \param
 *
 * \return bool 返回文件是否是二进制文件
 */
bool AbstractFileInfo::isBundle() const
{
    CALL_PROXY(isBundle());

    return false;
}

bool AbstractFileInfo::isShared() const
{
    CALL_PROXY(isShared());

    return false;
}

/*!
 * \brief inode linux系统下的唯一表示符
 *
 * \return quint64 文件的inode
 */
quint64 DFMBASE_NAMESPACE::AbstractFileInfo::inode() const
{
    CALL_PROXY(inode());

    return 0;
}
/*!
 * \brief isBundle 获取文件的链接目标文件
 *
 * Returns the absolute path to the file or directory a symbolic link points to,
 *
 * or an empty string if the object isn't a symbolic link.
 *
 * This name may not represent an existing file; it is only a string.
 *
 * QFileInfo::exists() returns true if the symlink points to an existing file.
 *
 * \param
 *
 * \return QString 链接目标文件的路径
 */
QString AbstractFileInfo::symLinkTarget() const
{
    CALL_PROXY(symLinkTarget());

    return QString();
}
/*!
 * \brief owner 获取文件的拥有者
 *
 * Returns the owner of the file. On systems where files do not have owners,
 *
 * or if an error occurs, an empty string is returned.
 *
 * This function can be time consuming under Unix (in the order of milliseconds).
 *
 * \param
 *
 * \return QString 文件的拥有者
 */
QString AbstractFileInfo::owner() const
{
    CALL_PROXY(owner());

    return QString();
}
/*!
 * \brief ownerId 获取文件的拥有者ID
 *
 * Returns the id of the owner of the file.
 *
 * \param
 *
 * \return uint 文件的拥有者ID
 */
uint AbstractFileInfo::ownerId() const
{
    CALL_PROXY(ownerId());

    return static_cast<uint>(-1);
}
/*!
 * \brief group 获取文件所属组
 *
 * Returns the group of the file.
 *
 * This function can be time consuming under Unix (in the order of milliseconds).
 *
 * \param
 *
 * \return QString 文件所属组
 */
QString AbstractFileInfo::group() const
{
    CALL_PROXY(group());

    return QString();
}
/*!
 * \brief groupId 获取文件所属组的ID
 *
 * Returns the id of the group the file belongs to.
 *
 * \param
 *
 * \return uint 文件所属组ID
 */
uint AbstractFileInfo::groupId() const
{
    CALL_PROXY(groupId());

    return static_cast<uint>(-1);
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
    CALL_PROXY(permission(permissions));

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
    CALL_PROXY(permissions());

    return QFileDevice::Permissions();
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
    CALL_PROXY(size());

    return 0;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::sizeFormat 使用kb，mb，gb显示文件大小
 * \return
 */
QString DFMBASE_NAMESPACE::AbstractFileInfo::sizeFormat() const
{
    CALL_PROXY(sizeFormat());

    return QString();
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::fileDisplayName() const
{
    CALL_PROXY(fileDisplayName());

    return QString();
}

/*!
 * \brief fileDisplayPinyinName 文件的拼音名称，一般为文件的显示名称转为拼音
 *
 * \return QString 文件的拼音名称
 */
QString DFMBASE_NAMESPACE::AbstractFileInfo::fileDisplayPinyinName() const
{
    const QString &displayName = fileDisplayName();

    return Pinyin::Chinese2Pinyin(displayName);
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::sizeDisplayName() const
{
    CALL_PROXY(sizeDisplayName());

    if (isDir()) {
        int size = countChildFile();

        if (size <= 1) {
            return QObject::tr("%1 item").arg(size);
        } else {
            return QObject::tr("%1 items").arg(size);
        }
    } else {
        return FileUtils::formatSize(size());
    }
}

/*!
 * \brief created 获取文件的创建时间
 *
 * Returns the date and time when the file was created,
 *
 * the time its metadata was last changed or the time of last modification,
 *
 * whichever one of the three is available (in that order).
 *
 * \param
 *
 * \return QDateTime 文件的创建时间的QDateTime实例
 */
QDateTime AbstractFileInfo::created() const
{
    CALL_PROXY(created());

    return QDateTime();
}
/*!
 * \brief birthTime 获取文件的创建时间
 *
 * Returns the date and time when the file was created / born.
 *
 * If the file birth time is not available, this function
 *
 * returns an invalid QDateTime.
 *
 * \param
 *
 * \return QDateTime 文件的创建时间的QDateTime实例
 */
QDateTime AbstractFileInfo::birthTime() const
{
    CALL_PROXY(birthTime());

    return created();
}
/*!
 * \brief metadataChangeTime 获取文件的改变时间
 *
 * Returns the date and time when the file metadata was changed.
 *
 * A metadata change occurs when the file is created,
 *
 * but it also occurs whenever the user writes or sets
 *
 * inode information (for example, changing the file permissions).
 *
 * \param
 *
 * \return QDateTime 文件的改变时间的QDateTime实例
 */
QDateTime AbstractFileInfo::metadataChangeTime() const
{
    CALL_PROXY(metadataChangeTime());

    return QDateTime();
}
/*!
 * \brief lastModified 获取文件的最后修改时间
 *
 * \param
 *
 * \return QDateTime 文件的最后修改时间的QDateTime实例
 */
QDateTime AbstractFileInfo::lastModified() const
{
    CALL_PROXY(lastModified());

    return QDateTime();
}
/*!
 * \brief lastRead 获取文件的最后读取时间
 *
 * \param
 *
 * \return QDateTime 文件的最后读取时间的QDateTime实例
 */
QDateTime AbstractFileInfo::lastRead() const
{
    CALL_PROXY(lastRead());

    return QDateTime();
}
/*!
 * \brief fileTime 获取文件的事件通过传入的参数
 *
 * \param QFile::FileTime time 时间类型
 *
 * \return QDateTime 文件的不同时间类型的时间的QDateTime实例
 */
QDateTime AbstractFileInfo::fileTime(QFileDevice::FileTime time) const
{
    CALL_PROXY(fileTime(time));

    if (time == QFileDevice::FileAccessTime) {
        return lastRead();
    } else if (time == QFileDevice::FileBirthTime) {
        return created();
    } else if (time == QFileDevice::FileMetadataChangeTime) {
        return metadataChangeTime();
    } else if (time == QFileDevice::FileModificationTime) {
        return lastModified();
    } else {
        return QDateTime();
    }
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::countChildFile 获取目录下有多少个文件（只有下一级）
 * \return 返回文件数量
 */
int DFMBASE_NAMESPACE::AbstractFileInfo::countChildFile() const
{
    CALL_PROXY(countChildFile());

    return -1;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::fileType 获取文件的设备类型
 * \return 返回文件的设备类型
 */
AbstractFileInfo::FileType DFMBASE_NAMESPACE::AbstractFileInfo::fileType() const
{
    CALL_PROXY(fileType());

    return kUnknown;
}

QVector<DFMBASE_NAMESPACE::ActionType> DFMBASE_NAMESPACE::AbstractFileInfo::menuActionList(DFMBASE_NAMESPACE::AbstractMenu::MenuType type) const
{
    Q_UNUSED(type)
    return {};
}

QSet<DFMBASE_NAMESPACE::ActionType> DFMBASE_NAMESPACE::AbstractFileInfo::disableMenuActionList() const
{
    return {};
}

/*!
 * \brief AbstractFileInfo::compareFunByKey Get sorting function through key
 * \param sortKey Sorted key value
 * \return Sorting function
 */
DFMBASE_NAMESPACE::AbstractFileInfo::CompareFunction AbstractFileInfo::compareFunByKey(const SortKey &sortKey) const
{
    CALL_PROXY(compareFunByKey(sortKey));

    switch (sortKey) {
    case kSortByFileName:
        return FileSortFunction::compareFileListByFileName;
    case kSortByModified:
        return FileSortFunction::compareFileListByModified;
    case kSortByFileSize:
        return FileSortFunction::compareFileListBySize;
    case kSortByFileCreated:
        return FileSortFunction::compareFileListByCreated;
    case kSortByFileLastRead:
        return FileSortFunction::compareFileListByLastRead;
    case kSortByFileMimeType:
        return FileSortFunction::compareFileListByMimeType;
    }
    return CompareFunction();
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByChildFileName Get the URL based on the name of the sub file
 * \param fileName Sub file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByChildFileName(const QString &fileName) const
{
    CALL_PROXY(getUrlByChildFileName(fileName));

    if (!isDir()) {
        return QUrl();
    }
    QUrl theUrl = url();
    theUrl.setPath(absoluteFilePath() + QDir::separator() + fileName);
    return theUrl;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByNewFileName Get URL based on new file name
 * \param fileName New file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    CALL_PROXY(getUrlByNewFileName(fileName));

    QUrl theUrl = url();

    theUrl.setPath(absolutePath() + QDir::separator() + fileName);

    return theUrl;
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::mimeTypeDisplayName() const
{
    CALL_PROXY(mimeTypeDisplayName());

    return MimeTypeDisplayManager::instance()->displayName(mimeTypeName());
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::fileTypeDisplayName Display name of the file type
 * \return Display name of the file typ
 */
QString DFMBASE_NAMESPACE::AbstractFileInfo::fileTypeDisplayName() const
{
    CALL_PROXY(fileTypeDisplayName());

    return QString::number(MimeTypeDisplayManager::instance()->displayNameToEnum(fileMimeType().name())).append(suffix());
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::canRedirectionFileUrl Can I redirect files
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::canRedirectionFileUrl() const
{
    CALL_PROXY(canRedirectionFileUrl());

    return false;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::redirectedFileUrl redirection file
 * \return
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::redirectedFileUrl() const
{
    CALL_PROXY(redirectedFileUrl());

    return url();
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canMoveOrCopy() const
{
    // TODO(liyigang) : temp impl
    if (!isReadable())
        return false;

    return true;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::canDrop
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::canDrop() const
{
    // todo lanxs
    /*if (isPrivate()) {
        return false;
    }*/

    if (!isSymLink()) {
        const bool isDesktop = mimeTypeName() == Global::kMimeTypeAppXDesktop;
        return isDir() || isDesktop || (canDragCompress() && isDragCompressFileFormat());
    }

    AbstractFileInfoPointer info = nullptr;

    do {
        const QUrl &targetUrl = symLinkTarget();

        if (targetUrl == url()) {
            return false;
        }

        info = InfoFactory::create<AbstractFileInfo>(targetUrl);

        if (!info) {
            return false;
        }
    } while (info->isSymLink());

    return info->canDrop();
}

bool AbstractFileInfo::canTag() const
{
    CALL_PROXY(canTag());

    return false;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::parentUrl
 * \return
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::parentUrl() const
{
    CALL_PROXY(parentUrl());

    return UrlRoute::urlParent(url());
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::supportedDragActions
 * \return
 */
Qt::DropActions DFMBASE_NAMESPACE::AbstractFileInfo::supportedDragActions() const
{
    CALL_PROXY(supportedDragActions());

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::supportedDropActions
 * \return
 */
Qt::DropActions DFMBASE_NAMESPACE::AbstractFileInfo::supportedDropActions() const
{
    CALL_PROXY(supportedDropActions());

    if (isWritable()) {
        return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    }

    if (canDrop()) {
        return Qt::CopyAction | Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::canDragCompress
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::canDragCompress() const
{
    CALL_PROXY(canDragCompress());

    return false;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::isDragCompressFileFormat
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::isDragCompressFileFormat() const
{
    CALL_PROXY(isDragCompressFileFormat());

    return false;
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::emptyDirectoryTip() const
{
    return QObject::tr("Folder is empty");
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::loadingTip() const
{
    return QObject::tr("Loading...");
}

void dfmbase::AbstractFileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids, DFileInfo::AttributeExtendFuncCallback callback) const
{
    CALL_PROXY(mediaInfoAttributes(type, ids, callback));
}

bool dfmbase::AbstractFileInfo::notifyAttributeChanged()
{
    CALL_PROXY(notifyAttributeChanged());

    return false;
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::mimeTypeName() const
{
    CALL_PROXY(mimeTypeName());

    return QString();
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::fileIcon
 * \return
 */
QIcon DFMBASE_NAMESPACE::AbstractFileInfo::fileIcon() const
{
    CALL_PROXY(fileIcon());

    return QIcon();
}

QList<QIcon> AbstractFileInfo::additionalIcon() const
{
    CALL_PROXY(additionalIcon());

    QList<QIcon> icons;

    static QIcon linkIcon(QIcon::fromTheme("emblem-symbolic-link"));
    static QIcon lockIcon(QIcon::fromTheme("emblem-locked"));
    static QIcon unreadableIcon(QIcon::fromTheme("emblem-unreadable"));
    static QIcon shareIcon(QIcon::fromTheme("emblem-shared"));

    if (isSymLink()) {
        icons << QIcon::fromTheme("emblem-symbolic-link", linkIcon);
    }

    if (!isWritable()) {
        icons << QIcon::fromTheme("emblem-readonly", lockIcon);
    }

    if (!isReadable()) {
        icons << QIcon::fromTheme("emblem-unreadable", unreadableIcon);
    }

    if (isShared()) {
        icons << QIcon::fromTheme("emblem-shared", shareIcon);
    }

    // TODO lanxs
    /*
#ifdef SW_LABEL
    QString labelIconPath = getLabelIcon();
    if (!labelIconPath.isEmpty()) {
        icons << QIcon(labelIconPath);
    }
#endif*/

    return icons;
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::iconName() const
{
    CALL_PROXY(iconName());

    return QString();
}

QString DFMBASE_NAMESPACE::AbstractFileInfo::genericIconName() const
{
    CALL_PROXY(genericIconName());

    return QString();
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::fileMimeType
 * \return
 */
QMimeType DFMBASE_NAMESPACE::AbstractFileInfo::fileMimeType() const
{
    CALL_PROXY(fileMimeType());

    return QMimeType();
}

/*!
 * \brief 用于获取特定类型文件的特定属性扩展接口
 * \return 返回特定属性的hash表
 */
QVariantHash DFMBASE_NAMESPACE::AbstractFileInfo::extraProperties() const
{
    CALL_PROXY(extraProperties());

    return QVariantHash();
}

/*!
 * \class DAbstractFileInfoPrivate 抽象文件信息私有类
 *
 * \brief 主要存储文件信息的成员变量和数据
 */
AbstractFileInfoPrivate::AbstractFileInfoPrivate(AbstractFileInfo *qq)
    : q(qq)
{
}

AbstractFileInfoPrivate::~AbstractFileInfoPrivate()
{
}
DFMBASE_END_NAMESPACE
