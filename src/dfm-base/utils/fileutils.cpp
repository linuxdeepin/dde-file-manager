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

#include <dfm-io/dfmio_utils.h>

#include <QFileInfo>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QTextCodec>

#include <unistd.h>

DFMBASE_BEGIN_NAMESPACE

static constexpr char DDE_TRASH_ID[] { "dde-trash" };
static constexpr char DDE_COMPUTER_ID[] { "dde-computer" };

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
    static QMap<QString, int> datas {
        std::pair<QString, int>("VFAT", 11),
        std::pair<QString, int>("EXFAT", 11),
        std::pair<QString, int>("EXT2", 16),
        std::pair<QString, int>("EXT3", 16),
        std::pair<QString, int>("EXT4", 16),
        std::pair<QString, int>("NTFS", 40),   // can be more than 40
    };

    const int DefaultLength = 11;
    return datas.value(fileSystem.toUpper(), DefaultLength);
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
        return df.getDeepinId() == DDE_TRASH_ID;
    }
    return false;
}

bool FileUtils::isComputerDesktopFile(const QUrl &url)
{
    if (isDesktopFile(url)) {
        DesktopFile df(url.toLocalFile());
        return df.getDeepinId() == DDE_COMPUTER_ID;
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

QMap<QUrl, QUrl> FileUtils::fileBatchAddText(const QList<QUrl> &originUrls, const QPair<QString, dfmbase::AbstractJobHandler::FileBatchAddTextFlags> &pair)
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

        if (pair.second == AbstractJobHandler::FileBatchAddTextFlags::kPrefix) {
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

DFMBASE_END_NAMESPACE
