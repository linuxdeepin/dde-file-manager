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
#include "dfm_global_defines.h"
#include "mimetype/mimedatabase.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/finallyutil.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/standardpaths.h"

#include <dfm-io/dfmio_utils.h>

#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QTextCodec>

#include <unistd.h>
#include <sys/stat.h>

DFMBASE_BEGIN_NAMESPACE

static constexpr char DDE_TRASH_ID[] { "dde-trash" };
static constexpr char DDE_COMPUTER_ID[] { "dde-computer" };
const static int kDefaultMemoryPageSize = 4096;

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

QMap<QString, QString> FileUtils::getKernelParameters()
{
    QFile cmdline("/proc/cmdline");
    cmdline.open(QIODevice::ReadOnly);
    QByteArray content = cmdline.readAll();

    QByteArrayList paraList(content.split(' '));

    QMap<QString, QString> result;
    result.insert("_ori_proc_cmdline", content);

    for (const QByteArray &onePara : paraList) {
        int equalsIdx = onePara.indexOf('=');
        QString key = equalsIdx == -1 ? onePara.trimmed() : onePara.left(equalsIdx).trimmed();
        QString value = equalsIdx == -1 ? QString() : onePara.right(equalsIdx).trimmed();
        result.insert(key, value);
    }

    return result;
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

    QRegExp reg(QString("/run/user/%1/.?gvfs/.+").arg(getuid()));

    return -1 != reg.indexIn(path);
}

QString FileUtils::preprocessingFileName(QString name)
{
    // eg: [\\:*\"?<>|\r\n]
    const QString &value = Application::genericObtuselySetting()->value("FileName", "non-allowableCharacters").toString();

    if (value.isEmpty())
        return name;

    return name.remove(QRegularExpression(value));
}

bool FileUtils::isDesktopFile(const QUrl &url)
{
    return url.toLocalFile().endsWith(".desktop");
}

bool FileUtils::isTrashDesktopFile(const QUrl &url)
{
    if (isDesktopFile(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == DDE_TRASH_ID;
    }
    return false;
}

bool FileUtils::isComputerDesktopFile(const QUrl &url)
{
    if (isDesktopFile(url)) {
        DesktopFile df(url.toLocalFile());
        return df.desktopDeepinId() == DDE_COMPUTER_ID;
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

bool FileUtils::isSmbPath(const QUrl &url)
{
    const QString &&strUrl = url.toString();

    // like file:///run/user/1000/gvfs/smb-share:domain=ttt,server=xx.xx.xx.xx,share=io,user=uos/path
    QRegExp reg("/run/user/.+gvfs/smb-share:.*server.+share.+");
    int idx = reg.indexIn(strUrl);

    if (-1 == idx) {
        // 传进来的可能是加密的路径
        idx = reg.indexIn(QUrl::fromPercentEncoding(strUrl.toLocal8Bit()));
    }

    if (-1 == idx) {
        // like smb://ttt;uos:1@xx.xx.xx.xx/io/path
        // maybe access by mapping addr, like smb://xxx.com/io
        reg.setPattern("smb://.+");
        idx = reg.indexIn(strUrl);
    }
    return -1 != idx;
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

        bool isDesktopApp = info->mimeTypeName().contains(Global::kMimetypeAppDesktop);

        ///###: symlink is also processed here.
        QString fileBaseName = isDesktopApp ? info->fileDisplayName() : info->baseName();
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        fileBaseName.replace(pair.first, pair.second);

        if (fileBaseName.trimmed().isEmpty()) {
            qWarning() << "replace fileBaseName(not include suffix) trimmed is empty string";
            continue;
        }

        int maxLength = Global::kMaxFileNameCharCount - suffix.toLocal8Bit().size();

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
        bool isDesktopApp = info->mimeTypeName().contains(Global::kMimetypeAppDesktop);

        QString fileBaseName = isDesktopApp ? info->fileDisplayName() : info->baseName();   //{ info->baseName() };
        QString oldFileName = fileBaseName;

        QString addText = pair.first;
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        int maxLength = Global::kMaxFileNameCharCount - info->fileName().toLocal8Bit().size();

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
        bool isDesktopApp = info->mimeTypeName().contains(Global::kMimetypeAppDesktop);

        QString fileBaseName { pair.first };
        const QString &indexString = QString::number(index);
        const QString &suffix = info->suffix().isEmpty() ? QString() : QString(".") + info->suffix();
        int maxLength = Global::kMaxFileNameCharCount - indexString.toLocal8Bit().size() - suffix.toLocal8Bit().size();

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

QString FileUtils::getSymlinkFileName(const QUrl &fileUrl, const QUrl &parentUrl)
{
    const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(fileUrl);

    if (info && info->exists()) {
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

DFMBASE_END_NAMESPACE
