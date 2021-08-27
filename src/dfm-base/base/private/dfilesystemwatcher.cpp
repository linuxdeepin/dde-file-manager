/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "base/private/dfilesystemwatcher_p.h"
#include "base/private/dfilesystemwatcher.h"
#include "dfm-base/dfm_base_global.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QTextCodec>
#include <QMimeDatabase>
#include <KEncodingProber>
#include <QRegularExpression>

#if defined(Q_OS_LINUX)
#include <sys/inotify.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace GlobalPrivate {

float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country)
{
    qreal hep_count = 0;
    int non_base_latin_count = 0;
    qreal unidentification_count = 0;
    int replacement_count = 0;

    QTextDecoder decoder(codec);
    const QString &unicode_data = decoder.toUnicode(data);

    for (int i = 0; i < unicode_data.size(); ++i) {
        const QChar &ch = unicode_data.at(i);

        if (ch.unicode() > 0x7f)
            ++non_base_latin_count;

        switch (ch.script()) {
        case QChar::Script_Hiragana:
        case QChar::Script_Katakana:
            hep_count += country == QLocale::Japan ? 1.2 : 0.5;
            unidentification_count += country == QLocale::Japan ? 0 : 0.3;
            break;
        case QChar::Script_Han:
            hep_count += country == QLocale::China ? 1.2 : 0.5;
            unidentification_count += country == QLocale::China ? 0 : 0.3;
            break;
        case QChar::Script_Hangul:
            hep_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 0 : 0.3;
            break;
        case QChar::Script_Cyrillic:
            hep_count += (country == QLocale::Russia) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Russia) ? 0 : 0.3;
            break;
        case QChar::Script_Greek:
            hep_count += (country == QLocale::Greece) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Greece) ? 0 : 0.3;
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
                ++hep_count;
            } else if (ch.isSurrogate() && ch.isHighSurrogate()) {
                ++i;

                if (i < unicode_data.size()) {
                    const QChar &next_ch = unicode_data.at(i);

                    if (!next_ch.isLowSurrogate()) {
                        --i;
                        break;
                    }

                    uint unicode = QChar::surrogateToUcs4(ch, next_ch);

                    // emoji
                    if (unicode >= 0x1f000 && unicode <= 0x1f6ff) {
                        hep_count += 2;
                    }
                }
            } else if (ch.unicode() == QChar::ReplacementCharacter) {
                ++replacement_count;
            } else if (ch.unicode() > 0x7f) {
                // 因为UTF-8编码的容错性很低，所以未识别的编码只需要判断是否为 QChar::ReplacementCharacter 就能排除
                if (codec->name() != "UTF-8")
                    ++unidentification_count;
            }
            break;
        }
    }

    // blumia: not sure why original author assume non_base_latin_count must greater than zero...
    if (non_base_latin_count == 0) {
        return 1.0f;
    }

    float c = static_cast<float>(qreal(hep_count) / non_base_latin_count / 1.2);

    c -= static_cast<float>(qreal(replacement_count) / non_base_latin_count);
    c -= static_cast<float>(qreal(unidentification_count) / non_base_latin_count);

    return qMax(0.0f, c);
}

QByteArray detectCharset(const QByteArray &data, const QString &fileName)
{
    // Return local encoding if nothing in file.
    if (data.isEmpty()) {
        return QTextCodec::codecForLocale()->name();
    }

    if (QTextCodec *c = QTextCodec::codecForUtfText(data, nullptr)) {
        return c->name();
    }

    QMimeDatabase mime_database;
    const QMimeType &mime_type = fileName.isEmpty() ? mime_database.mimeTypeForData(data) : mime_database.mimeTypeForFileNameAndData(fileName, data);
    const QString &mimetype_name = mime_type.name();
    KEncodingProber::ProberType proberType = KEncodingProber::Universal;

    if (mimetype_name == QStringLiteral("application/xml")
            || mimetype_name == QStringLiteral("text/html")
            || mimetype_name == QStringLiteral("application/xhtml+xml")) {
        const QString &_data = QString::fromLatin1(data);
        QRegularExpression pattern("<\\bmeta.+\\bcharset=(?'charset'\\S+?)\\s*['\"/>]");

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        const QString &charset = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                               QRegularExpression::DontCheckSubjectStringMatchOption).captured("charset");

        if (!charset.isEmpty()) {
            return charset.toLatin1();
        }

        pattern.setPattern("<\\bmeta\\s+http-equiv=\"Content-Language\"\\s+content=\"(?'language'[a-zA-Z-]+)\"");

        const QString &language = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                                QRegularExpression::DontCheckSubjectStringMatchOption).captured("language");

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
    } else if (mimetype_name == "text/x-python") {
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
    const QList<QPair<KEncodingProber::ProberType, QLocale::Country>> fallback_list {
        {KEncodingProber::ChineseSimplified, QLocale::China},
        {KEncodingProber::ChineseTraditional, QLocale::China},
        {KEncodingProber::Japanese, QLocale::Japan},
        {KEncodingProber::Korean, QLocale::NorthKorea},
        {KEncodingProber::Cyrillic, QLocale::Russia},
        {KEncodingProber::Greek, QLocale::Greece},
        {proberType, QLocale::system().country()}
    };

    KEncodingProber prober(proberType);
    prober.feed(data);
    float pre_confidence = prober.confidence();
    QByteArray pre_encoding = prober.encoding();

    QTextCodec *def_codec = QTextCodec::codecForLocale();
    QByteArray encoding;
    float confidence = 0;

    for (auto i : fallback_list) {
        prober.setProberType(i.first);
        prober.feed(data);

        float prober_confidence = prober.confidence();
        QByteArray prober_encoding = prober.encoding();

        if (i.first != proberType && qFuzzyIsNull(prober_confidence)) {
            prober_confidence = pre_confidence;
            prober_encoding = pre_encoding;
        }

    confidence:
        if (QTextCodec *codec = QTextCodec::codecForName(prober_encoding)) {
            if (def_codec == codec)
                def_codec = nullptr;

            float c = codecConfidenceForData(codec, data, i.second);

            if (prober_confidence > 0.5) {
                c = c / 2 + prober_confidence / 2;
            } else {
                c = c / 3 * 2 + prober_confidence / 3;
            }

            if (c > confidence) {
                confidence = c;
                encoding = prober_encoding;
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
            // 使用 proberType 类型探测出的结果结合此国家再次做编码检查
            i.first = proberType;
            prober_confidence = pre_confidence;
            prober_encoding = pre_encoding;
            goto confidence;
        }
    }

    if (def_codec && codecConfidenceForData(def_codec, data, QLocale::system().country()) > confidence) {
        return def_codec->name();
    }

    return encoding;
}

QString toUnicode(const QByteArray &data, const QString &fileName = "")
{
    if (data.isEmpty())
        return QString();

    const QByteArray &encoding = detectCharset(data, fileName);

    if (QTextCodec *codec = QTextCodec::codecForName(encoding)) {
        return codec->toUnicode(data);
    }

    return QString::fromLocal8Bit(data);
}

bool fileNameCorrection(const QString &filePath)
{
    QFileInfo info(filePath);
    QProcess ls;

    ls.start("ls", QStringList() << "-1" << "--color=never" << info.absolutePath());
    ls.waitForFinished();

    const QByteArray &request = ls.readAllStandardOutput();

    for (const QByteArray &name : request.split('\n')) {
        QString strFileName{ GlobalPrivate::toUnicode(name) };

        if (strFileName == info.fileName() && strFileName.toLocal8Bit() != name) {
            const QByteArray &path = info.absolutePath().toLocal8Bit() + QDir::separator().toLatin1() + name;

            return fileNameCorrection(path);
        }
    }

    return false;
}

} //nemspace GlobalPrivate

DFileSystemWatcherPrivate::DFileSystemWatcherPrivate(int fd, DFileSystemWatcher *qq)
    : q_ptr(qq)
    , inotifyFd(fd)
    , notifier(fd, QSocketNotifier::Read, qq)
{
    fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
    qq->connect(&notifier, SIGNAL(activated(int)), q_ptr, SLOT(_q_readFromInotify()));
}

DFileSystemWatcherPrivate::~DFileSystemWatcherPrivate()
{
    notifier.setEnabled(false);
    Q_FOREACH (int id, pathToID)
        inotify_rm_watch(inotifyFd, id < 0 ? -id : id);

    ::close(inotifyFd);
}

QStringList DFileSystemWatcherPrivate::addPaths(const QStringList &paths, QStringList *files, QStringList *directories)
{
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);
        bool isDir = fi.isDir();
        if (isDir) {
            if (directories->contains(path))
                continue;
        } else {
            if (files->contains(path))
                continue;
        }

        int wd = inotify_add_watch(inotifyFd,
                                   QFile::encodeName(path),
                                   (isDir
                                    ? (0
                                       | IN_ATTRIB
                                       | IN_MOVE
                                       | IN_MOVE_SELF
                                       | IN_CREATE
                                       | IN_DELETE
                                       | IN_DELETE_SELF
                                       | IN_MODIFY
                                       | IN_UNMOUNT
                                       )
                                    : (0
                                       | IN_ATTRIB
                                       | IN_CLOSE_WRITE
                                       | IN_MODIFY
                                       | IN_MOVE
                                       | IN_MOVE_SELF
                                       | IN_DELETE_SELF
                                       )));
        if (wd < 0) {
            perror("DFileSystemWatcherPrivate::addPaths: inotify_add_watch failed");
            continue;
        }

        it.remove();

        int id = isDir ? -wd : wd;
        if (id < 0) {
            directories->append(path);
        } else {
            files->append(path);
        }

        pathToID.insert(path, id);
        idToPath.insert(id, path);
    }

    return p;
}

QStringList DFileSystemWatcherPrivate::removePaths(const QStringList &paths, QStringList *files, QStringList *directories)
{
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        int id = pathToID.take(path);
        for (auto hit = idToPath.find(id); hit != idToPath.end() && hit.key() == id; ++hit) {
            if (hit.value() == path) {
                idToPath.erase(hit);
                break;
            }
        }

        it.remove();

        if (!idToPath.contains(id)) {
            int wd = id < 0 ? -id : id;
            //qDebug() << "removing watch for path" << path << "wd" << wd;
            inotify_rm_watch(inotifyFd, wd);
        }

        if (id < 0) {
            directories->removeAll(path);
        } else {
            files->removeAll(path);
        }
    }

    return p;
}

void DFileSystemWatcherPrivate::_q_readFromInotify()
{
    Q_Q(DFileSystemWatcher);
//    qDebug() << "QInotifyFileSystemWatcherEngine::readFromInotify";

    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
    // fix task#36123 【自测】【桌面专业版】【SP2】【wayland】【文件管理器】 文件管理器概率出现卡死
    if (buffSize == 0) {
        return;
    }
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = static_cast<int>(read(inotifyFd, buffer.data(), static_cast<size_t>(buffSize)));
    char *at = buffer.data();
    char * const end = at + buffSize;

    QList<inotify_event *> eventList;
    QMultiHash<int, QString> batch_pathmap;
    /// only save event: IN_MOVE_TO
    QMultiMap<int, QString> cookieToFilePath;
    QMultiMap<int, QString> cookieToFileName;
    QSet<int> hasMoveFromByCookie;
#ifdef QT_DEBUG
    int exist_count = 0;
#endif
    while (at < end) {
        inotify_event *event = reinterpret_cast<inotify_event *>(at);
        QStringList paths;

        at += sizeof(inotify_event) + event->len;

        int id = event->wd;
        paths = idToPath.values(id);
        if (paths.empty()) {
            // perhaps a directory?
            id = -id;
            paths = idToPath.values(id);
            if (paths.empty())
                continue;
        }

        if (!(event->mask & IN_MOVED_TO) || !hasMoveFromByCookie.contains(event->cookie)) {
            //fix bug57628归档管理器压缩成功后，桌面不显示
            //归档管理器使用命令调用7z完成压缩，压缩多个文件不在相同路径时会采用追加的方式，每次追加都会创建zip.tmp文件，追加后删除zip，再移动zip.tmp为zip文件。
            //多次追加时会创建多次zip.tmp文件，如果丢弃后面的创建事件，会导致移动时找不到原文件。
            /*!
             * 经查以下代码提交记录，以及与作者沟通确认，添加去除重复事件的逻辑，是为了处理当时的wps表格保存含有大量数据行时，会收到多次重复的保存事件，导致wps表格卡死。
             * 在现有的版本（wps表格：11.1.0.9719）测试问题已不存在，谨慎放开。
            */
#ifdef QT_DEBUG
            auto it = std::find_if(eventList.begin(), eventList.end(), [event](inotify_event *e){
                    return event->wd == e->wd && event->mask == e->mask &&
                           event->cookie == e->cookie &&
                           event->len == e->len &&
                           !strcmp(event->name, e->name);
            });

            if (it != eventList.end()) {
                qDebug() << "exist event and event->wd:" << event->wd << "event->mask:" << event->mask
                         << "event->cookie:" << event->cookie << "event->len:" << event->len
                         << "event->name:" << event->name << "exist counts " << ++exist_count;
            }
#endif
            eventList.append(event);

            const QList<QString> bps = batch_pathmap.values(id);
            for (auto &path : paths) {
                if (!bps.contains(path)) {
                    batch_pathmap.insert(id, path);
                }
            }
        }

        if (event->mask & IN_MOVED_TO) {
            for (auto &path : paths) {
                cookieToFilePath.insert(event->cookie, path);
            }
            cookieToFileName.insert(event->cookie, QString::fromUtf8(event->name));
        }

        if (event->mask & IN_MOVED_FROM)
            hasMoveFromByCookie << event->cookie;
    }

//    qDebug() << "event count:" << eventList.count();

    QList<inotify_event *>::const_iterator it = eventList.constBegin();
    while (it != eventList.constEnd()) {
        const inotify_event &event = **it;
        ++it;

//        qDebug() << "inotify event, wd" << event.wd << "cookie" << event.cookie << "mask" << hex << event.mask;

        int id = event.wd;
        QStringList paths = batch_pathmap.values(id);

        if (paths.empty()) {
            id = -id;
            paths = batch_pathmap.values(id);

            if (paths.isEmpty())
                continue;
        }
        const QString &name = QString::fromUtf8(event.name, static_cast<int>(strlen(event.name)));

        for (auto &path : paths) {
//            qDebug() << "event for path" << path;

            /// TODO: Existence of invalid utf8 characters QFile can not read the file information
            if (event.name != QString::fromLocal8Bit(event.name, static_cast<int>(strlen(event.name))).toLocal8Bit()) {
                if (event.mask & (IN_CREATE | IN_MOVED_TO)) {
                    GlobalPrivate::fileNameCorrection(path);
                }
            }

            if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
                do {
                    if (event.mask & IN_MOVE_SELF) {
                        QMap<int, QString>::const_iterator iterator = cookieToFilePath.constBegin();

                        bool isMove = false;

                        while (iterator != cookieToFilePath.constEnd()) {
                            const QString &_path = iterator.value();
                            const QString &_name = cookieToFileName.value(iterator.key());

                            if (QFileInfo(_path + QDir::separator() + _name) == QFileInfo(path)) {
                                isMove = true;
                                break;
                            }

                            ++iterator;
                        }

                        if (isMove)
                            break;
                    } else if(event.mask & IN_UNMOUNT){//! 文件系统没卸载进行信号发送处理
                        Q_EMIT q->fileSystemUMount(path, QString(), DFileSystemWatcher::QPrivateSignal());
                    }

                    /// Keep watcher
//                    pathToID.remove(path);
//                    idToPath.remove(id, getPathFromID(id));
//                    if (!idToPath.contains(id))
//                        inotify_rm_watch(inotifyFd, event.wd);
//
//                    if (id < 0)
//                        onDirectoryChanged(path, true);
//                    else
//                        onFileChanged(path, true);

                    Q_EMIT q->fileDeleted(path, QString(), DFileSystemWatcher::QPrivateSignal());
                } while (false);
            } else {
                if (id < 0)
                    onDirectoryChanged(path, false);
                else
                    onFileChanged(path, false);
            }

            QString filePath = path;

            if (id < 0) {
                if (path.endsWith(QDir::separator()))
                    filePath = path + name;
                else
                    filePath = path  + QDir::separator() + name;
            }

            if (event.mask & IN_CREATE) {
//                qDebug() << "IN_CREATE" << filePath << name;

                if (name.isEmpty()) {
                    if (pathToID.contains(path)) {
                        q->removePath(path);
                        q->addPath(path);
                    }
                } else if (pathToID.contains(filePath)) {
                    q->removePath(filePath);
                    q->addPath(filePath);
                }

                Q_EMIT q->fileCreated(path, name, DFileSystemWatcher::QPrivateSignal());
            }

            if (event.mask & IN_DELETE) {
//                qDebug() << "IN_DELETE" << filePath;

                Q_EMIT q->fileDeleted(path, name, DFileSystemWatcher::QPrivateSignal());
            }

            if (event.mask & IN_MOVED_FROM) {
                const QString toName = cookieToFileName.value(event.cookie);

                if (cookieToFilePath.values(event.cookie).empty()) {
                    Q_EMIT q->fileMoved(path, name, "", "", DFileSystemWatcher::QPrivateSignal());
                }
                else {
                    for (QString &toPath : cookieToFilePath.values(event.cookie)) {
//                        qDebug() << "IN_MOVED_FROM" << filePath << "to path:" << toPath << "to name:" << toName;

                        Q_EMIT q->fileMoved(path, name, toPath, toName, DFileSystemWatcher::QPrivateSignal());
                    }
                }
            }

            if (event.mask & IN_MOVED_TO) {
//                qDebug() << "IN_MOVED_TO" << filePath << name;

                if (!hasMoveFromByCookie.contains(event.cookie))
                Q_EMIT q->fileMoved(QString(), QString(), path, name, DFileSystemWatcher::QPrivateSignal());
            }

            if (event.mask & IN_ATTRIB) {
//                qDebug() << "IN_ATTRIB" <<  event.mask << filePath;

                Q_EMIT q->fileAttributeChanged(path, name, DFileSystemWatcher::QPrivateSignal());
            }

            /*only monitor file close event which is opend by write mode*/
            if (event.mask & IN_CLOSE_WRITE) {
//                qDebug() << "IN_CLOSE_WRITE" <<  event.mask << filePath;

                Q_EMIT q->fileClosed(path, id < 0 ? name : QString(), DFileSystemWatcher::QPrivateSignal());
            }

            if (event.mask & IN_MODIFY) {
//                qDebug() << "IN_MODIFY" <<  event.mask << filePath << name;

                Q_EMIT q->fileModified(path, name, DFileSystemWatcher::QPrivateSignal());
            }
        }
    }
}

void DFileSystemWatcherPrivate::onFileChanged(const QString &path, bool removed)
{
    if (!files.contains(path)) {
        // the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed) {
        files.removeAll(path);
    }
//    emit q->fileChanged(path, DFileSystemWatcher::QPrivateSignal());
}

void DFileSystemWatcherPrivate::onDirectoryChanged(const QString &path, bool removed)
{
    if (!directories.contains(path)) {
        // perhaps the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed) {
        directories.removeAll(path);
    }
//    emit q->directoryChanged(path, DFileSystemWatcher::QPrivateSignal());
}

/*!
    \class DFileSystemWatcher
    \inmodule QtCore
    \brief The DFileSystemWatcher class provides an interface for monitoring files and directories for modifications.
    \ingroup io
    \since 4.2
    \reentrant

    DFileSystemWatcher monitors the file system for changes to files
    and directories by watching a list of specified paths.

    Call addPath() to watch a particular file or directory. Multiple
    paths can be added using the addPaths() function. Existing paths can
    be removed by using the removePath() and removePaths() functions.

    DFileSystemWatcher examines each path added to it. Files that have
    been added to the DFileSystemWatcher can be accessed using the
    files() function, and directories using the directories() function.

    The fileChanged() signal is emitted when a file has been modified,
    renamed or removed from disk. Similarly, the directoryChanged()
    signal is emitted when a directory or its contents is modified or
    removed.  Note that DFileSystemWatcher stops monitoring files once
    they have been renamed or removed from disk, and directories once
    they have been removed from disk.

    \note On systems running a Linux kernel without inotify support,
    file systems that contain watched paths cannot be unmounted.

    \note Windows CE does not support directory monitoring by
    default as this depends on the file system driver installed.

    \note The act of monitoring files and directories for
    modifications consumes system resources. This implies there is a
    limit to the number of files and directories your process can
    monitor simultaneously. On all BSD variants, for
    example, an open file descriptor is required for each monitored
    file. Some system limits the number of open file descriptors to 256
    by default. This means that addPath() and addPaths() will fail if
    your process tries to add more than 256 files or directories to
    the file system monitor. Also note that your process may have
    other file descriptors open in addition to the ones for files
    being monitored, and these other open descriptors also count in
    the total. OS X uses a different backend and does not
    suffer from this issue.


    \sa QFile, QDir
*/


/*!
    Constructs a new file system watcher object with the given \a parent.
*/
DFileSystemWatcher::DFileSystemWatcher(QObject *parent)
    : QObject(parent)
{
    int fd = -1;
#ifdef IN_CLOEXEC
    fd = inotify_init1(IN_CLOEXEC);
#endif
    if (fd == -1) {
        fd = inotify_init();
    }

    if (fd != -1)
        d_ptr.reset(new DFileSystemWatcherPrivate(fd, this));
}

/*!
    Constructs a new file system watcher object with the given \a parent
    which monitors the specified \a paths list.
*/
DFileSystemWatcher::DFileSystemWatcher(const QStringList &paths, QObject *parent)
    : DFileSystemWatcher(parent)
{
    addPaths(paths);
}

/*!
    Destroys the file system watcher.
*/
DFileSystemWatcher::~DFileSystemWatcher()
{ }

/*!
    Adds \a path to the file system watcher if \a path exists. The
    path is not added if it does not exist, or if it is already being
    monitored by the file system watcher.

    If \a path specifies a directory, the directoryChanged() signal
    will be emitted when \a path is modified or removed from disk;
    otherwise the fileChanged() signal is emitted when \a path is
    modified, renamed or removed.

    If the watch was successful, true is returned.

    Reasons for a watch failure are generally system-dependent, but
    may include the resource not existing, access failures, or the
    total watch count limit, if the platform has one.

    \note There may be a system dependent limit to the number of
    files and directories that can be monitored simultaneously.
    If this limit is been reached, \a path will not be monitored,
    and false is returned.

    \sa addPaths(), removePath()
*/
bool DFileSystemWatcher::addPath(const QString &file)
{
    if (file.isEmpty()) {
        //qWarning("DFileSystemWatcher::addPath: path is empty");
        return true;
    }

    QStringList paths = addPaths(QStringList(file));
    return paths.isEmpty();
}

/*!
    Adds each path in \a paths to the file system watcher. Paths are
    not added if they not exist, or if they are already being
    monitored by the file system watcher.

    If a path specifies a directory, the directoryChanged() signal
    will be emitted when the path is modified or removed from disk;
    otherwise the fileChanged() signal is emitted when the path is
    modified, renamed, or removed.

    The return value is a list of paths that could not be watched.

    Reasons for a watch failure are generally system-dependent, but
    may include the resource not existing, access failures, or the
    total watch count limit, if the platform has one.

    \note There may be a system dependent limit to the number of
    files and directories that can be monitored simultaneously.
    If this limit has been reached, the excess \a paths will not
    be monitored, and they will be added to the returned QStringList.

    \sa addPath(), removePaths()
*/
QStringList DFileSystemWatcher::addPaths(const QStringList &paths)
{
    Q_D(DFileSystemWatcher);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        //qWarning("DFileSystemWatcher::addPaths: list is empty");
        return QStringList();
    }

    if (d)
        p = d->addPaths(p, &d->files, &d->directories);

    return p;
}

/*!
    Removes the specified \a path from the file system watcher.

    If the watch is successfully removed, true is returned.

    Reasons for watch removal failing are generally system-dependent,
    but may be due to the path having already been deleted, for example.

    \sa removePaths(), addPath()
*/
bool DFileSystemWatcher::removePath(const QString &path)
{
    if (path.isEmpty()) {
      //  qWarning("DFileSystemWatcher::removePath: path is empty");
        return true;
    }

    QStringList paths = removePaths(QStringList(path));
    return paths.isEmpty();
}

/*!
    Removes the specified \a paths from the file system watcher.

    The return value is a list of paths which were not able to be
    unwatched successfully.

    Reasons for watch removal failing are generally system-dependent,
    but may be due to the path having already been deleted, for example.

    \sa removePath(), addPaths()
*/
QStringList DFileSystemWatcher::removePaths(const QStringList &paths)
{
    Q_D(DFileSystemWatcher);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        //qWarning("DFileSystemWatcher::removePaths: list is empty");
        return QStringList();
    }

    if (d)
        p = d->removePaths(p, &d->files, &d->directories);

    return p;
}

/*!
    \fn void DFileSystemWatcher::fileChanged(const QString &path)

    This signal is emitted when the file at the specified \a path is
    modified, renamed or removed from disk.

    \sa directoryChanged()
*/

/*!
    \fn void DFileSystemWatcher::directoryChanged(const QString &path)

    This signal is emitted when the directory at a specified \a path
    is modified (e.g., when a file is added or deleted) or removed
    from disk. Note that if there are several changes during a short
    period of time, some of the changes might not emit this signal.
    However, the last change in the sequence of changes will always
    generate this signal.

    \sa fileChanged()
*/

/*!
    \fn QStringList DFileSystemWatcher::directories() const

    Returns a list of paths to directories that are being watched.

    \sa files()
*/

/*!
    \fn QStringList DFileSystemWatcher::files() const

    Returns a list of paths to files that are being watched.

    \sa directories()
*/

QStringList DFileSystemWatcher::directories() const
{
    Q_D(const DFileSystemWatcher);

    if (!d)
        return QStringList();

    return d->directories;
}

void DFileSystemWatcher::_q_readFromInotify()
{
    Q_D(DFileSystemWatcher);

    d->_q_readFromInotify();
}

QStringList DFileSystemWatcher::files() const
{
    Q_D(const DFileSystemWatcher);

    if (!d)
        return QStringList();

    return d->files;
}

//#include "moc_dfilesystemwatcher.cpp"
