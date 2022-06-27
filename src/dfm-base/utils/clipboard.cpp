/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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
#include "clipboard.h"
#include "base/schemefactory.h"
#include "base/urlroute.h"

#include "dfm-base/dfm_global_defines.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMutex>
#include <QDebug>
#include <QUrl>

#include <sys/stat.h>
#include <unistd.h>
#include <X11/Xlib.h>

using namespace dfmbase;
ClipBoard *ClipBoard::self { nullptr };

namespace GlobalData {
static QList<QUrl> clipboardFileUrls;
static QMutex clipboardFileUrlsMutex;
static QList<quint64> clipbordFileinode;
static QAtomicInt remoteCurrentCount = 0;
static ClipBoard::ClipboardAction clipboardAction = ClipBoard::kUnknownAction;

static constexpr char kUserIdKey[] = "userId";
static constexpr char kRemoteCopyKey[] = "uos/remote-copy";
static constexpr char kGnomeCopyKey[] = "x-special/gnome-copied-files";

void onClipboardDataChanged()
{
    {
        QMutexLocker lk(&clipboardFileUrlsMutex);
        clipboardFileUrls.clear();
    }
    clipbordFileinode.clear();
    const QMimeData *mimeData = qApp->clipboard()->mimeData();
    if (!mimeData || mimeData->formats().isEmpty()) {
        qWarning() << "get null mimeData from QClipBoard or remote formats is null!";
        return;
    }
    if (mimeData->hasFormat(kRemoteCopyKey)) {
        qInfo() << "clipboard use other !";
        clipboardAction = ClipBoard::kRemoteAction;
        remoteCurrentCount++;
        return;
    }
    const QByteArray &data = mimeData->data(kGnomeCopyKey);

    if (data.startsWith("cut")) {
        clipboardAction = ClipBoard::kCutAction;
    } else if (data.startsWith("copy")) {
        clipboardAction = ClipBoard::kCopyAction;
    } else {
        clipboardAction = ClipBoard::kUnknownAction;
    }
    QString errorStr;
    for (QUrl &url : mimeData->urls()) {
        if (url.scheme().isEmpty())
            url.setScheme(Global::Scheme::kFile);

        {
            QMutexLocker lk(&clipboardFileUrlsMutex);
            clipboardFileUrls << url;
        }
        //链接文件的inode不加入clipbordFileinode，只用url判断clip，避免多个同源链接文件的逻辑误判
        const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url, true, &errorStr);

        if (!info) {
            qWarning() << QString("create file info error, case : %1").arg(errorStr);
            continue;
        }
        if (info->isSymLink())
            continue;

        struct stat statInfo;
        int fileStat = stat(url.path().toStdString().c_str(), &statInfo);
        if (0 == fileStat)
            clipbordFileinode << statInfo.st_ino;
    }
}
}

ClipBoard::ClipBoard(QObject *parent)
    : QObject(parent)
{
    connect(qApp->clipboard(), &QClipboard::dataChanged, this, &ClipBoard::onClipboardDataChanged);
    GlobalData::onClipboardDataChanged();
}

ClipBoard *ClipBoard::instance()
{
    if (!self)
        self = new ClipBoard;
    return self;
}
/*!
 * \brief ClipBoard::setUrlsToClipboard Set URLs to clipboard
 * \param list URL list of files
 * \param action Action of operation
 * \param mimeData Data written to the clipboard
 */
void ClipBoard::setUrlsToClipboard(const QList<QUrl> &list, ClipBoard::ClipboardAction action, QMimeData *mimeData)
{
    if (action == kUnknownAction)
        return;

    if (!mimeData)
        mimeData = new QMimeData;

    QByteArray ba = (action == ClipBoard::kCutAction) ? "cut" : "copy";
    QString text;
    QByteArray iconBa;
    QDataStream stream(&iconBa, QIODevice::WriteOnly);

    int maxIconsNum = 3;
    QString error;
    for (const QUrl &qurl : list) {
        ba.append("\n");
        ba.append(qurl.toString());

        const QString &path = qurl.toLocalFile();

        const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(qurl, true, &error);

        if (!info) {
            qWarning() << QString("create file info error, case : %1").arg(error);
            continue;
        }
        if (maxIconsNum-- > 0) {
            QStringList iconList;
            if (info->isSymLink()) {
                iconList << "emblem-symbolic-link";
            }
            if (!info->isWritable()) {
                iconList << "emblem-readonly";
            }
            if (!info->isReadable()) {
                iconList << "emblem-unreadable";
            }
            // TODO lanxs::目前缩略图还没有处理，等待处理完成了在修改
            // 多文件时只显示文件图标, 一个文件时显示缩略图(如果有的话)
            //            DFileInfo *fi = dynamic_cast<DFileInfo *>(info.data());
            //            QIcon icon = fi ? DFileIconProvider::globalProvider()->icon(*fi) :
            //                         DFileIconProvider::globalProvider()->icon(info->toQFileInfo());
            //            DAbstractFileInfo::FileType fileType = mimeTypeDisplayManager->displayNameToEnum(info->mimeTypeName());
            //            if (list.size() == 1 && fileType == DAbstractFileInfo::FileType::Images) {
            //                QIcon thumb(DThumbnailProvider::instance()->thumbnailFilePath(info->toQFileInfo(), DThumbnailProvider::Large));
            //                if (thumb.isNull()) {
            //                    //qWarning() << "thumbnail file faild " << fileInfo->absoluteFilePath();
            //                } else {
            //                    icon = thumb;
            //                }
            //            }
            //            stream << iconList << icon;
        }

        if (!path.isEmpty()) {
            text += path + '\n';
        }
    }

    mimeData->setText(text.endsWith('\n') ? text.left(text.length() - 1) : text);
    mimeData->setData("x-special/gnome-copied-files", ba);
    mimeData->setData("x-dfm-copied/file-icons", iconBa);
    mimeData->setUrls(list);
    // fix bug 63441
    // 如果是剪切操作，则禁止跨用户的粘贴操作
    if (ClipBoard::kCutAction == action) {
        QByteArray userId;
        userId.append(QString::number(getuid()));
        mimeData->setData(GlobalData::kUserIdKey, userId);
    }

    qApp->clipboard()->setMimeData(mimeData);
}
/*!
 * \brief ClipBoard::setDataToClopboard Set user data to clipboard
 * \param mimeData
 */
void ClipBoard::setDataToClipboard(QMimeData *mimeData)
{
    if (!mimeData) {
        qWarning() << "set data to clipboard failed, mimeData is null!";
        return;
    }

    qApp->clipboard()->setMimeData(mimeData);
}

/*!
 * \brief ClipBoard::supportCut support cut, Prohibit boasting of user cut operations
 * \return bool
 */
bool ClipBoard::supportCut()
{
    Q_ASSERT(qApp);

    QByteArray userId = qApp->clipboard()->mimeData()->data(GlobalData::kUserIdKey);
    return !userId.isEmpty() && (userId.toInt() == static_cast<int>(getuid()));
}

/*!
 * \brief ClipBoard::clearClipboard  Clean the shear plate
 */
void ClipBoard::clearClipboard()
{
    qApp->clipboard()->setText(QString());
}
/*!
 * \brief ClipBoard::getRemoteUrls Get URLs written to the clipboard
 * after remote download
 * \return URL list of files
 */
QList<QUrl> ClipBoard::getRemoteUrls()
{
    return getUrlsByX11();
}
/*!
 * \brief ClipBoard::clipboardFileUrlList Get URLs in the clipboard
 * \return
 */
QList<QUrl> ClipBoard::clipboardFileUrlList() const
{
    return GlobalData::clipboardFileUrls;
}
/*!
 * \brief ClipBoard::clipboardFileInodeList Gets the inode of URLs in the clipboard
 * \return
 */
QList<quint64> ClipBoard::clipboardFileInodeList() const
{
    return GlobalData::clipbordFileinode;
}
/*!
 * \brief ClipBoard::clipboardAction Gets the current operation of the clipboard
 * \return
 */
ClipBoard::ClipboardAction ClipBoard::clipboardAction() const
{
    return GlobalData::clipboardAction;
}
/*!
 * \brief ClipBoard::getUrlsByX11 Use X11 to read URLs downloaded
 * remotely from the clipboard
 * \return URL list of files
 */
QList<QUrl> ClipBoard::getUrlsByX11()
{
    QAtomicInt currentCount = GlobalData::remoteCurrentCount;
    const QMimeData *mimedata = qApp->clipboard()->mimeData();
    if (!mimedata) {
        qWarning() << "the clipboard mimedata is invalid!";
        return QList<QUrl>();
    }
    if (GlobalData::clipboardAction != kRemoteAction) {
        qWarning() << "current action is not RemoteAction ,error action " << GlobalData::clipboardAction;
        return QList<QUrl>();
    }
    //使用x11创建一个窗口去阻塞获取URl
    Display *display = XOpenDisplay(nullptr);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, color, color);

    char *result = nullptr;
    unsigned long ressize = 0, restail = 0;
    int resbits;
    Atom bufid = XInternAtom(display, "CLIPBOARD", False),
         fmtid = XInternAtom(display, "text/uri-list", False),
         propid = XInternAtom(display, "XSEL_DATA", False),
         incrid = XInternAtom(display, "INCR", False);
    XEvent event;

    QList<QUrl> urls;
    QString results;
    QAtomicInteger<bool> isCanceled = false;

    XSelectInput(display, window, PropertyChangeMask);
    XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
    QList<QUrl> currentClipboardFileUrls;
    do {
        XNextEvent(display, &event);
    } while (event.type != SelectionNotify || event.xselection.selection != bufid);
    if (event.xselection.property) {
        XGetWindowProperty(display, window, propid, 0, LONG_MAX / 4, True, AnyPropertyType,
                           &fmtid, &resbits, &ressize, &restail, (unsigned char **)&result);
        if (fmtid != incrid) {
            qInfo() << QString(result);
            urls += QUrl::fromStringList(QString(result).split("\n"));
        }
        XFree(result);

        if (fmtid == incrid) {
            do {
                do {
                    XNextEvent(display, &event);
                    if (event.type == SelectionNotify) {
                        isCanceled = true;
                        break;
                    }
                } while (event.type != PropertyNotify || event.xproperty.atom != propid
                         || event.xproperty.state != PropertyNewValue);

                if (isCanceled)
                    break;

                XGetWindowProperty(display, window, propid, 0, 0, True, AnyPropertyType, &fmtid, &resbits,
                                   &ressize, &restail, (unsigned char **)&result);
                XFree(result);
                XGetWindowProperty(display, window, propid, 0, static_cast<long>(restail), True, AnyPropertyType, &fmtid, &resbits,
                                   &ressize, &restail, (unsigned char **)&result);

                if (QString(result) != "/")
                    results += QString(result);

                XFree(result);

                QList<QUrl> tmpurls;
                tmpurls += QUrl::fromStringList(results.split("\n"));
                for (QUrl url : tmpurls) {
                    if (!url.toString().startsWith(Global::Scheme::kFile))
                        continue;

                    QString path = url.path();
                    path = path.replace(QRegExp("/*/"), "/");
                    if (path.isEmpty() || path == "/")
                        continue;
                    QUrl temp = QUrl::fromLocalFile(path);
                    currentClipboardFileUrls << temp;
                }

                if (currentClipboardFileUrls.count() > 0)
                    break;
            } while (true);

            if (!isCanceled) {
                XGetWindowProperty(display, window, propid, 0, LONG_MAX / 4, True, AnyPropertyType, &fmtid, &resbits, &ressize, &restail, (unsigned char **)&result);
                if (QString(result) != "/")
                    results += QString(result);
                XFree(result);
            }
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    if (isCanceled) {
        qWarning() << "user cancel remote download !";
        return QList<QUrl>();
    }

    urls += QUrl::fromStringList(results.split("\n"));

    QList<QUrl> clipboardFileUrls;
    for (QUrl url : urls) {
        //链接文件的inode不加入clipbordFileinode，只用url判断clip，避免多个同源链接文件的逻辑误判
        if (!url.toString().startsWith(Global::Scheme::kFile))
            continue;

        QString path = url.path();
        path = path.replace(QRegExp("/*/"), "/");
        if (path.isEmpty() || path == "/")
            continue;
        QUrl temp = QUrl::fromLocalFile(path);
        clipboardFileUrls << temp;
    }

    qInfo() << results << urls << clipboardFileUrls;

    if (GlobalData::clipboardAction == kRemoteAction && currentCount == GlobalData::remoteCurrentCount) {
        QMutexLocker lk(&GlobalData::clipboardFileUrlsMutex);
        GlobalData::clipboardFileUrls.clear();
        GlobalData::clipboardFileUrls = clipboardFileUrls;
        GlobalData::remoteCurrentCount = 0;
    }

    return clipboardFileUrls;
}

void ClipBoard::onClipboardDataChanged()
{
    GlobalData::onClipboardDataChanged();

    emit clipboardDataChanged();
}
