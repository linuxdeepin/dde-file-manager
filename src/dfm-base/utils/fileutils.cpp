/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "utils/fileutils.h"
#include "desktopfile.h"
#include "windowutils.h"
#include "dfm_global_defines.h"
#include "mimetype/mimedatabase.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/finallyutil.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/mimetype/dmimedatabase.h"

#include <KCodecs>
#include <KEncodingProber>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QTextCodec>
#include <QSet>
#include <QRegularExpression>
#include <QCollator>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QScreen>
#include <QStandardPaths>

#include <unistd.h>
#include <sys/stat.h>

namespace dfmbase {

static constexpr char kDDETrashId[] { "dde-trash" };
static constexpr char kDDEComputerId[] { "dde-computer" };
static constexpr char kDDEHomeId[] { "dde-home" };
static constexpr char kSharePixmapPath[] { "/usr/share/pixmaps" };
const static int kDefaultMemoryPageSize = 4096;

QMutex FileUtils::cacheCopyingMutex;
QSet<QUrl> FileUtils::copyingUrl;

static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country);

/*!
 * \class FileUtils
 *
 * \brief Utility class providing static helper methods for file management
 */

bool FileUtils::mkdir(const QUrl &url, const QString &dirName, QString *errorString)
{
    QString error;
    FinallyUtil finally([&]() {if (errorString) *errorString = error; });
    if (!url.isValid()) {
        error = "Failed, can't use empty url from create dir";
        return false;
    }

    if (UrlRoute::isVirtual(url)) {
        error = "Failed, can't use virtual url from create dir";
        return false;
    }

    QFileInfo info(UrlRoute::urlToPath(url));
    if (!info.exists()) {
        error = "Failed, url not exists";
        return false;
    }

    if (!info.isDir()) {
        error = "Failed, url pat not is dir";
        return false;
    }

    if (!info.permissions().testFlag(QFile::Permission::WriteUser)) {
        error = "Failed, not permission create dir";
        return false;
    }

    auto localDirPath = info.filePath() + "/" + dirName;
    if (QFileInfo(localDirPath).exists()) {
        error = QString("Failed, current dir is exists \n %0").arg(localDirPath);
        return false;
    }

    if (!QDir().mkdir(localDirPath)) {
        error = "Failed, unknown error from create new dir";
        return false;
    }

    finally.dismiss();
    return true;
}

bool FileUtils::touch(const QUrl &url,
                      const QString &fileName,
                      QString *errorString)
{
    QString error;
    FinallyUtil finally([&]() {if (errorString) *errorString = error; });

    if (!url.isValid()) {
        error = "Failed, can't use empty url from create dir";
        return false;
    }

    if (UrlRoute::isVirtual(url)) {
        error = "Failed, can't use virtual url from create dir";
        return false;
    }

    if (!UrlRoute::isVirtual(url)) {
        QFileInfo info(UrlRoute::urlToPath(url));
        if (!info.isDir() || !info.exists()) {
            error = "Failed, Fileinfo error";
            return false;
        }
        if (!info.permissions().testFlag(QFile::Permission::WriteUser)) {
            error = "Failed, Fileinfo permission error";
            return false;
        }
        QFile file(info.path() + "/" + fileName);
        if (!file.open(QIODevice::Truncate)) {
            error = "Failed, Create new dir unknown error";
            return false;
        }
    }

    finally.dismiss();
    return true;
}

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
        qWarning() << "Negative number passed to formatSize():" << num;
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
#define pair(key, val) std::pair<QString, int>(key, val)
    const static QMap<QString, int> datas {
        pair("vfat", 11),   // man 8 mkfs.fat
        pair("ext2", 16),   // man 8 mke2fs
        pair("ext3", 16),   // man 8 mke2fs
        pair("ext4", 16),   // man 8 mke2fs
        pair("btrfs", 255),   // https://btrfs.wiki.kernel.org/index.php/Manpage/btrfs-filesystem
        pair("f2fs", 512),   // https://www.kernel.org/doc/Documentation/filesystems/f2fs.txt    https://git.kernel.org/pub/scm/linux/kernel/git/jaegeuk/f2fs-tools.git/tree/mkfs/f2fs_format_main.c
        pair("jfs", 16),   // jfsutils/mkfs/mkfs.c:730
        pair("exfat", 15),   // man 8 mkexfatfs
        pair("nilfs2", 80),   // man 8 mkfs.nilfs2
        pair("ntfs", 32),   // https://docs.microsoft.com/en-us/dotnet/api/system.io.driveinfo.volumelabel?view=netframework-4.8
        pair("reiserfs", 15),   // man 8 mkreiserfs said its max length is 16, but after tested, only 15 chars are accepted.
        pair("xfs", 12)   // https://github.com/edward6/reiser4progs/blob/master/include/reiser4/types.h fs_hint_t
    };
    return datas.value(fileSystem.toLower(), 11);
}

bool FileUtils::isGvfsFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/|^/media/[\\s\\S]*/smbmounts)" };   // TODO(xust) /media/$USER/smbmounts might be changed in the future.
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}

bool FileUtils::isMtpFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { R"(^/run/user/\d+/gvfs/mtp:host|^root/.gvfs/mtp:host)" };
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

bool FileUtils::isContainProhibitPath(const QList<QUrl> &urls)
{
    // prohibit Paths
    const QStringList kUsrProhibitPaths {
        QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(),
        FileUtils::bindPathTransform(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(), true),

        QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first(),
        FileUtils::bindPathTransform(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first(), true),

        QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(),
        FileUtils::bindPathTransform(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first(), true),

        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(),
        FileUtils::bindPathTransform(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(), true),

        QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first(),
        FileUtils::bindPathTransform(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first(), true),

        QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first(),
        FileUtils::bindPathTransform(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first(), true)
    };

    for (const auto &url : urls) {
        // usr Prohibit Paths

        if (!url.isEmpty() && kUsrProhibitPaths.contains(url.path())) {
            return true;
        }
    }
    return false;
}

bool FileUtils::isDesktopFile(const QUrl &url)
{
    return url.toLocalFile().endsWith(".desktop");
}

bool FileUtils::isTrashDesktopFile(const QUrl &url)
{
    if (isDesktopFile(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == kDDETrashId;
    }
    return false;
}

bool FileUtils::isComputerDesktopFile(const QUrl &url)
{
    if (isDesktopFile(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == kDDEComputerId;
    }
    return false;
}

bool FileUtils::isHomeDesktopFile(const QUrl &url)
{
    if (isDesktopFile(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == kDDEHomeId;
    }
    return false;
}

bool FileUtils::isSameDevice(const QUrl &url1, const QUrl &url2)
{
    if (url1.scheme() != url2.scheme())
        return false;

    if (url1.isLocalFile()) {
        return DFMIO::DFMUtils::devicePathFromUrl(url1) == DFMIO::DFMUtils::devicePathFromUrl(url2);
    }

    return url1.host() == url2.host() && url1.port() == url1.port();
}

bool FileUtils::isSameFile(const QUrl &url1, const QUrl &url2)
{
    struct stat statFromInfo;
    struct stat statToInfo;
    int fromStat = stat(url1.toString().toLocal8Bit().data(), &statFromInfo);
    int toStat = stat(url2.toString().toLocal8Bit().data(), &statToInfo);
    if (0 == fromStat && 0 == toStat) {
        // 通过inode判断是否是同一个文件
        if (statFromInfo.st_ino == statToInfo.st_ino
            && statFromInfo.st_dev == statToInfo.st_dev) {   //! 需要判断设备号
            return true;
        }
    }
    return false;
}

bool FileUtils::isLowSpeedDevice(const QUrl &url)
{
    const QString &path = url.path();

    static QMutex mutex;
    QMutexLocker lk(&mutex);
    static QRegularExpression regExp("^/run/user/\\d+/gvfs/(?<scheme>\\w+(-?)\\w+):\\S*",
                                     QRegularExpression::DotMatchesEverythingOption
                                             | QRegularExpression::DontCaptureOption
                                             | QRegularExpression::OptimizeOnFirstUsageOption);

    const QRegularExpressionMatch &match = regExp.match(path, 0, QRegularExpression::NormalMatch,
                                                        QRegularExpression::DontCheckSubjectStringMatchOption);

    if (match.hasMatch()) {
        const QString &scheme = match.captured("scheme");
        static QStringList schemeList = { QString(Global::Scheme::kMtp),
                                          QString(Global::Scheme::kGPhoto),
                                          QString(Global::Scheme::kGPhoto2),
                                          QString(Global::Scheme::kSmb),
                                          QString(Global::Scheme::kSmbShare),
                                          QString(Global::Scheme::kFtp),
                                          QString(Global::Scheme::kSFtp) };
        return schemeList.contains(scheme);
    }

    const QString &device = dfmio::DFMUtils::devicePathFromUrl(url);

    return device.startsWith(QString(Global::Scheme::kMtp) + "://")
            || device.startsWith(QString(Global::Scheme::kGPhoto) + "://")
            || device.startsWith(QString(Global::Scheme::kGPhoto2) + "://")
            || device.startsWith(QString(Global::Scheme::kSmbShare) + "://")
            || device.startsWith(QString(Global::Scheme::kSmb) + "://");
}

bool FileUtils::isLocalDevice(const QUrl &url)
{
    return !DFMIO::DFMUtils::fileIsRemovable(url.toLocalFile());
}

bool FileUtils::isCdRomDevice(const QUrl &url)
{
    return DFMIO::DFMUtils::devicePathFromUrl(url).startsWith("/dev/sr");
}

bool FileUtils::trashIsEmpty()
{
    const QString &path = StandardPaths::location(StandardPaths::kTrashFilesPath);
    DecoratorFile file(path);
    if (!file.exists())
        return true;

    DecoratorFileEnumerator enumerator(path, QStringList(),
                                       DFMIO::DEnumerator::DirFilter::kAllEntries | DFMIO::DEnumerator::DirFilter::kHidden | DFMIO::DEnumerator::DirFilter::kSystem | DFMIO::DEnumerator::DirFilter::kNoDotAndDotDot);
    return !enumerator.hasNext();
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

QMap<QUrl, QUrl> FileUtils::fileBatchReplaceText(const QList<QUrl> &originUrls, const QPair<QString, QString> &pair)
{
    if (originUrls.isEmpty()) {
        return QMap<QUrl, QUrl> {};
    }

    QMap<QUrl, QUrl> result;

    for (auto url : originUrls) {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

        if (!info)
            continue;

        bool isDesktopApp = info->mimeTypeName().contains(Global::Mime::kTypeAppDesktop);

        ///###: symlink is also processed here.
        QString fileBaseName = isDesktopApp ? info->fileDisplayName() : info->baseName();
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        fileBaseName.replace(pair.first, pair.second);

        if (fileBaseName.trimmed().isEmpty()) {
            qWarning() << "replace fileBaseName(not include suffix) trimmed is empty string";
            continue;
        }

        int maxLength = NAME_MAX - suffix.toLocal8Bit().size();

        if (fileBaseName.toLocal8Bit().size() > maxLength) {
            fileBaseName = cutString(fileBaseName, maxLength, QTextCodec::codecForLocale());
        }

        if (!isDesktopApp) {
            fileBaseName += suffix;
        }
        QUrl changedUrl { info->getUrlByNewFileName(fileBaseName) };

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
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

        if (!info)
            continue;

        // debug case 25414: failure to rename desktop app name
        bool isDesktopApp = info->mimeTypeName().contains(Global::Mime::kTypeAppDesktop);

        QString fileBaseName = isDesktopApp ? info->fileDisplayName() : info->baseName();   //{ info->baseName() };
        QString oldFileName = fileBaseName;

        QString addText = pair.first;
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        int maxLength = NAME_MAX - info->fileName().toLocal8Bit().size();

        if (addText.toLocal8Bit().size() > maxLength) {
            addText = cutString(addText, maxLength, QTextCodec::codecForLocale());
        }

        if (pair.second == AbstractJobHandler::FileNameAddFlag::kPrefix) {
            fileBaseName.insert(0, addText);
        } else {
            fileBaseName.append(addText);
        }

        if (!isDesktopApp) {
            fileBaseName += suffix;
        }
        QUrl changedUrl = { info->getUrlByNewFileName(fileBaseName) };

        if (isDesktopApp) {
            qDebug() << "this is desktop app case,file name will be changed { " << oldFileName << " } to { " << fileBaseName << " } for path:" << info->url();
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
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

        if (!info)
            continue;

        // debug case 25414: failure to rename desktop app name
        bool isDesktopApp = info->mimeTypeName().contains(Global::Mime::kTypeAppDesktop);

        QString fileBaseName { pair.first };
        const QString &indexString = QString::number(index);
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        int maxLength = NAME_MAX - indexString.toLocal8Bit().size() - suffix.toLocal8Bit().size();

        if (fileBaseName.toLocal8Bit().size() > maxLength) {
            fileBaseName = cutString(fileBaseName, maxLength, QTextCodec::codecForLocale());
        }

        fileBaseName = isDesktopApp ? (fileBaseName + indexString) : (fileBaseName + indexString + suffix);
        QUrl beModifieddUrl = { info->getUrlByNewFileName(fileBaseName) };
        result.insert(url, beModifieddUrl);

        modifyUrls << beModifieddUrl;

        // 如果源url包含了待修改的url 就需要重组结果
        if (originUrls.contains(beModifieddUrl))
            needRecombination = true;

        if (isDesktopApp) {
            qDebug() << "this is desktop app case,file name will be changed as { " << fileBaseName << " } for path:" << info->url();
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

QString FileUtils::cutString(const QString &text, int dataByteSize, const QTextCodec *codec)
{
    QString newText;
    int bytes = 0;

    for (int i = 0; i < text.size(); ++i) {
        const QChar &ch = text.at(i);
        QByteArray data;
        QString fullChar;

        if (ch.isSurrogate()) {
            if ((++i) >= text.size())
                break;

            const QChar &nextCh = text.at(i);

            if (!ch.isHighSurrogate() || !nextCh.isLowSurrogate())
                break;

            data = codec->fromUnicode(text.data() + i - 1, 2);
            fullChar.setUnicode(text.data() + i - 1, 2);
        } else {
            data = codec->fromUnicode(text.data() + i, 1);
            fullChar.setUnicode(text.data() + i, 1);
        }

        if (codec->toUnicode(data) != fullChar) {
            qWarning() << "Failed convert" << fullChar << "to" << codec->name() << "coding";
            continue;
        }

        bytes += data.size();

        if (bytes > dataByteSize)
            break;

        newText.append(ch);

        if (ch.isSurrogate())
            newText.append(text.at(i));
    }

    return newText;
}

QString FileUtils::nonExistSymlinkFileName(const QUrl &fileUrl, const QUrl &parentUrl)
{
    const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(fileUrl);

    if (info && DecoratorFile(fileUrl).exists()) {
        QString baseName = info->fileDisplayName() == info->fileName() ? info->baseName() : info->fileDisplayName();
        QString shortcut = QObject::tr("Shortcut");
        QString linkBaseName;

        int number = 1;

        forever {
            if (info->isFile()) {
                if (info->suffix().isEmpty()) {
                    if (number == 1) {

                        linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                    } else {
                        linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                    }
                } else {
                    if (number == 1) {
                        linkBaseName = QString("%1 %2.%3").arg(baseName, shortcut, info->suffix());
                    } else {
                        linkBaseName = QString("%1 %2%3.%4").arg(baseName, shortcut, QString::number(number), info->suffix());
                    }
                }
            } else if (info->isDir()) {
                if (number == 1) {
                    linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                } else {
                    linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                }
            } else if (info->isSymLink()) {
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

    const QByteArray &encoding = detectCharset(data, fileName);

    if (QTextCodec *codec = QTextCodec::codecForName(encoding)) {
        return codec->toUnicode(data);
    }

    return QString::fromLocal8Bit(data);
}

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

/*!
 * \brief FileUtils::getMemoryPageSize 获取当前內存页大小
 * \return 返回内存页大小
 */
quint16 FileUtils::getMemoryPageSize()
{
    static const quint16 memoryPageSize = static_cast<quint16>(getpagesize());
    return memoryPageSize > 0 ? memoryPageSize : kDefaultMemoryPageSize;
}

QIcon FileUtils::searchAppIcon(const DesktopFile &app, const QIcon &defaultIcon)
{
    // Resulting icon
    QIcon icon;

    // First attempt, try load icon from theme
    icon = QIcon::fromTheme(app.desktopIcon());
    if (!icon.isNull()) {
        return icon;
    }

    // Second attempt, check whether icon is a valid file
    const QString &iconPath = app.desktopIcon();
    const QUrl &iconUrl = QUrl::fromLocalFile(iconPath);
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(iconUrl.scheme(), static_cast<QUrl>(iconUrl));
    if (factory) {
        QSharedPointer<DFMIO::DFile> dfile = factory->createFile();
        if (dfile && dfile->exists()) {
            icon = QIcon(app.desktopIcon());
            if (!icon.isNull()) {
                return icon;
            }
        }
    }

    // Next, try luck with application name
    QString name = app.desktopFileName().remove(".desktop").split("/").last();
    icon = QIcon::fromTheme(name);
    if (!icon.isNull()) {
        return icon;
    }

    // Last chance
    const QUrl &pixmapUrl = QUrl::fromLocalFile(QString(kSharePixmapPath));
    QSharedPointer<DFMIO::DEnumerator> enumerator = DecoratorFileEnumerator(pixmapUrl, {}, DFMIO::DEnumerator::DirFilter::kFiles | DFMIO::DEnumerator::DirFilter::kNoDotAndDotDot).enumeratorPtr();
    if (enumerator) {
        while (enumerator->hasNext()) {
            QSharedPointer<DFMIO::DFileInfo> fileinfo = enumerator->fileInfo();
            if (fileinfo) {
                const QString &fileName = fileinfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardName).toString();
                if (fileName.contains(name)) {
                    return QIcon(QString(kSharePixmapPath) + QDir::separator() + fileName);
                }
            }
        }
    }

    // Default icon
    return defaultIcon;
}

QIcon FileUtils::searchGenericIcon(const QString &category, const QIcon &defaultIcon)
{
    QIcon icon = QIcon::fromTheme(category + "-generic");
    if (!icon.isNull()) {
        return icon;
    }
    icon = QIcon::fromTheme(category + "-x-generic");
    return icon.isNull() ? defaultIcon : icon;
}

QIcon FileUtils::searchMimeIcon(QString mime, const QIcon &defaultIcon)
{
    QIcon icon = QIcon::fromTheme(mime.replace("/", "-"), defaultIcon);
    return icon;
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

bool FileUtils::compareString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    class StrCollator : public QCollator
    {
    public:
        explicit StrCollator(const QLocale &locale = QLocale())
            : QCollator(locale)
        {
            setNumericMode(true);
            setCaseSensitivity(Qt::CaseInsensitive);
        }
    };

    thread_local static StrCollator sortCollator;
    auto startWithSymbol = [](const QString &text) -> bool {
        if (text.isEmpty())
            return false;

        // Matches strings beginning with letters, numbers, and Chinese
        QRegExp regExp("^[a-zA-Z0-9\u4e00-\u9fa5].*$");
        return !regExp.exactMatch(text);
    };

    auto startWithHanzi = [](const QString &text) -> bool {
        if (text.isEmpty())
            return false;

        return text.at(0).script() == QChar::Script_Han;
    };

    // Other symbols need to be ranked last, and judgment needs to be made before Chinese
    if (startWithSymbol(str1)) {
        if (!startWithSymbol(str2))
            return order == Qt::DescendingOrder;
    } else if (startWithSymbol(str2))
        return order != Qt::DescendingOrder;

    if (startWithHanzi(str1)) {
        if (!startWithHanzi(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithHanzi(str2)) {
        return order != Qt::DescendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

QString FileUtils::dateTimeFormat()
{
    static QString format = "yyyy/MM/dd HH:mm:ss";
    return format;
}

bool FileUtils::setBackGround(const QString &pictureFilePath)
{
    QDBusMessage msgIntrospect = QDBusMessage::createMethodCall("com.deepin.daemon.Appearance",
                                                                "/com/deepin/daemon/Appearance",
                                                                "org.freedesktop.DBus.Introspectable",
                                                                "Introspect");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msgIntrospect);
    call.waitForFinished();
    if (call.isFinished()) {
        QDBusReply<QString> reply = call.reply();
        QString value = reply.value();

        if (value.contains("SetMonitorBackground")) {
            QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.daemon.Appearance",
                                                              "/com/deepin/daemon/Appearance",
                                                              "com.deepin.daemon.Appearance",
                                                              "SetMonitorBackground");
            if (WindowUtils::isWayLand()) {
                QDBusInterface interface("com.deepin.daemon.Display",
                                         "/com/deepin/daemon/Display",
                                         "com.deepin.daemon.Display");
                QString screenname = qvariant_cast<QString>(interface.property("Primary"));
                msg.setArguments({ screenname, pictureFilePath });
            } else {
                msg.setArguments({ qApp->primaryScreen()->name(), pictureFilePath });
            }

            QDBusConnection::sessionBus().asyncCall(msg);
            qDebug() << "FileUtils::setBackground call Appearance SetMonitorBackground";
            return true;
        }
    }

    QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.daemon.Appearance",
                                                      "/com/deepin/daemon/Appearance",
                                                      "com.deepin.daemon.Appearance",
                                                      "Set");
    msg.setArguments({ "Background", pictureFilePath });
    QDBusConnection::sessionBus().asyncCall(msg);
    qDebug() << "FileUtils::setBackground call Appearance Set";

    return true;
}

QString FileUtils::nonExistFileName(AbstractFileInfoPointer fromInfo, AbstractFileInfoPointer targetDir, std::function<bool(const QString &)> functionCheck)
{
    if (!targetDir || !DecoratorFile(targetDir->url()).exists()) {
        return QString();
    }

    if (!targetDir->isDir()) {
        return QString();
    }

    const QString &copyText = QCoreApplication::translate("DoCopyFilesWorker", "copy",
                                                          "Extra name added to new file name when used for file name.");

    AbstractFileInfoPointer targetFileInfo { nullptr };
    QString fileBaseName = fromInfo->completeBaseName();
    QString suffix = fromInfo->suffix();
    QString fileName = fromInfo->fileName();
    //在7z分卷压缩后的名称特殊处理7z.003
    const QString &reg = ".7z.[0-9]{3,10}$";
    if (fileName.contains(QRegularExpression(reg))) {
        const int &index = fileName.indexOf(QRegularExpression(reg));
        fileBaseName = fileName.left(index);
        suffix = fileName.mid(index + 1);
    }

    int number = 0;
    QString newFileName;
    QUrl newUrl;
    do {
        newFileName = number > 0 ? QString("%1(%2 %3)").arg(fileBaseName, copyText).arg(number) : QString("%1(%2)").arg(fileBaseName, copyText);

        if (!suffix.isEmpty()) {
            newFileName.append('.').append(suffix);
        }

        ++number;
        newUrl = targetDir->url();
        newUrl.setPath(newUrl.path() + "/" + newFileName);
        targetFileInfo = InfoFactory::create<AbstractFileInfo>(newUrl);
    } while ((targetFileInfo && DecoratorFile(newUrl).exists()) || (functionCheck ? functionCheck(newFileName) : false));

    return newFileName;
}

QString FileUtils::bindPathTransform(const QString &path, bool toDevice)
{
    if (!path.startsWith("/") || path == "/")
        return path;

    const QMap<QString, QString> &table = DeviceUtils::fstabBindInfo();
    if (table.isEmpty())
        return path;

    QString bindPath(path);
    if (toDevice) {
        for (const auto &mntPoint : table.values()) {
            if (path.startsWith(mntPoint)) {
                bindPath.replace(mntPoint, table.key(mntPoint));
                break;
            }
        }
    } else {
        for (const auto &device : table.keys()) {
            if (path.startsWith(device)) {
                bindPath.replace(device, table[device]);
                break;
            }
        }
    }

    return bindPath;
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
                qWarning() << re.errorString();
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
            qWarning() << re.errorString();
            continue;
        }

        if (re.match(name).hasMatch()) {
            return true;
        }
    }

    return false;
}

}
