// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/utils/chinese2pinyin.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-io/dfmio_utils.h>

#include <QMetaType>
#include <QDateTime>
#include <QVariant>
#include <QDir>

USING_IO_NAMESPACE

namespace dfmbase {
Q_GLOBAL_STATIC_WITH_ARGS(int, type_id, { qRegisterMetaType<FileInfoPointer>("FileInfo") })

/*!
 * \class DFileInfo 抽象文件信息类
 *
 * \brief 内部实现Url到真实路径的信息关联，设置的真实本地路径总是指向虚拟路径Url
 *
 * 在这之前你应该明确你的路径是否被DFMUrlRoute注册，如果没有注册那么Url会指向QUrl标准定义：
 *
 * QUrl(file:///root) 标识/root路径
 */

/*!
 * \brief DFileInfo 构造函数
 *
 * \param QUrl & 文件的URL
 */
FileInfo::FileInfo(const QUrl &url)
    : AbstractFileInfo(url), dptr(new FileInfoPrivate(this)) { Q_UNUSED(type_id) }

      FileInfo::~FileInfo()
{
}
/*!
 * \brief = 重载操作符=
 *
 * \param const DFileInfo & DFileInfo实例对象的引用
 *
 * \return DFileInfo & 新DFileInfo实例对象的引用
 */
FileInfo &FileInfo::operator=(const FileInfo &fileinfo)
{
    url = fileinfo.url;
    return *this;
}
/*!
 * \brief == 重载操作符==
 *
 * \param const DFileInfo & DFileInfo实例对象的引用
 *
 * \return bool 传入的DFileInfo实例对象和自己是否相等
 */
bool FileInfo::operator==(const FileInfo &fileinfo) const
{
    return url == fileinfo.url;
}
/*!
 * \brief != 重载操作符!=
 *
 * \param const DFileInfo & DFileInfo实例对象的引用
 *
 * \return bool 传入的DFileInfo实例对象和自己是否不相等
 */
bool FileInfo::operator!=(const FileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

bool DFMBASE_NAMESPACE::FileInfo::initQuerier()
{
    return false;
}

void DFMBASE_NAMESPACE::FileInfo::initQuerierAsync(int ioPriority, DFMBASE_NAMESPACE::FileInfo::initQuerierAsyncCallback func, void *userData)
{
    Q_UNUSED(ioPriority);
    Q_UNUSED(func);
    Q_UNUSED(userData);
}
/*!
 * \brief exists 文件是否存在
 *
 * \param
 *
 * \return 返回文件是否存在
 */
bool FileInfo::exists() const
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
void FileInfo::refresh()
{
}

void dfmbase::FileInfo::cacheAttribute(DFileInfo::AttributeID id, const QVariant &value)
{
    Q_UNUSED(id);
    Q_UNUSED(value);
}
/*!
  * \brief 获取文件名称，默认是获取文件的全名称带suffix，此接口不会实现异步，全部使用Qurl去
  * 处理或者字符串处理，这都比较快
  * \param FileNameInfoType
  */
QString dfmbase::FileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case FileNameInfoType::kFileName:
        [[fallthrough]];
    case FileNameInfoType::kFileNameOfRename:
        return dptr->fileName();
    case FileNameInfoType::kBaseName:
        [[fallthrough]];
    case FileNameInfoType::kCompleteBaseName:
        [[fallthrough]];
    case FileNameInfoType::kBaseNameOfRename:
        return dptr->baseName();
    case FileNameInfoType::kSuffixOfRename:
        [[fallthrough]];
    case FileNameInfoType::kSuffix:
        return dptr->suffix();
    case FileNameInfoType::kIconName:
        return const_cast<FileInfo *>(this)->fileMimeType().iconName();
    case FileNameInfoType::kGenericIconName:
        return const_cast<FileInfo *>(this)->fileMimeType().genericIconName();
    default:
        return QString();
    }
}
/*!
  * \brief 获取文件路径，默认是文件全路径，此接口不会实现异步，全部使用Qurl去
  * 处理或者字符串处理，这都比较快
  * \param FileNameInfoType
  */
QString dfmbase::FileInfo::pathOf(const PathInfoType type) const
{
    Q_UNUSED(type);
    switch (type) {
    case FilePathInfoType::kFilePath:
        [[fallthrough]];
    case FilePathInfoType::kAbsoluteFilePath:
        [[fallthrough]];
    case FilePathInfoType::kCanonicalPath:
        return url.path();
    case FilePathInfoType::kPath:
        [[fallthrough]];
    case FilePathInfoType::kAbsolutePath:
        return UrlRoute::urlParent(url).path();
    default:
        return QString();
    }
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
bool FileInfo::permission(QFileDevice::Permissions permissions) const
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
QFileDevice::Permissions FileInfo::permissions() const
{
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
qint64 FileInfo::size() const
{
    return 0;
}

/*!
  * \brief 获取文件的时间信息
  * \param FileTimeType
  */
QVariant dfmbase::FileInfo::timeOf(const TimeInfoType type) const
{
    switch (type) {
    case TimeInfoType::kCreateTime:
        [[fallthrough]];
    case TimeInfoType::kBirthTime:
        [[fallthrough]];
    case TimeInfoType::kMetadataChangeTime:
        [[fallthrough]];
    case TimeInfoType::kLastModified:
        [[fallthrough]];
    case TimeInfoType::kLastRead:
        [[fallthrough]];
    case TimeInfoType::kDeletionTime:
        return QDateTime();
    case TimeInfoType::kCreateTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kBirthTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kMetadataChangeTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kLastModifiedSecond:
        [[fallthrough]];
    case TimeInfoType::kLastReadSecond:
        [[fallthrough]];
    case TimeInfoType::kDeletionTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kCreateTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kBirthTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kMetadataChangeTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kLastModifiedMSecond:
        [[fallthrough]];
    case TimeInfoType::kLastReadMSecond:
        [[fallthrough]];
    case TimeInfoType::kDeletionTimeMSecond:
        return 0;
    default:
        return QVariant();
    }
}
/*!
 * \brief DFMBASE_NAMESPACE::FileInfo::countChildFile 获取目录下有多少个文件（只有下一级）
 * \return 返回文件数量
 */
int DFMBASE_NAMESPACE::FileInfo::countChildFile() const
{
    return -1;
}

int dfmbase::FileInfo::countChildFileAsync() const
{
    return -1;
}

/*!
 * \brief DFMBASE_NAMESPACE::FileInfo::fileType 获取文件的设备类型
 * \return 返回文件的设备类型
 */
FileInfo::FileType DFMBASE_NAMESPACE::FileInfo::fileType() const
{
    return FileType::kUnknown;
}

/*!
 * \brief 获取文件路径，默认是文件全路径，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileNameInfoType
 */
QString dfmbase::FileInfo::displayOf(const DisPlayInfoType type) const
{
    switch (type) {
    case DisPlayInfoType::kSizeDisplayName:
        if (isAttributes(OptInfoType::kIsDir))
            return "-";   // for dir don't display items count, highly improve the view's performance
        else
            return FileUtils::formatSize(size());
    case DisPlayInfoType::kFileDisplayPath:
        return url.path();
    case DisPlayInfoType::kMimeTypeDisplayName:
        return MimeTypeDisplayManager::instance()->displayName(nameOf(FileNameInfoType::kMimeTypeName));
    case DisPlayInfoType::kFileTypeDisplayName: {

        int mimeDisplayNum = static_cast<int>(MimeTypeDisplayManager::
                                              instance()
                                                      ->displayNameToEnum(const_cast<FileInfo *>(this)->fileMimeType().name()));
        return QString("%1.").arg(mimeDisplayNum, 2, 10, QChar('0'))
                .append(nameOf(FileNameInfoType::kSuffix));
        }
    case DisPlayInfoType::kFileDisplayPinyinName:
        if (pinyinName.isEmpty()) {
            const QString &displayName = this->displayOf(DisplayInfoType::kFileDisplayName);
            const_cast<FileInfo *>(this)->pinyinName = Pinyin::Chinese2Pinyin(displayName);
        }

        return pinyinName;
    default:
        return QString();
    }
}

/*!
 * \brief 获取文件url，默认是文件的url，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileUrlInfoType
 */
QUrl dfmbase::FileInfo::urlOf(const UrlInfoType type) const
{
    // FileUrlInfoType::kUrl don't you proxy,must use the original url
    if (FileUrlInfoType::kUrl == type)
        return url;
    switch (type) {
    case FileUrlInfoType::kOriginalUrl:
        [[fallthrough]];
    case FileUrlInfoType::kRedirectedFileUrl:
        return url;
    case FileUrlInfoType::kParentUrl:
        return UrlRoute::urlParent(url);
    default:
        return QUrl();
    }
}

QUrl dfmbase::FileInfo::getUrlByType(const UrlInfoType type, const QString &fileName) const
{
    switch (type) {
    case FileUrlInfoType::kGetUrlByNewFileName:
        return dptr->getUrlByNewFileName(fileName);
    case FileUrlInfoType::kGetUrlByChildFileName:
        return dptr->getUrlByChildFileName(fileName);
    default:
        return QUrl();
    }
}
/*!
  * \brief view进入当前目录的提示信息，固定字符串处理，不实现异步
  * \param SupportType
  */
Qt::DropActions DFMBASE_NAMESPACE::FileInfo::supportedOfAttributes(const SupportedType type) const
{
    switch (type) {
    case SupportType::kDrag:
        return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    case SupportType::kDrop:
        if (isAttributes(OptInfoType::kIsWritable)) {
            return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
        }

        if (dptr->canDrop()) {
            return Qt::CopyAction | Qt::MoveAction;
        }
        return Qt::IgnoreAction;
    default:
        return Qt::IgnoreAction;
    }
}

/*!
 * \brief 获取文件扩展属性，如果创建时使用的异步，那么这里获取就是使用异步，没获取到就是
 * 默认，获取了就是读取属性
 * \param FileIsType
 */
bool dfmbase::FileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsRoot:
        return pathOf(PathInfoType::kFilePath) == "/";
    default:
        return false;
    }
}

bool dfmbase::FileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanFetch:
        return isAttributes(OptInfoType::kIsDir)
                && !isAttributes(OptInfoType::kIsPrivate);
    case FileCanType::kCanDrop:
        return dptr->canDrop();
    case FileCanType::kCanDrag:
        [[fallthrough]];
    case FileCanType::kCanHidden:
        [[fallthrough]];
    case FileCanType::kCanMoveOrCopy:
        return true;
    default:
        return false;
    }
}

QVariant dfmbase::FileInfo::extendAttributes(const ExtInfoType type) const
{
    switch (type) {
    case FileExtendedInfoType::kInode:
        return 0;
    case FileExtendedInfoType::kOwner:
        [[fallthrough]];
    case FileExtendedInfoType::kSizeFormat:
        [[fallthrough]];
    case FileExtendedInfoType::kGroup:
        return QString();
    case FileExtendedInfoType::kOwnerId:
        [[fallthrough]];
    case FileExtendedInfoType::kGroupId:
        return static_cast<uint>(-1);
    default:
        QReadLocker locker(&extendOtherCacheLock);
        return extendOtherCache.value(type);
    }
}

/*!
 * \brief view进入当前目录的提示信息，固定字符串处理，不实现异步
 * \param ViewType
 */
QString DFMBASE_NAMESPACE::FileInfo::viewOfTip(const ViewInfoType type) const
{
    switch (type) {
    case ViewType::kEmptyDir:
        return QObject::tr("Folder is empty");
    case ViewType::kLoading:
        return QObject::tr("Loading...");
    default:
        return QString();
    }
}

QVariant DFMBASE_NAMESPACE::FileInfo::customAttribute(const char *key, const DFileInfo::DFileAttributeType type)
{
    Q_UNUSED(type);
    Q_UNUSED(key);
    return QVariant();
}

QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> DFMBASE_NAMESPACE::FileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids) const
{
    Q_UNUSED(type);
    Q_UNUSED(ids);
    return QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant>();
}

void dfmbase::FileInfo::updateAttributes(const QList<dfmbase::FileInfo::FileInfoAttributeID> &types)
{
    Q_UNUSED(types);
}

/*!
  * \brief setExtendedAttributes 设置文件的扩展属性
  * \param ExInfo 扩展属性key \param QVariant 属性
  * return
  */
void DFMBASE_NAMESPACE::FileInfo::setExtendedAttributes(const ExtInfoType &key, const QVariant &value)
{
    QWriteLocker locker(&extendOtherCacheLock);
    extendOtherCache.insert(key, value);
}
/*!
 * \brief DFMBASE_NAMESPACE::FileInfo::fileIcon
 * \return
 */
QIcon DFMBASE_NAMESPACE::FileInfo::fileIcon()
{
    return QIcon();
}

/*!
 * \brief DFMBASE_NAMESPACE::FileInfo::fileMimeType
 * \return
 */
QMimeType DFMBASE_NAMESPACE::FileInfo::fileMimeType(QMimeDatabase::MatchMode mode /*= QMimeDatabase::MatchDefault*/)
{
    Q_UNUSED(mode);
    return QMimeType();
}

QMimeType dfmbase::FileInfo::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    Q_UNUSED(mode);
    return QMimeType();
}

/*!
 * \brief 用于获取特定类型文件的特定属性扩展接口
 * \return 返回特定属性的hash表
 */
QVariantHash DFMBASE_NAMESPACE::FileInfo::extraProperties() const
{
    return QVariantHash();
}

QVariant DFMBASE_NAMESPACE::FileInfo::customData(int role) const
{
    Q_UNUSED(role);
    return QVariant();
}

/*!
 * \class DFileInfoPrivate 抽象文件信息私有类
 *
 * \brief 主要存储文件信息的成员变量和数据
 */
FileInfoPrivate::FileInfoPrivate(FileInfo *qq)
    : q(qq)
{
}

FileInfoPrivate::~FileInfoPrivate()
{
}

/*!
 * \brief DFMBASE_NAMESPACE::FileInfo::getUrlByChildFileName Get the URL based on the name of the sub file
 * \param fileName Sub file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::FileInfoPrivate::getUrlByChildFileName(const QString &fileName) const
{
    if (!q->isAttributes(OptInfoType::kIsDir)) {
        return QUrl();
    }
    QUrl theUrl = q->fileUrl();
    theUrl.setPath(DFMIO::DFMUtils::buildFilePath(q->pathOf(PathInfoType::kAbsoluteFilePath).toStdString().c_str(),
                                                  fileName.toStdString().c_str(), nullptr));
    return theUrl;
}

/*!
 * \brief DFMBASE_NAMESPACE::FileInfo::getUrlByNewFileName Get URL based on new file name
 * \param fileName New file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::FileInfoPrivate::getUrlByNewFileName(const QString &fileName) const
{
    QUrl theUrl = q->fileUrl();
    const QString &newPath = DFMIO::DFMUtils::buildFilePath(q->pathOf(PathInfoType::kAbsolutePath).toStdString().c_str(), fileName.toStdString().c_str(), nullptr);
    theUrl.setPath(newPath);

    return theUrl;
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
QString FileInfoPrivate::fileName() const
{
    QString filePath = q->fileUrl().path();

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
QString FileInfoPrivate::baseName() const
{
    const QString &fileName = this->fileName();
    const QString &suffix = q->nameOf(NameInfoType::kSuffix);

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.length() - suffix.length() - 1);
}

QString dfmbase::FileInfoPrivate::suffix() const
{
    if (q->isAttributes(OptInfoType::kIsDir)) {
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

/*!
 * \brief DFMBASE_NAMESPACE::FileInfo::canDrop
 * \return
 */
bool DFMBASE_NAMESPACE::FileInfoPrivate::canDrop()
{
    if (q->isAttributes(OptInfoType::kIsPrivate)) {
        return false;
    }

    if (!q->isAttributes(OptInfoType::kIsSymLink)) {
        const bool isDesktop = q->nameOf(NameInfoType::kMimeTypeName) == Global::Mime::kTypeAppXDesktop;
        return (q->isAttributes(OptInfoType::kIsDir) && q->isAttributes(OptInfoType::kIsWritable)) || isDesktop;
    }

    FileInfoPointer info = nullptr;
    QString linkTargetPath = q->pathOf(PathInfoType::kSymLinkTarget);

    do {
        const QUrl &targetUrl = QUrl::fromLocalFile(linkTargetPath);

        if (targetUrl == q->fileUrl()) {
            return false;
        }

        info = InfoFactory::create<FileInfo>(targetUrl);

        if (!info) {
            return false;
        }

        linkTargetPath = info->pathOf(PathInfoType::kSymLinkTarget);
    } while (info->isAttributes(OptInfoType::kIsSymLink));

    return info->canAttributes(CanableInfoType::kCanDrop);
}

}
