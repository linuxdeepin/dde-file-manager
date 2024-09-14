// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <KCodecs>
#    include <KEncodingProber>
#endif

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/dfile.h>
#include <dfm-io/denumerator.h>

#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QTextCodec>
#endif
#include <QSet>
#include <QRegularExpression>
#include <QCollator>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QScreen>
#include <QStandardPaths>

#include <DDBusSender>

#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>

#ifdef COMPILE_ON_V23
#    define APPEARANCE_SERVICE "org.deepin.dde.Appearance1"
#    define APPEARANCE_PATH "/org/deepin/dde/Appearance1"
#else
#    define APPEARANCE_SERVICE "com.deepin.daemon.Appearance"
#    define APPEARANCE_PATH "/com/deepin/daemon/Appearance"
#endif

namespace dfmbase {

static constexpr char kDDETrashId[] { "dde-trash" };
static constexpr char kDDEComputerId[] { "dde-computer" };
static constexpr char kDDEHomeId[] { "dde-home" };
static constexpr char kFileAllTrash[] { "dfm.trash.allfiletotrash" };
const static int kDefaultMemoryPageSize = 4096;

QMutex FileUtils::cacheCopyingMutex;
QSet<QUrl> FileUtils::copyingUrl;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country);
#endif

QString sizeString(const QString &str)
{
    int beginPos = str.indexOf('.');

    if (beginPos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > beginPos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

QString FileUtils::formatSize(qint64 num, bool withUnitVisible, int precision, int forceUnit, QStringList unitList)
{
    if (num < 0) {
        qCWarning(logDFMBase) << "Negative number passed to formatSize():" << num;
        num = 0;
    }

    bool isForceUnit = (forceUnit >= 0);
    QStringList list;
    qreal fileSize(num);

    if (unitList.size() == 0) {
        list << " B"
             << " KB"
             << " MB"
             << " GB"
             << " TB";   // should we use KiB since we use 1024 here?
    } else {
        list = unitList;
    }

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024 && !isForceUnit) {
            break;
        }

        if (isForceUnit && index == forceUnit) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    QString unitString = withUnitVisible ? unit : QString();
    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', precision)), unitString);
}

int FileUtils::supportedMaxLength(const QString &fileSystem)
{
    const static QMap<QString, int> datas {
        { "vfat", 11 },   // man 8 mkfs.fat
        { "ext2", 16 },   // man 8 mke2fs
        { "ext3", 16 },   // man 8 mke2fs
        { "ext4", 16 },   // man 8 mke2fs
        { "btrfs", 255 },   // https://btrfs.wiki.kernel.org/index.php/Manpage/btrfs-filesystem
        { "f2fs", 512 },   // https://www.kernel.org/doc/Documentation/filesystems/f2fs.txt    https://git.kernel.org/pub/scm/linux/kernel/git/jaegeuk/f2fs-tools.git/tree/mkfs/f2fs_format_main.c
        { "jfs", 16 },   // jfsutils/mkfs/mkfs.c:730
        { "exfat", 15 },   // man 8 mkexfatfs
        { "nilfs2", 80 },   // man 8 mkfs.nilfs2
        { "ntfs", 32 },   // https://docs.microsoft.com/en-us/dotnet/api/system.io.driveinfo.volumelabel?view=netframework-4.8
        { "reiserfs", 15 },   // man 8 mkreiserfs said its max length is 16, but after tested, only 15 chars are accepted.
        { "xfs", 12 }   // https://github.com/edward6/reiser4progs/blob/master/include/reiser4/types.h fs_hint_t
    };
    return datas.value(fileSystem.toLower(), 11);
}

bool FileUtils::isGvfsFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/|^/media/[\\s\\S]*/smbmounts)" };
    // TODO(xust) /media/$USER/smbmounts might be changed in the future.
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}

bool FileUtils::isMtpFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { R"(^/run/user/\d+/gvfs/mtp:host|^/root/.gvfs/mtp:host)" };
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}

bool FileUtils::isGphotoFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { R"(^/run/user/\d+/gvfs/gphoto2:host|^/root/.gvfs/gphoto2:host)" };
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}

QString FileUtils::preprocessingFileName(QString name)
{
    // eg: [\\:*\"?<>|\r\n]
    const QString &value = Application::genericObtuselySetting()->value("FileName", "non-allowableCharacters").toString();

    if (value.isEmpty())
        return name;

    return name.remove(QRegularExpression(value));
}

bool FileUtils::processLength(const QString &srcText, int srcPos, int maxLen, bool useCharCount, QString &dstText, int &dstPos)
{
    auto textLength = [&](const QString &text) {
        return useCharCount ? text.length() : text.toLocal8Bit().length();
    };

    int editTextCurrLen = textLength(srcText);
    int editTextRangeOutLen = editTextCurrLen - maxLen;
    if (editTextRangeOutLen > 0 && maxLen != INT_MAX) {
        QString leftText = srcText.left(srcPos);
        QString rightText = srcText.mid(srcPos);

        while (textLength(leftText + rightText) > maxLen) {
            if (leftText.isEmpty())
                return false;

            auto list = leftText.toUcs4();
            list.removeLast();
            leftText = QString::fromUcs4(list.data(), list.size());
        }

        dstPos = leftText.size();
        dstText = leftText + rightText;
        return srcText.size() != dstText.size();
    } else {
        dstText = srcText;
        dstPos = srcPos;
        return false;
    }
}

bool FileUtils::isContainProhibitPath(const QList<QUrl> &urls)
{
    QStringList prohibitPaths;
    static const QStringList &kKeys { "Desktop", "Videos", "Music", "Pictures", "Documents", "Downloads" };
    auto addPathFunc = [&prohibitPaths](const QString &user = {}) {
        std::for_each(kKeys.begin(), kKeys.end(), [&prohibitPaths, &user](const QString &key) {
            const QString &path = user.isEmpty() ? SystemPathUtil::instance()->systemPath(key)
                                                 : SystemPathUtil::instance()->systemPathOfUser(key, user);
            const QString &bindPath { FileUtils::bindPathTransform(path, true) };
            prohibitPaths.append(path);
            if (!prohibitPaths.contains(bindPath))
                prohibitPaths.append(bindPath);
        });
    };

    // root user can access all home dirs
    if (Q_UNLIKELY(SysInfoUtils::isRootUser())) {
        const QStringList &userNames { SysInfoUtils::getAllUsersOfHome() };
        std::for_each(userNames.begin(), userNames.end(), [&addPathFunc](const QString &name) {
            addPathFunc(name);
        });
    }
    // add self
    addPathFunc();

    return std::any_of(urls.begin(), urls.end(), [&prohibitPaths](const QUrl &url) {
        return (!url.isEmpty() && prohibitPaths.contains(url.path()));
    });
}

bool FileUtils::isDesktopFile(const QUrl &url)
{
    // At present, there is no dfmio library code. For temporary repair, use the method on v20 to obtain mimeType
    auto info = InfoFactory::create<FileInfo>(url);
    if (!info)
        return false;
    return isDesktopFileInfo(info);
}

bool FileUtils::isDesktopFileSuffix(const QUrl &url)
{
    // It's not rigorous,
    // but there are interfaces that call "isDesktopFile"
    // so often that it would be a performance loss to
    // create a fileinfo
    return url.toString().endsWith(".desktop");
}

bool FileUtils::isDesktopFileInfo(const FileInfoPointer &info)
{
    Q_ASSERT(info);
    const QString &suffix = info->nameOf(NameInfoType::kSuffix);
    if (suffix == DFMBASE_NAMESPACE::Global::Scheme::kDesktop
        || info->urlOf(UrlInfoType::kParentUrl).path() == StandardPaths::location(StandardPaths::StandardLocation::kDesktopPath)
        || info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool()) {
        const QUrl &url = info->urlOf(UrlInfoType::kUrl);
        QMimeType type = info->fileMimeType();
        if (!type.isValid())
            type = DMimeDatabase().mimeTypeForFile(url.path(), QMimeDatabase::MatchDefault, QString());

        // QMimeType::suffixes is not the same as fileinfo's `kSuffix`.
        if (type.name() == "application/x-desktop"
            && type.suffixes().contains(DFMBASE_NAMESPACE::Global::Scheme::kDesktop, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

bool FileUtils::isTrashDesktopFile(const QUrl &url)
{
    if (isDesktopFileSuffix(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == kDDETrashId;
    }
    return false;
}

bool FileUtils::isComputerDesktopFile(const QUrl &url)
{
    if (isDesktopFileSuffix(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == kDDEComputerId;
    }
    return false;
}

bool FileUtils::isHomeDesktopFile(const QUrl &url)
{
    if (isDesktopFileSuffix(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == kDDEHomeId;
    }
    return false;
}

bool FileUtils::isSameDevice(const QUrl &url1, const QUrl &url2)
{
    if (url1.scheme() != url2.scheme())
        return false;

    if (isLocalFile(url1)) {
        return DFMIO::DFMUtils::devicePathFromUrl(url1) == DFMIO::DFMUtils::devicePathFromUrl(url2);
    }

    return url1.host() == url2.host() && url1.port() == url1.port();
}

bool FileUtils::isSameFile(const QUrl &url1, const QUrl &url2, const Global::CreateFileInfoType infoCache)
{
    if (UniversalUtils::urlEquals(url1, url2))
        return true;

    auto info1 = InfoFactory::create<FileInfo>(url1, infoCache);
    auto info2 = InfoFactory::create<FileInfo>(url2, infoCache);
    if (!info1 || !info2)
        return false;

    struct stat statFromInfo;
    struct stat statToInfo;

    const QString &path1 = info1->pathOf(PathInfoType::kAbsoluteFilePath);
    const QString &path2 = info2->pathOf(PathInfoType::kAbsoluteFilePath);
    int fromStat = stat(path1.toLocal8Bit().data(), &statFromInfo);
    int toStat = stat(path2.toLocal8Bit().data(), &statToInfo);
    if (0 == fromStat && 0 == toStat) {
        // 通过inode判断是否是同一个文件
        if (statFromInfo.st_ino == statToInfo.st_ino
            && statFromInfo.st_dev == statToInfo.st_dev) {   //! 需要判断设备号
            return true;
        }
    }
    return false;
}

bool FileUtils::isLocalDevice(const QUrl &url)
{
    //return !DFMIO::DFMUtils::fileIsRemovable(url) && !isGvfsFile(url);
    if (isGvfsFile(url))
        return false;

    if (DeviceUtils::isExternalBlock(url))
        return false;

    if (DevProxyMng->isFileOfProtocolMounts(url.path()))
        return false;

    return true;
}

bool FileUtils::isCdRomDevice(const QUrl &url)
{
    return DFMIO::DFMUtils::devicePathFromUrl(url).startsWith("/dev/sr");
}

bool FileUtils::trashIsEmpty()
{
    // not use cache, because some times info unreliable, such as watcher inited temporality
    auto info = InfoFactory::create<FileInfo>(trashRootUrl(), Global::CreateFileInfoType::kCreateFileInfoSync);
    if (info) {
        return info->countChildFile() == 0;
    }
    return true;
}

QUrl FileUtils::trashRootUrl()
{
    QUrl url;
    url.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kTrash);
    url.setPath("/");
    url.setHost("");
    return url;
}

bool FileUtils::isTrashFile(const QUrl &url)
{
    if (url.scheme() == DFMBASE_NAMESPACE::Global::Scheme::kTrash)
        return true;
    if (url.path().startsWith(StandardPaths::location(StandardPaths::kTrashLocalFilesPath)))
        return true;

    const QString &rule = QString("/.Trash-%1/(files|info)/").arg(getuid());
    QRegularExpression reg(rule);
    QRegularExpressionMatch matcher = reg.match(url.toString());
    return matcher.hasMatch();
}

bool FileUtils::isTrashRootFile(const QUrl &url)
{
    if (UniversalUtils::urlEquals(url, trashRootUrl()))
        return true;

    if (UniversalUtils::urlEquals(url, QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashLocalFilesPath))))
        return true;

    const QString &rule = QString("/.Trash-%1/files").arg(getuid());

    return url.toString().endsWith(rule);
}

bool FileUtils::isHigherHierarchy(const QUrl &urlBase, const QUrl &urlCompare)
{
    QUrl url = urlCompare;
    while (url.isValid() && url != QUrl::fromLocalFile(R"(/)")) {
        if (urlBase.isParentOf(url))
            return true;
        url = DFMIO::DFMUtils::directParentUrl(url);
    }
    return false;
}

bool FileUtils::isLocalFile(const QUrl &url)
{
    if (url.isLocalFile())
        return true;

    // see if the original path is from local.
    // since only ext* filesystems are supported to mounted with dlnfs,
    // check the url by udisks.
    // the dlnfs mount is captured by gvfs and is regarded as protocol device.
    // so if it's NOT external block mounts file, it's local file.
    if (DeviceUtils::isSubpathOfDlnfs(url.path()))
        return !(DevProxyMng->isFileOfExternalBlockMounts(url.path()));

    return false;
}

/*!
 * \brief FileUtils::getFileNameLength, if the `url` is suburl of dlnfs mountpoint, then use char count rather than byte count to judge filename length.
 * \param url
 * \param name
 * \return
 */
int FileUtils::getFileNameLength(const QUrl &url, const QString &name)
{
    return FileUtils::supportLongName(url) ? name.length() : name.toLocal8Bit().length();
}

QMap<QUrl, QUrl> FileUtils::fileBatchReplaceText(const QList<QUrl> &originUrls, const QPair<QString, QString> &pair)
{
    if (originUrls.isEmpty()) {
        return QMap<QUrl, QUrl> {};
    }

    QMap<QUrl, QUrl> result;

    for (const auto &url : originUrls) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);

        if (!info)
            continue;

        bool isDesktopApp = info->nameOf(NameInfoType::kMimeTypeName).contains(Global::Mime::kTypeAppDesktop);

        ///###: symlink is also processed here.
        const QString &suffix = info->nameOf(NameInfoType::kSuffix).isEmpty()
                ? QString()
                : QString(".") + info->nameOf(NameInfoType::kSuffix);
        QString fileBaseName;
        if (isDesktopApp) {
            fileBaseName = info->displayOf(DisPlayInfoType::kFileDisplayName);
        } else {
            fileBaseName = info->nameOf(NameInfoType::kFileName);
            fileBaseName.chop(suffix.length());
        }

        fileBaseName.replace(pair.first, pair.second);

        if (fileBaseName.trimmed().isEmpty()) {
            qCWarning(logDFMBase) << "replace fileBaseName(not include suffix) trimmed is empty string";
            continue;
        }

        int maxLength = NAME_MAX - suffix.toLocal8Bit().size();
        fileBaseName = cutFileName(fileBaseName, maxLength, FileUtils::supportLongName(url));

        if (!isDesktopApp) {
            fileBaseName += suffix;
        }
        QUrl changedUrl { info->getUrlByType(UrlInfoType::kGetUrlByNewFileName, fileBaseName) };

        if (changedUrl != url)
            result.insert(url, changedUrl);
    }

    return result;
}

QMap<QUrl, QUrl> FileUtils::fileBatchAddText(const QList<QUrl> &originUrls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &pair)
{
    if (originUrls.isEmpty()) {
        return QMap<QUrl, QUrl> {};
    }

    QMap<QUrl, QUrl> result;

    for (auto url : originUrls) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);

        if (!info)
            continue;

        // debug case 25414: failure to rename desktop app name
        bool isDesktopApp = info->nameOf(NameInfoType::kMimeTypeName).contains(Global::Mime::kTypeAppDesktop);

        QString fileBaseName = isDesktopApp ? info->displayOf(DisPlayInfoType::kFileDisplayName)
                                            : info->nameOf(NameInfoType::kBaseName);   //{ info->baseName() };
        QString oldFileName = fileBaseName;

        QString addText = pair.first;
        const QString &suffix = info->nameOf(NameInfoType::kSuffix).isEmpty()
                ? QString()
                : QString(".") + info->nameOf(NameInfoType::kSuffix);

        int maxLength = NAME_MAX - getFileNameLength(url, info->nameOf(NameInfoType::kFileName));
        addText = cutFileName(addText, maxLength, FileUtils::supportLongName(url));

        if (pair.second == AbstractJobHandler::FileNameAddFlag::kPrefix) {
            fileBaseName.insert(0, addText);
        } else {
            fileBaseName.append(addText);
        }

        if (!isDesktopApp) {
            fileBaseName += suffix;
        }
        QUrl changedUrl = { info->getUrlByType(UrlInfoType::kGetUrlByNewFileName, fileBaseName) };

        if (isDesktopApp) {
            qCDebug(logDFMBase) << "this is desktop app case,file name will be changed { " << oldFileName << " } to { "
                                << fileBaseName << " } for path:" << info->urlOf(UrlInfoType::kUrl);
        }

        if (changedUrl != url)
            result.insert(url, changedUrl);
    }

    return result;
}

QMap<QUrl, QUrl> FileUtils::fileBatchCustomText(const QList<QUrl> &originUrls, const QPair<QString, QString> &pair)
{
    if (originUrls.isEmpty() || pair.first.isEmpty() || pair.second.isEmpty()) {   //###: here, jundge whether there are fileUrls in originUrls.
        return QMap<QUrl, QUrl> {};
    }

    unsigned long long serialNumber { pair.second.toULongLong() };
    unsigned long long index { 0 };

    if (serialNumber == ULONG_LONG_MAX) {   //##: Maybe, this value will be equal to the max value of the type of unsigned long long
        index = serialNumber - originUrls.size();
    } else {
        index = serialNumber;
    }

    QMap<QUrl, QUrl> result;

    QList<QUrl> modifyUrls;

    bool needRecombination = false;
    for (auto url : originUrls) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);

        if (!info)
            continue;

        // debug case 25414: failure to rename desktop app name
        bool isDesktopApp = info->nameOf(NameInfoType::kMimeTypeName).contains(Global::Mime::kTypeAppDesktop);

        QString fileBaseName { pair.first };
        const QString &indexString = QString::number(index);
        const QString &suffix = info->nameOf(NameInfoType::kSuffix).isEmpty()
                ? QString()
                : QString(".") + info->nameOf(NameInfoType::kSuffix);
        int maxLength = NAME_MAX - getFileNameLength(url, indexString) - suffix.toLocal8Bit().size();
        fileBaseName = cutFileName(fileBaseName, maxLength, FileUtils::supportLongName(url));

        fileBaseName = isDesktopApp ? (fileBaseName + indexString) : (fileBaseName + indexString + suffix);
        QUrl beModifieddUrl = { info->getUrlByType(UrlInfoType::kGetUrlByNewFileName, fileBaseName) };
        result.insert(url, beModifieddUrl);

        modifyUrls << beModifieddUrl;

        // 如果源url包含了待修改的url 就需要重组结果
        if (originUrls.contains(beModifieddUrl))
            needRecombination = true;

        if (isDesktopApp) {
            qCDebug(logDFMBase) << "this is desktop app case,file name will be changed as { "
                                << fileBaseName << " } for path:" << info->urlOf(UrlInfoType::kUrl);
        }

        ++index;
    }

    // 重组map
    if (needRecombination) {
        QList<QUrl> originUrlsTemp = originUrls;

        auto it = modifyUrls.begin();
        while (it != modifyUrls.end()) {
            QUrl url = *it;
            if (originUrlsTemp.contains(url)) {
                originUrlsTemp.removeOne(url);
                it = modifyUrls.erase(it);
                continue;
            }
            ++it;
        }

        if (originUrlsTemp.size() == modifyUrls.size()) {
            result.clear();
            for (int i = 0, end = originUrlsTemp.size(); i < end; ++i) {
                result.insert(originUrlsTemp[i], modifyUrls[i]);
            }
        }
    }

    return result;
}

QString FileUtils::cutFileName(const QString &name, int maxLength, bool useCharCount)
{
    QString tmpName = name;
    if (useCharCount) {
        if (tmpName.length() > maxLength)
            tmpName = tmpName.left(maxLength);

        return tmpName;
    }

    if (tmpName.toLocal8Bit().length() <= maxLength)
        return tmpName;

    tmpName.clear();
    int bytes = 0;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    auto codec = QTextCodec::codecForLocale();
#else
    auto encoder = QStringEncoder(QStringEncoder::System);
    auto decoder = QStringDecoder(QStringEncoder::System);
#endif
    for (int i = 0; i < name.size(); ++i) {
        const QChar &ch = name.at(i);
        QByteArray data;
        QString fullChar;

        if (ch.isSurrogate()) {
            if ((++i) >= name.size())
                break;

            const QChar &nextCh = name.at(i);
            if (!ch.isHighSurrogate() || !nextCh.isLowSurrogate())
                break;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            data = codec->fromUnicode(name.data() + i - 1, 2);
#else
            data = encoder.encode(QString(name.data() + i - 1, 2));
#endif
            fullChar.setUnicode(name.data() + i - 1, 2);
        } else {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            data = codec->fromUnicode(name.data() + i, 1);
#else
            data = encoder.encode(QString(name.data() + i, 1));
#endif
            fullChar.setUnicode(name.data() + i, 1);
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if (codec->toUnicode(data) != fullChar) {
            qCWarning(logDFMBase) << "Failed convert" << fullChar << "to" << codec->name() << "coding";
            continue;
        }
#else
        if (decoder.decode(data) != fullChar) {
            qCWarning(logDFMBase) << "Failed convert" << fullChar << "to" << data << "coding";
            continue;
        }
#endif

        bytes += data.size();
        if (bytes > maxLength)
            break;

        tmpName.append(ch);
        if (ch.isSurrogate())
            tmpName.append(name.at(i));
    }

    return tmpName;
}

QString FileUtils::nonExistSymlinkFileName(const QUrl &fileUrl, const QUrl &parentUrl)
{
    const FileInfoPointer &info = InfoFactory::create<FileInfo>(fileUrl);

    if (info && DFMIO::DFile(fileUrl).exists()) {
        QString baseName = info->displayOf(DisPlayInfoType::kFileDisplayName) == info->nameOf(NameInfoType::kFileName)
                ? info->nameOf(NameInfoType::kBaseName)
                : info->displayOf(DisPlayInfoType::kFileDisplayName);
        QString shortcut = QObject::tr("Shortcut");
        QString linkBaseName;

        int number = 0;

        forever {
            if (info->isAttributes(OptInfoType::kIsFile)) {
                if (info->nameOf(NameInfoType::kSuffix).isEmpty()) {
                    if (number == 0) {
                        linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                    } else {
                        linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                    }
                } else {
                    if (number == 0) {
                        linkBaseName = QString("%1 %2.%3").arg(baseName, shortcut, info->nameOf(NameInfoType::kSuffix));
                    } else {
                        linkBaseName = QString("%1 %2%3.%4").arg(baseName, shortcut, QString::number(number), info->nameOf(NameInfoType::kSuffix));
                    }
                }
            } else if (info->isAttributes(OptInfoType::kIsDir)) {
                if (number == 0) {
                    linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                } else {
                    linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                }
            } else if (info->isAttributes(OptInfoType::kIsSymLink)) {
                return QString();
            }

            if (parentUrl.isEmpty()) {
                return linkBaseName;
            }

            QDir parentDir = QDir(parentUrl.path());
            if (parentDir.exists(linkBaseName)) {
                ++number;
            } else {
                //链接文件失效后exists会返回false，通过lstat再次判断链接文件本身是否存在
                auto strLinkPath = parentDir.filePath(linkBaseName).toStdString();
                struct stat st;
                if ((lstat(strLinkPath.c_str(), &st) == 0) && S_ISLNK(st.st_mode))
                    ++number;
                else
                    return linkBaseName;
            }
        }
    }

    return QString();
}

QString FileUtils::toUnicode(const QByteArray &data, const QString &fileName)
{
    if (data.isEmpty())
        return QString();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    const QByteArray &encoding = detectCharset(data, fileName);

    if (QTextCodec *codec = QTextCodec::codecForName(encoding)) {
        return codec->toUnicode(data);
    }
#endif
    return QString::fromLocal8Bit(data);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
QByteArray FileUtils::detectCharset(const QByteArray &data, const QString &fileName)
{
    // Return local encoding if nothing in file.
    if (data.isEmpty()) {
        return QTextCodec::codecForLocale()->name();
    }

    if (QTextCodec *c = QTextCodec::codecForUtfText(data, nullptr)) {
        return c->name();
    }

    DFMBASE_NAMESPACE::DMimeDatabase mimeDatabase;
    const QMimeType &mimeType = fileName.isEmpty() ? mimeDatabase.mimeTypeForData(data) : mimeDatabase.mimeTypeForFileNameAndData(fileName, data);
    const QString &mimetypeName = mimeType.name();
    KEncodingProber::ProberType proberType = KEncodingProber::Universal;

    if (mimetypeName == Global::Mime::kTypeAppXml
        || mimetypeName == Global::Mime::kTypeTextHtml
        || mimetypeName == Global::Mime::kTypeAppXhtmlXml) {
        const QString &_data = QString::fromLatin1(data);
        QRegularExpression pattern("<\\bmeta.+\\bcharset=(?'charset'\\S+?)\\s*['\"/>]");

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        const QString &charset = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                               QRegularExpression::DontCheckSubjectStringMatchOption)
                                         .captured("charset");

        if (!charset.isEmpty()) {
            return charset.toLatin1();
        }

        pattern.setPattern("<\\bmeta\\s+http-equiv=\"Content-Language\"\\s+content=\"(?'language'[a-zA-Z-]+)\"");

        const QString &language = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                                QRegularExpression::DontCheckSubjectStringMatchOption)
                                          .captured("language");

        if (!language.isEmpty()) {
            QLocale l(language);

            switch (l.script()) {
            case QLocale::ArabicScript:
                proberType = KEncodingProber::Arabic;
                break;
            case QLocale::SimplifiedChineseScript:
                proberType = KEncodingProber::ChineseSimplified;
                break;
            case QLocale::TraditionalChineseScript:
                proberType = KEncodingProber::ChineseTraditional;
                break;
            case QLocale::CyrillicScript:
                proberType = KEncodingProber::Cyrillic;
                break;
            case QLocale::GreekScript:
                proberType = KEncodingProber::Greek;
                break;
            case QLocale::HebrewScript:
                proberType = KEncodingProber::Hebrew;
                break;
            case QLocale::JapaneseScript:
                proberType = KEncodingProber::Japanese;
                break;
            case QLocale::KoreanScript:
                proberType = KEncodingProber::Korean;
                break;
            case QLocale::ThaiScript:
                proberType = KEncodingProber::Thai;
                break;
            default:
                break;
            }
        }
    } else if (mimetypeName == Global::Mime::kTypeTextXPython) {
        QRegularExpression pattern("^#coding\\s*:\\s*(?'coding'\\S+)$");
        QTextStream stream(data);

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        stream.setCodec("latin1");

        while (!stream.atEnd()) {
            const QString &_data = stream.readLine();
            const QString &coding = pattern.match(_data, 0).captured("coding");

            if (!coding.isEmpty()) {
                return coding.toLatin1();
            }
        }
    }

    // for CJK
    const QList<QPair<KEncodingProber::ProberType, QLocale::Country>> fallbackList {
        { KEncodingProber::ChineseSimplified, QLocale::China },
        { KEncodingProber::ChineseTraditional, QLocale::China },
        { KEncodingProber::Japanese, QLocale::Japan },
        { KEncodingProber::Korean, QLocale::NorthKorea },
        { KEncodingProber::Cyrillic, QLocale::Russia },
        { KEncodingProber::Greek, QLocale::Greece },
        { proberType, QLocale::system().country() }
    };

    KEncodingProber prober(proberType);
    prober.feed(data);
    float preConfidence = prober.confidence();
    QByteArray preEncoding = prober.encoding();

    QTextCodec *defCodec = QTextCodec::codecForLocale();
    QByteArray encoding;
    float confidence = 0;

    for (auto i : fallbackList) {
        prober.setProberType(i.first);
        prober.feed(data);

        float proberConfidence = prober.confidence();
        QByteArray proberEncoding = prober.encoding();

        if (i.first != proberType && qFuzzyIsNull(proberConfidence)) {
            proberConfidence = preConfidence;
            proberEncoding = preEncoding;
        }

    confidence:
        if (QTextCodec *codec = QTextCodec::codecForName(proberEncoding)) {
            if (defCodec == codec)
                defCodec = nullptr;

            float c = codecConfidenceForData(codec, data, i.second);

            if (proberConfidence > 0.5) {
                c = c / 2 + proberConfidence / 2;
            } else {
                c = c / 3 * 2 + proberConfidence / 3;
            }

            if (c > confidence) {
                confidence = c;
                encoding = proberEncoding;
            }

            if (i.first == KEncodingProber::ChineseTraditional && c < 0.5) {
                // test Big5
                c = codecConfidenceForData(QTextCodec::codecForName("Big5"), data, i.second);

                if (c > 0.5 && c > confidence) {
                    confidence = c;
                    encoding = "Big5";
                }
            }
        }

        if (i.first != proberType) {
            // 使用 proberType 类型探测出的结果再次做编码检查
            i.first = proberType;
            proberConfidence = preConfidence;
            proberEncoding = preEncoding;
            goto confidence;
        }
    }

    if (defCodec && codecConfidenceForData(defCodec, data, QLocale::system().country()) > confidence) {
        return defCodec->name();
    }

    return encoding;
}
#endif

/*!
 * \brief FileUtils::getMemoryPageSize 获取当前內存页大小
 * \return 返回内存页大小
 */
quint16 FileUtils::getMemoryPageSize()
{
    static const quint16 memoryPageSize = static_cast<quint16>(getpagesize());
    return memoryPageSize > 0 ? memoryPageSize : kDefaultMemoryPageSize;
}

qint32 FileUtils::getCpuProcessCount()
{
    static const int cpuProcessCount = static_cast<int>(sysconf(_SC_NPROCESSORS_CONF));
    return cpuProcessCount;
}

void FileUtils::cacheCopyingFileUrl(const QUrl &url)
{
    QMutexLocker locker(&cacheCopyingMutex);

    copyingUrl.insert(url);
}

void FileUtils::removeCopyingFileUrl(const QUrl &url)
{
    QMutexLocker locker(&cacheCopyingMutex);
    copyingUrl.remove(url);
}

bool FileUtils::containsCopyingFileUrl(const QUrl &url)
{
    QMutexLocker locker(&cacheCopyingMutex);
    return copyingUrl.contains(url);
}

// TODO: remot it!
void FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType type, const QUrl &url)
{
    if (!url.isValid())
        return;

    auto isRemoteMount = [=](const QUrl &url) -> bool {
        if (DeviceUtils::isSamba(url))
            return true;
        if (DeviceUtils::isFtp(url))
            return true;

        return false;
    };

    if (!isRemoteMount(url))
        return;

    const QUrl &urlParent = DFMIO::DFMUtils::directParentUrl(url);
    if (!urlParent.isValid())
        return;

    AbstractFileWatcherPointer watcher = WatcherFactory::create<AbstractFileWatcher>(DFMIO::DFMUtils::directParentUrl(url));
    if (!watcher)
        return;

    switch (type) {
    case DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded:
        watcher->notifyFileAdded(url);
        return;
    case DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted:
        watcher->notifyFileDeleted(url);
        return;
    case DFMBASE_NAMESPACE::Global::FileNotifyType::kFileChanged:
        watcher->notifyFileChanged(url);
        return;
    default:
        return;
    }
}
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

bool FileUtils::isNumOrChar(const QChar ch)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    return (ch >= 48 && ch <= 57) || (ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122);
#else
    auto chValue = ch.unicode();
    return (chValue >= 48 && chValue <= 57) || (chValue >= 65 && chValue <= 90) || (chValue >= 97 && chValue <= 122);
#endif
}

bool FileUtils::isNumber(const QChar ch)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    return (ch >= 48 && ch <= 57);
#else
    auto chValue = ch.unicode();
    return (chValue >= 48 && chValue <= 57);
#endif
}

bool FileUtils::isSymbol(const QChar ch)
{
    return ch.script() != QChar::Script_Han && !isNumOrChar(ch);
}

QString FileUtils::numberStr(const QString &str, int pos)
{
    QString tmp;
    auto total = str.length();

    while (pos > 0 && isNumber(str.at(pos))) {
        pos--;
    }

    if (!isNumber(str.at(pos)))
        pos++;

    while (pos < total && isNumber(str.at(pos))) {
        tmp += str.at(pos);
        pos++;
    }

    return tmp;
}

// 升序，第一个小，true
bool FileUtils::compareByStringEx(const QString &str1, const QString &str2)
{
    thread_local static DCollator sortCollator;
    QString suf1 = str1.right(str1.length() - str1.lastIndexOf(".") - 1);
    QString suf2 = str2.right(str2.length() - str2.lastIndexOf(".") - 1);
    QString name1 = str1.left(str1.lastIndexOf("."));
    QString name2 = str2.left(str2.lastIndexOf("."));
    int length1 = name1.length();
    int length2 = name2.length();
    auto total = length1 > length2 ? length2 : length1;

    bool preIsNum = false;
    bool isSybol1 = false, isSybol2 = false, isHanzi1 = false,
         isHanzi2 = false, isNumb1 = false, isNumb2 = false;
    for (int i = 0; i < total; ++i) {
        // 判断相等和大小写相等，跳过
        if (str1.at(i) == str2.at(i) || str1.at(i).toLower() == str2.at(i).toLower()) {
            preIsNum = isNumber(str1.at(i));
            continue;
        }
        isNumb1 = isNumber(str1.at(i));
        isNumb2 = isNumber(str2.at(i));
        if ((preIsNum && (isNumb1 ^ isNumb2)) || (isNumb1 && isNumb2)) {
            // 取后面几位的数字作比较后面的数字,先比较位数
            // 位数大的大
            auto str1n = numberStr(str1, preIsNum ? i - 1 : i).toUInt();
            auto str2n = numberStr(str2, preIsNum ? i - 1 : i).toUInt();
            if (str1n == str2n)
                return str1.at(i) < str2.at(i);
            return str1n < str2n;
        }

        // 判断特殊字符就排到最后
        isSybol1 = isSymbol(str1.at(i));
        isSybol2 = isSymbol(str2.at(i));
        if (isSybol1 ^ isSybol2)
            return !isSybol1;

        if (isSybol1)
            return str1.at(i) < str2.at(i);

        // 判断汉字
        isHanzi1 = str1.at(i).script() == QChar::Script_Han;
        isHanzi2 = str2.at(i).script() == QChar::Script_Han;
        if (isHanzi2 ^ isHanzi1)
            return !isHanzi1;

        if (isHanzi1)
            return sortCollator.compare(str1.at(i), str2.at(i)) < 0;

        // 判断数字或者字符
        if (!isNumb1 && !isNumb2)
            return str1.at(i).toLower() < str2.at(i).toLower();

        return isNumb1;
    }

    if (length1 == length2) {
        if (suf1.isEmpty() ^ suf2.isEmpty())
            return suf1.isEmpty();

        if (suf2.startsWith(suf1) ^ suf1.startsWith(suf2))
            return suf2.startsWith(suf1);

        return suf1 < suf2;
    }

    return length1 < length2;
}

bool FileUtils::compareString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    return !((order == Qt::AscendingOrder) ^ compareByStringEx(str1, str2));
}

QString FileUtils::encryptString(const QString &str)
{
    QByteArray byteArray = str.toUtf8();
    QByteArray encodedByteArray = byteArray.toBase64();
    return QString::fromUtf8(encodedByteArray);
}

QString FileUtils::decryptString(const QString &str)
{
    QByteArray encodedByteArray = str.toUtf8();
    QByteArray decodedByteArray = QByteArray::fromBase64(encodedByteArray);
    return QString::fromUtf8(decodedByteArray);
}

QString FileUtils::dateTimeFormat()
{
    return "yyyy/MM/dd HH:mm:ss";
}

bool FileUtils::setBackGround(const QString &pictureFilePath)
{
    if (QFileInfo::exists("/var/lib/deepin/permission-manager/wallpaper_locked")) {
        QDBusInterface notify("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
        notify.asyncCall(QString("Notify"),
                         QString("dde-file-manager"),   // title
                         static_cast<uint>(0),
                         QString("dde-file-manager"),   // icon
                         QObject::tr("This system wallpaper is locked. Please contact your admin."),
                         QString(), QStringList(), QVariantMap(), 5000);
        qCInfo(logDFMBase) << "wallpaper is locked..";
        return false;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(APPEARANCE_SERVICE,
                                                          APPEARANCE_PATH,
                                                          APPEARANCE_SERVICE,
                                                          "Set");
    message.setArguments({ "greeterbackground", pictureFilePath });
    QDBusConnection::sessionBus().asyncCall(message);
    qCInfo(logDFMBase) << "setgreeterbackground calls Appearance Set";

    QDBusMessage msgIntrospect = QDBusMessage::createMethodCall(APPEARANCE_SERVICE,
                                                                APPEARANCE_PATH,
                                                                "org.freedesktop.DBus.Introspectable",
                                                                "Introspect");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msgIntrospect);
    call.waitForFinished();
    if (call.isFinished()) {
        QDBusReply<QString> reply = call.reply();
        QString value = reply.value();

        if (value.contains("SetMonitorBackground")) {
            QDBusMessage msg = QDBusMessage::createMethodCall(APPEARANCE_SERVICE,
                                                              APPEARANCE_PATH,
                                                              APPEARANCE_SERVICE,
                                                              "SetMonitorBackground");
            const QString screen = qApp->primaryScreen()->name();
            msg.setArguments({ screen, pictureFilePath });
            QDBusConnection::sessionBus().asyncCall(msg);
            qCInfo(logDFMBase) << "setBackground calls Appearance SetMonitorBackground" << screen;
            return true;
        }
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(APPEARANCE_SERVICE,
                                                      APPEARANCE_PATH,
                                                      APPEARANCE_SERVICE,
                                                      "Set");
    msg.setArguments({ "Background", pictureFilePath });
    QDBusConnection::sessionBus().asyncCall(msg);
    qCInfo(logDFMBase) << "setBackground calls Appearance Set";

    return true;
}

QString FileUtils::nonExistFileName(FileInfoPointer fromInfo, FileInfoPointer targetDir)
{
    if (!targetDir || !DFMIO::DFile(targetDir->urlOf(UrlInfoType::kUrl)).exists()) {
        return QString();
    }

    if (!targetDir->isAttributes(OptInfoType::kIsDir)) {
        return QString();
    }

    const QString &copySuffix = QObject::tr(" (copy)", "this should be translated in Noun version rather Verb, the first space should be ignore if translate to Chinese");
    const QString &copySuffix2 = QObject::tr(" (copy %1)", "this should be translated in Noun version rather Verb, the first space should be ignore if translate to Chinese");

    QString fileBaseName = fromInfo->nameOf(NameInfoType::kCompleteBaseName);
    QString suffix = fromInfo->nameOf(NameInfoType::kSuffix);
    QString fileName = fromInfo->nameOf(NameInfoType::kFileName);
    //在7z分卷压缩后的名称特殊处理7z.003
    const QString &reg = ".7z.[0-9]{3,10}$";
    if (fileName.contains(QRegularExpression(reg))) {
        const int &index = fileName.indexOf(QRegularExpression(reg));
        fileBaseName = fileName.left(index);
        suffix = fileName.mid(index + 1);
    }

    int number = 0;
    QString newFileName;

    QUrl newUrl = targetDir->urlOf(UrlInfoType::kUrl);

    do {
        auto nameSuffix = number > 0 ? copySuffix2.arg(number) : copySuffix;
        newFileName = QString("%1%2").arg(fileBaseName, nameSuffix);

        if (!suffix.isEmpty()) {
            newFileName.append('.').append(suffix);
        }

        ++number;
        newUrl = targetDir->urlOf(UrlInfoType::kUrl);
        newUrl.setPath(newUrl.path() + "/" + newFileName);
    } while (DFMIO::DFile(newUrl).exists());

    return newFileName;
}

QString FileUtils::bindPathTransform(const QString &path, bool toDevice)
{
    return DeviceUtils::bindPathTransform(path, toDevice);
}

int FileUtils::dirFfileCount(const QUrl &url)
{
    if (!url.isValid())
        return 0;
    DFMIO::DEnumerator enumerator(url);
    return int(enumerator.fileCount());
}

bool FileUtils::fileCanTrash(const QUrl &url)
{
    // gio does not support root user to move ordinary user files to trash
    auto info = InfoFactory::create<FileInfo>(url);
    if (SysInfoUtils::isRootUser()) {
        int ownerId = info.isNull() ? -1 : info->extendAttributes(FileInfo::FileExtendedInfoType::kOwnerId).toInt();
        if (ownerId != 0)
            return false;
    }

    // 获取当前配置
    bool alltotrash = DConfigManager::instance()->value(kDefaultCfgPath, kFileAllTrash).toBool();
    if (!alltotrash)
        return info ? info->extendAttributes(ExtInfoType::kFileLocalDevice).toBool() : isLocalDevice(url);
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/)" };
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return !match.hasMatch();
}

QUrl FileUtils::bindUrlTransform(const QUrl &url)
{
    auto tmp = url;

    if (!FileUtils::isTrashFile(url) || !url.path().contains("\\")) {
        tmp.setPath(FileUtils::bindPathTransform(url.path(), false));
        return tmp;
    }

    auto path = FileUtils::trashPathToNormal(url.path());
    path = FileUtils::bindPathTransform(path, false);
    path = FileUtils::normalPathToTrash(path);
    tmp.setPath(path);
    return tmp;
}

QString FileUtils::trashPathToNormal(const QString &trash)
{
    if (!trash.contains("\\"))
        return trash;
    QString normal = trash;
    normal = normal.replace("\\", "/");
    normal = normal.replace("//", "/");
    return normal;
}

QString FileUtils::normalPathToTrash(const QString &normal)
{
    QString trash = normal;
    trash = trash.replace("/", "\\");
    trash.push_front("/");
    return trash;
}

bool FileUtils::supportLongName(const QUrl &url)
{
    const static QList<QString> datas {
        "vfat", "exfat", "ntfs", "fuseblk", "fuse.dlnfs"
    };

    const QString &fileSystem = dfmio::DFMUtils::fsTypeFromUrl(url);
    return datas.contains(fileSystem) || DeviceUtils::isSubpathOfDlnfs(url.path());
}

QUrl DesktopAppUrl::trashDesktopFileUrl()
{
    static QUrl trash = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-trash.desktop");
    return trash;
}

QUrl DesktopAppUrl::computerDesktopFileUrl()
{
    static QUrl computer = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-computer.desktop");
    return computer;
}

QUrl DesktopAppUrl::homeDesktopFileUrl()
{
    static QUrl home = QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kDesktopPath) + "/dde-home.desktop");
    return home;
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
///###: Do not modify it.
///###: it's auxiliary.
float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country)
{
    qreal hepCount = 0;
    int nonBaseLatinCount = 0;
    qreal unidentificationCount = 0;
    int replacementCount = 0;

    QTextDecoder decoder(codec);
    const QString &unicodeData = decoder.toUnicode(data);

    for (int i = 0; i < unicodeData.size(); ++i) {
        const QChar &ch = unicodeData.at(i);

        if (ch.unicode() > 0x7f)
            ++nonBaseLatinCount;

        switch (ch.script()) {
        case QChar::Script_Hiragana:
        case QChar::Script_Katakana:
            hepCount += country == QLocale::Japan ? 1.2 : 0.5;
            unidentificationCount += country == QLocale::Japan ? 0 : 0.3;
            break;
        case QChar::Script_Han:
            hepCount += country == QLocale::China ? 1.2 : 0.5;
            unidentificationCount += country == QLocale::China ? 0 : 0.3;
            break;
        case QChar::Script_Hangul:
            hepCount += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 1.2 : 0.5;
            unidentificationCount += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 0 : 0.3;
            break;
        case QChar::Script_Cyrillic:
            hepCount += (country == QLocale::Russia) ? 1.2 : 0.5;
            unidentificationCount += (country == QLocale::Russia) ? 0 : 0.3;
            break;
        case QChar::Script_Greek:
            hepCount += (country == QLocale::Greece) ? 1.2 : 0.5;
            unidentificationCount += (country == QLocale::Greece) ? 0 : 0.3;
            break;
        default:
            // full-width character, emoji, 常用标点, 拉丁文补充1，天城文及其补充，CJK符号和标点符号（如：【】）
            if ((ch.unicode() >= 0xff00 && ch <= 0xffef)
                || (ch.unicode() >= 0x2600 && ch.unicode() <= 0x27ff)
                || (ch.unicode() >= 0x2000 && ch.unicode() <= 0x206f)
                || (ch.unicode() >= 0x80 && ch.unicode() <= 0xff)
                || (ch.unicode() >= 0xa8e0 && ch.unicode() <= 0xa8ff)
                || (ch.unicode() >= 0x0900 && ch.unicode() <= 0x097f)
                || (ch.unicode() >= 0x3000 && ch.unicode() <= 0x303f)) {
                ++hepCount;
            } else if (ch.isSurrogate() && ch.isHighSurrogate()) {
                ++i;

                if (i < unicodeData.size()) {
                    const QChar &next_ch = unicodeData.at(i);

                    if (!next_ch.isLowSurrogate()) {
                        --i;
                        break;
                    }

                    uint unicode = QChar::surrogateToUcs4(ch, next_ch);

                    // emoji
                    if (unicode >= 0x1f000 && unicode <= 0x1f6ff) {
                        hepCount += 2;
                    }
                }
            } else if (ch.unicode() == QChar::ReplacementCharacter) {
                ++replacementCount;
            } else if (ch.unicode() > 0x7f) {
                // 因为UTF-8编码的容错性很低，所以未识别的编码只需要判断是否为 QChar::ReplacementCharacter 就能排除
                if (codec->name() != "UTF-8")
                    ++unidentificationCount;
            }
            break;
        }
    }

    // blumia: not sure why original author assume non_base_latin_count must greater than zero...
    if (nonBaseLatinCount == 0) {
        return 1.0f;
    }

    float c = static_cast<float>(qreal(hepCount) / nonBaseLatinCount / 1.2);

    c -= static_cast<float>(qreal(replacementCount) / nonBaseLatinCount);
    c -= static_cast<float>(qreal(unidentificationCount) / nonBaseLatinCount);

    return qMax(0.0f, c);
}
#endif

Match::Match(const QString &group)
{
    for (const QString &key : Application::instance()->genericObtuselySetting()->keys(group)) {
        const QString &value = Application::instance()->genericObtuselySetting()->value(group, key).toString();

        int last_dir_split = value.lastIndexOf(QDir::separator());

        if (last_dir_split >= 0) {
            QString path = value.left(last_dir_split);

            if (path.startsWith("~/")) {
                path.replace(0, 1, QDir::homePath());
            }

            patternList << qMakePair(path, value.mid(last_dir_split + 1));
        } else {
            patternList << qMakePair(QString(), value);
        }
    }
}

bool Match::match(const QString &path, const QString &name)
{
    // 这里可能会析构 先复制一份再循环
    const QList<QPair<QString, QString>> patternListNew = patternList;
    for (auto pattern : patternListNew) {
        QRegularExpression re(QString(), QRegularExpression::MultilineOption);

        if (!pattern.first.isEmpty()) {
            re.setPattern(pattern.first);

            if (!re.isValid()) {
                qCWarning(logDFMBase) << re.errorString();
                continue;
            }

            if (!re.match(path).hasMatch()) {
                continue;
            }
        }

        if (pattern.second.isEmpty()) {
            return true;
        }

        re.setPattern(pattern.second);

        if (!re.isValid()) {
            qCWarning(logDFMBase) << re.errorString();
            continue;
        }

        if (re.match(name).hasMatch()) {
            return true;
        }
    }

    return false;
}

}
