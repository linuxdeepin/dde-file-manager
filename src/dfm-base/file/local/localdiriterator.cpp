// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/file/local/private/localdiriterator_p.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <dfm-io/denumerator.h>
#include <dfm-io/dfmio_utils.h>

#include <QDir>
#include <QFile>
#include <QRegularExpression>

#include <functional>
#include <cerrno>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

USING_IO_NAMESPACE
using namespace dfmbase;
using namespace GlobalDConfDefines::ConfigPath;

namespace DConfigKeys {
static constexpr char kAllAsync[] { "dfm.iterator.allasync" };
}

namespace {

QSet<QString> loadHideFileList(const QString &dirPath)
{
    QFile hiddenFile(QDir(dirPath).filePath(".hidden"));
    if (!hiddenFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    const QString data = QString::fromUtf8(hiddenFile.readAll());
    const QStringList entries = data.split('\n', Qt::SkipEmptyParts);
    return QSet<QString>(entries.begin(), entries.end());
}

QString resolveSymlinkTargetPath(const QString &entryPath, const QString &parentPath)
{
    QByteArray buffer;
    buffer.resize(PATH_MAX);
    const QByteArray nativePath = QFile::encodeName(entryPath);
    const ssize_t size = ::readlink(nativePath.constData(), buffer.data(), buffer.size() - 1);
    if (size <= 0)
        return QString();

    buffer[static_cast<int>(size)] = '\0';
    QString targetPath = QFile::decodeName(buffer.constData());
    if (QDir::isRelativePath(targetPath))
        targetPath = QDir(parentPath).absoluteFilePath(targetPath);

    return QDir::cleanPath(targetPath);
}

SortInfoPointer createSortInfo(const QString &parentPath, const QString &fileName, const QSet<QString> &hideList)
{
    const QString entryPath = QDir(parentPath).filePath(fileName);
    const QByteArray nativePath = QFile::encodeName(entryPath);

    struct stat entryStat;
    if (::lstat(nativePath.constData(), &entryStat) != 0)
        return nullptr;

    struct stat effectiveStat = entryStat;
    const bool isSymLink = S_ISLNK(entryStat.st_mode);
    if (isSymLink) {
        const QString targetPath = resolveSymlinkTargetPath(entryPath, parentPath);
        if (!targetPath.isEmpty() && !ProtocolUtils::isRemoteFile(QUrl::fromLocalFile(targetPath))) {
            const QByteArray targetNativePath = QFile::encodeName(targetPath);
            struct stat targetStat;
            if (::stat(targetNativePath.constData(), &targetStat) == 0)
                effectiveStat = targetStat;
        }
    }

    SortInfoPointer info(new SortFileInfo);
    info->setUrl(QUrl::fromLocalFile(entryPath));
    info->setSize(effectiveStat.st_size);
    info->setSymlink(isSymLink);
    info->setDir(S_ISDIR(effectiveStat.st_mode));
    info->setFile(!S_ISDIR(effectiveStat.st_mode));
    info->setHide(fileName.startsWith(".") || hideList.contains(fileName));
    info->setReadable((effectiveStat.st_mode & S_IRUSR) != 0);
    info->setWriteable((effectiveStat.st_mode & S_IWUSR) != 0);
    info->setExecutable((effectiveStat.st_mode & S_IXUSR) != 0);
    info->setLastReadTime(effectiveStat.st_atim.tv_sec);
    info->setLastModifiedTime(effectiveStat.st_mtim.tv_sec);
    // st_ctim is inode change time on Linux, kept for compatibility with existing behavior.
    info->setCreateTime(effectiveStat.st_ctim.tv_sec);
    info->setInfoCompleted(true);
    return info;
}

}   // namespace

LocalDirIteratorPrivate::LocalDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                                                 QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                 LocalDirIterator *q)
    : q(q),
      rootPath(UrlRoute::urlToPath(url))
{
    const QUrl &urlReally = QUrl::fromLocalFile(UrlRoute::urlToPath(url));
    dfmioDirIterator.reset(new DFMIO::DEnumerator(urlReally, nameFilters,
                                                  static_cast<DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                                  static_cast<DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags))));
}

LocalDirIteratorPrivate::~LocalDirIteratorPrivate()
{
}

FileInfoPointer LocalDirIteratorPrivate::fileInfo()
{
    if (dfmioDirIterator.isNull())
        return nullptr;
    return fileInfo(dfmioDirIterator->fileInfo());
}

FileInfoPointer LocalDirIteratorPrivate::fileInfo(const QSharedPointer<DFileInfo> dfmInfo)
{
    if (dfmInfo.isNull())
        return nullptr;

    auto url = dfmInfo->uri();
    QSharedPointer<FileInfo> info { nullptr };
    const QString &fileName = dfmInfo->attribute(DFileInfo::AttributeID::kStandardName, nullptr).toString();
    bool isHidden = false;
    if (fileName.startsWith(".")) {
        isHidden = true;
    } else {
        isHidden = hideFileList.contains(fileName);
    }

    auto targetPath = dfmInfo->attribute(dfmio::DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
    if (ProtocolUtils::isLocalFile(url) && (targetPath.isEmpty() || ProtocolUtils::isLocalFile(QUrl::fromLocalFile(targetPath)))) {
        info = QSharedPointer<SyncFileInfo>(new SyncFileInfo(url));
    } else {
        info = QSharedPointer<AsyncFileInfo>(new AsyncFileInfo(url, dfmInfo));
        info->setExtendedAttributes(ExtInfoType::kFileIsHid, isHidden);
        info.dynamicCast<AsyncFileInfo>()->cacheAsyncAttributes(q->property("QueryAttributes").toString());
    }

    if (info) {
        if (!q->property("QueryAttributes").toString().isEmpty()
            && q->property("QueryAttributes").toString() != "*") {
            info->setExtendedAttributes(ExtInfoType::kFileNeedUpdate, true);
            info->setExtendedAttributes(ExtInfoType::kFileNeedTransInfo, true);
        }
        info->setExtendedAttributes(ExtInfoType::kFileIsHid, isHidden);
        info->setExtendedAttributes(ExtInfoType::kFileCdRomDevice, isCdRomDevice);
    } else {
        qCWarning(logDFMBase) << "info is nullptr url = " << url;
    }

    return info;
}

QList<FileInfoPointer> LocalDirIteratorPrivate::fileInfos()
{
    if (dfmioDirIterator.isNull())
        return {};
    auto dfmInfos = dfmioDirIterator->fileInfoList();
    QList<FileInfoPointer> infos;
    for (const auto &dfmInfo : dfmInfos) {
        auto info = fileInfo(dfmInfo);
        if (info.isNull())
            continue;
        infos.append(info);
    }

    return infos;
}

/*!
 * @class LocalDirIterator 本地（即和系统盘在同一个磁盘的目录）文件迭代器类
 *
 * @brief 使用dfm-io实现了本文件的迭代器
 */
LocalDirIterator::LocalDirIterator(const QUrl &url,
                                   const QStringList &nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags), d(new LocalDirIteratorPrivate(url, nameFilters, filters, flags, this))
{
}

LocalDirIterator::~LocalDirIterator()
{
}
/*!
 * \brief next 下一个文件，迭代器指向下一个文件
 *
 * \return QUrl 返回第一个文件的QUrl
 **/
QUrl LocalDirIterator::next()
{
    if (d->dfmioDirIterator) {
        d->currentUrl = d->dfmioDirIterator->next();
    }

    return d->currentUrl;
}
/*!
 * \brief hasNext 获取迭代器是否还有下一个文件
 *
 * \return bool 返回是否还有下一个文件
 */
bool LocalDirIterator::hasNext() const
{
    if (!d->initQuerry && d->dfmioDirIterator) {
        d->initQuerry = true;
        auto querry = property("QueryAttributes").toString();
        if (!querry.isEmpty())
            d->dfmioDirIterator->setQueryAttributes(querry);
    }
    if (d->dfmioDirIterator)
        return d->dfmioDirIterator->hasNext();

    return false;
}

void LocalDirIterator::close()
{
    d->canceled.storeRelease(true);
    if (d->dfmioDirIterator)
        d->dfmioDirIterator->cancel();
}
/*!
 * \brief fileName 获取文件迭代器当前文件的文件名称
 *
 * \return QString 返回文件迭代器当前文件的文件名称
 **/
QString LocalDirIterator::fileName() const
{
    QString path = fileUrl().path();
    if (path.isEmpty())
        return QString();

    path = path.replace(QRegularExpression("/*/"), "/");
    if (path == "/")
        return QString();

    if (path.endsWith("/"))
        path = path.left(path.size() - 1);
    QStringList pathList = path.split("/");
    return pathList.last();
}
/*!
 * \brief fileUrl 获取文件迭代器当前文件全路径url
 *
 * \return QString 返回文件迭代器当前文件全路径url
 */
QUrl LocalDirIterator::fileUrl() const
{
    return UrlRoute::pathToReal(d->currentUrl.path());
}
/*!
 * \brief fileUrl 获取文件迭代器当前文件的文件信息
 *
 * 必须使用事件去获取文件信息是否有缓存
 *
 * \return DAbstractFileInfoPointer 返回文件迭代器当前文件的文件信息的智能指针
 **/
const FileInfoPointer LocalDirIterator::fileInfo() const
{
    return d->fileInfo();
}
/*!
 * \brief url 获取文件迭代器的基本文件路径的url
 *
 * \return QUrl 返回文件迭代器的基本文件路径的url Returns the base url of the iterator.
 */
QUrl LocalDirIterator::url() const
{
    if (d->dfmioDirIterator)
        return UrlRoute::pathToReal(d->dfmioDirIterator->uri().path());

    return QUrl();
}

void LocalDirIterator::cacheBlockIOAttribute()
{
    const QUrl &rootUrl = this->url();
    const QUrl &url = DFMIO::DFMUtils::buildFilePath(rootUrl.toString().toStdString().c_str(), ".hidden", nullptr);
    d->hideFileList = DFMIO::DFMUtils::hideListFromUrl(url);
    d->isLocalDevice = ProtocolUtils::isLocalFile(rootUrl);
    d->isCdRomDevice = FileUtils::isCdRomDevice(rootUrl);
}

void LocalDirIterator::setArguments(const QVariantMap &args)
{
    if (!d->dfmioDirIterator)
        return;

    if (args.value("sortRole").isValid())
        d->dfmioDirIterator->setSortRole(static_cast<DFMIO::DEnumerator::SortRoleCompareFlag>(args.value("sortRole").toInt()));
    if (args.value("mixFileAndDir").isValid())
        d->dfmioDirIterator->setSortMixed(args.value("mixFileAndDir").toBool());
    if (args.value("sortOrder").isValid())
        d->dfmioDirIterator->setSortOrder(static_cast<Qt::SortOrder>(args.value("sortOrder").toInt()));
}

QList<SortInfoPointer> LocalDirIterator::sortFileInfoList()
{
    if (d->rootPath.isEmpty())
        return {};

    d->canceled.storeRelease(false);
    const QSet<QString> hideList = loadHideFileList(d->rootPath);

    DIR *dir = ::opendir(QFile::encodeName(d->rootPath).constData());
    if (!dir)
        return {};

    QList<SortInfoPointer> sortList;
    while (!d->canceled.loadAcquire()) {
        errno = 0;
        dirent *entry = ::readdir(dir);
        if (!entry)
            break;

        const QString fileName = QFile::decodeName(entry->d_name);
        if (fileName == "." || fileName == "..")
            continue;

        auto info = createSortInfo(d->rootPath, fileName, hideList);
        if (info.isNull())
            continue;
        sortList.append(info);
    }

    ::closedir(dir);
    return sortList;
}

bool LocalDirIterator::oneByOne()
{
    // all dir iterator will in async proccess if this func return true directly.
    if (DConfigManager::instance()->value(kDefaultCfgPath, DConfigKeys::kAllAsync, false).toBool())
        return true;

    // local dir will iterator in sync proccess and others in aysnc proccess
    if (!url().isValid())
        return true;

    auto info = InfoFactory::create<FileInfo>(url());
    if (info)
        return !info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool() || !d->dfmioDirIterator;

    return !ProtocolUtils::isLocalFile(url()) || !d->dfmioDirIterator;
}

bool LocalDirIterator::initIterator()
{
    if (d->dfmioDirIterator)
        return d->dfmioDirIterator->initEnumerator(oneByOne());
    return false;
}

DEnumeratorFuture *LocalDirIterator::asyncIterator()
{
    if (!d->initQuerry && d->dfmioDirIterator) {
        d->initQuerry = true;
        auto querry = property("QueryAttributes").toString();
        if (!querry.isEmpty())
            d->dfmioDirIterator->setQueryAttributes(querry);
    }
    if (d->dfmioDirIterator)
        return d->dfmioDirIterator->asyncIterator();
    return nullptr;
}

QList<FileInfoPointer> LocalDirIterator::fileInfos() const
{
    if (d->dfmioDirIterator)
        return {};
    return d->fileInfos();
}
