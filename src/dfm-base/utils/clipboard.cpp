// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboard.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/file/local/localfileiconprovider.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/utils/fileutils.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMutex>
#include <QDebug>
#include <QUrl>

#include <DThumbnailProvider>

#include <sys/stat.h>
#include <unistd.h>
#include <X11/Xlib.h>

using namespace dfmbase;

namespace GlobalData {
static QList<QUrl> clipboardFileUrls;
static QMutex clipboardFileUrlsMutex;
static QAtomicInt remoteCurrentCount = 0;
static ClipBoard::ClipboardAction clipboardAction = ClipBoard::kUnknownAction;

static constexpr char kUserIdKey[] = "userId";
static constexpr char kRemoteCopyKey[] = "uos/remote-copy";
static constexpr char kGnomeCopyKey[] = "x-special/gnome-copied-files";
static constexpr char kRemoteAssistanceCopyKey[] = "uos/remote-copied-files";

void onClipboardDataChanged()
{

    QMutexLocker lk(&clipboardFileUrlsMutex);
    clipboardFileUrls.clear();

    const QMimeData *mimeData = qApp->clipboard()->mimeData();
    if (!mimeData || mimeData->formats().isEmpty()) {
        qCWarning(logDFMBase) << "get null mimeData from QClipBoard or remote formats is null!";
        return;
    }
    if (mimeData->hasFormat(kRemoteCopyKey)) {
        qCWarning(logDFMBase) << "clipboard use other !";
        clipboardAction = ClipBoard::kRemoteAction;
        remoteCurrentCount++;
        return;
    }
    // 远程协助功能
    if (mimeData->hasFormat(kRemoteAssistanceCopyKey)) {
        qCInfo(logDFMBase) << "Remote copy: set remote copy action";
        clipboardAction = ClipBoard::kRemoteCopiedAction;
        return;
    }
    // 没有文件拷贝
    if (!mimeData->hasFormat(kGnomeCopyKey)) {
        qCWarning(logDFMBase) << "no kGnomeCopyKey target in mimedata formats!";
        clipboardAction = ClipBoard::kUnknownAction;
        return;
    }
    const QString &data = mimeData->data(kGnomeCopyKey);
    const static QRegularExpression regCut("cut\nfile://"), regCopy("copy\nfile://");
    if (data.contains(regCut)) {
        clipboardAction = ClipBoard::kCutAction;
    } else if (data.contains(regCopy)) {
        clipboardAction = ClipBoard::kCopyAction;
    } else {
        qCWarning(logDFMBase) << "wrong kGnomeCopyKey data = " << data;
        clipboardAction = ClipBoard::kUnknownAction;
    }

    for (const auto &url : mimeData->urls()) {
        if (url.isValid() && !url.scheme().isEmpty())
            clipboardFileUrls << url;
    }
}
}   // namespace GlobalData

ClipBoard::ClipBoard(QObject *parent)
    : QObject(parent)
{
    connect(qApp->clipboard(), &QClipboard::dataChanged, this, [this]() {
        onClipboardDataChanged();
        emit clipboardDataChanged();
    });
}

ClipBoard *ClipBoard::instance()
{
    static ClipBoard ins;
    return &ins;
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

    if (action == ClipBoard::kCutAction && SystemPathUtil::instance()->checkContainsSystemPath(list))
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
        ba.append(qurl.toString().toUtf8());

        const QString &path = qurl.toLocalFile();
        if (!path.isEmpty()) {
            text += path + '\n';
        }

        if (maxIconsNum-- > 0) {
            const FileInfoPointer &info = InfoFactory::create<FileInfo>(qurl, Global::CreateFileInfoType::kCreateFileInfoAuto, &error);

            if (!info) {
                qCWarning(logDFMBase) << QString("create file info error, case : %1").arg(error);
                continue;
            }
            QStringList iconList;
            if (info->isAttributes(OptInfoType::kIsSymLink)) {
                iconList << "emblem-symbolic-link";
            }
            if (!info->isAttributes(OptInfoType::kIsWritable)) {
                iconList << "emblem-readonly";
            }
            if (!info->isAttributes(OptInfoType::kIsReadable)) {
                iconList << "emblem-unreadable";
            }
            // TODO lanxs::目前缩略图还没有处理，等待处理完成了在修改
            // 多文件时只显示文件图标, 一个文件时显示缩略图(如果有的话)
            QIcon icon = LocalFileIconProvider::globalProvider()->icon(info.data());
            FileInfo::FileType fileType = MimeTypeDisplayManager::
                                                  instance()
                                                          ->displayNameToEnum(info->nameOf(NameInfoType::kMimeTypeName));
            if (list.size() == 1 && fileType == FileInfo::FileType::kImages) {
                QIcon thumb(DTK_GUI_NAMESPACE::DThumbnailProvider::instance()->thumbnailFilePath(QFileInfo(info->pathOf(PathInfoType::kAbsoluteFilePath)),
                                                                                                 DTK_GUI_NAMESPACE::DThumbnailProvider::Large));
                if (thumb.isNull()) {
                    //qCWarning(logDFMBase) << "thumbnail file faild " << fileInfo->absoluteFilePath();
                } else {
                    icon = thumb;
                }
            }
            stream << iconList << icon;
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
        userId.append(QString::number(getuid()).toUtf8());
        mimeData->setData(GlobalData::kUserIdKey, userId);
    }

    qApp->clipboard()->setMimeData(mimeData);
}
/*!
 * \brief ClipBoard::setCurUrlToClipboardForRemote Set Remote Assistance target urls
 * \param curUrl
 */
void ClipBoard::setCurUrlToClipboardForRemote(const QUrl &curUrl)
{
    if (curUrl.isEmpty())
        return;
    QByteArray localPath;
    if (dfmbase::FileUtils::isLocalFile(curUrl)) {
        localPath = curUrl.toString().toLocal8Bit();
    } else {
        qCInfo(logDFMBase) << "Remote Assistance copy: current url not local file";
        return;
    }

    if (localPath.isEmpty())
        return;
    QMimeData *mimeData = new QMimeData();
    mimeData->setData(GlobalData::kRemoteAssistanceCopyKey, localPath);
    mimeData->setText(curUrl.toString());
    qApp->clipboard()->setMimeData(mimeData);
}
/*!
 * \brief ClipBoard::setDataToClopboard Set user data to clipboard
 * \param mimeData
 */
void ClipBoard::setDataToClipboard(QMimeData *mimeData)
{
    if (!mimeData) {
        qCWarning(logDFMBase) << "set data to clipboard failed, mimeData is null!";
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

    const QByteArray &userId = qApp->clipboard()->mimeData()->data(GlobalData::kUserIdKey);
    return userId.isEmpty() || (userId.toInt() == static_cast<int>(getuid()));
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
    QMutexLocker lk(&GlobalData::clipboardFileUrlsMutex);
    return GlobalData::clipboardFileUrls;
}
/*!
 * \brief ClipBoard::clipboardAction Gets the current operation of the clipboard
 * \return
 */
ClipBoard::ClipboardAction ClipBoard::clipboardAction() const
{
    return GlobalData::clipboardAction;
}

void ClipBoard::removeUrls(const QList<QUrl> &urls)
{
    QList<QUrl> clipboardUrls = GlobalData::clipboardFileUrls;
    ClipBoard::ClipboardAction action = GlobalData::clipboardAction;

    if (!clipboardUrls.isEmpty() && action != ClipBoard::kUnknownAction) {
        bool hasRemoved = false;
        for (int i = 0; i < urls.size() && !clipboardUrls.isEmpty(); ++i) {
            int cnt = clipboardUrls.removeAll(urls[i]);
            if (!hasRemoved && cnt != 0)
                hasRemoved = true;
        }

        if (clipboardUrls.isEmpty())
            clearClipboard();
        else if (hasRemoved)
            setUrlsToClipboard(clipboardUrls, action);
    }
}

void ClipBoard::replaceClipboardUrl(const QUrl &oldUrl, const QUrl &newUrl)
{
    QList<QUrl> clipboardUrls = GlobalData::clipboardFileUrls;
    ClipBoard::ClipboardAction action = GlobalData::clipboardAction;
    if (clipboardUrls.isEmpty() || action == ClipBoard::kUnknownAction)
        return;

    int index = clipboardUrls.indexOf(oldUrl);
    if (-1 == index)
        return;

    clipboardUrls.replace(index, newUrl);
    setUrlsToClipboard(clipboardUrls, action);
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
        qCWarning(logDFMBase) << "the clipboard mimedata is invalid!";
        return QList<QUrl>();
    }
    if (GlobalData::clipboardAction != kRemoteAction) {
        qCWarning(logDFMBase) << "current action is not RemoteAction ,error action " << GlobalData::clipboardAction;
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
                           &fmtid, &resbits, &ressize, &restail, reinterpret_cast<unsigned char **>(&result));
        if (fmtid != incrid) {
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
                                   &ressize, &restail, reinterpret_cast<unsigned char **>(&result));
                XFree(result);
                XGetWindowProperty(display, window, propid, 0, static_cast<long>(restail), True, AnyPropertyType, &fmtid, &resbits,
                                   &ressize, &restail, reinterpret_cast<unsigned char **>(&result));

                if (QString(result) != "/")
                    results += QString(result);

                XFree(result);

                QList<QUrl> tmpurls;
                tmpurls += QUrl::fromStringList(results.split("\n"));
                for (QUrl url : tmpurls) {
                    if (!url.toString().startsWith(Global::Scheme::kFile))
                        continue;

                    QString path = url.path();
                    path = path.replace(QRegularExpression("/*/"), "/");
                    if (path.isEmpty() || path == "/")
                        continue;
                    QUrl temp = QUrl::fromLocalFile(path);
                    currentClipboardFileUrls << temp;
                }

                if (currentClipboardFileUrls.count() > 0)
                    break;
            } while (true);

            if (!isCanceled) {
                XGetWindowProperty(display, window, propid, 0, LONG_MAX / 4,
                                   True, AnyPropertyType, &fmtid, &resbits, &ressize, &restail,
                                   reinterpret_cast<unsigned char **>(&result));
                if (QString(result) != "/")
                    results += QString(result);
                XFree(result);
            }
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    if (isCanceled) {
        qCWarning(logDFMBase) << "user cancel remote download !";
        return QList<QUrl>();
    }

    urls += QUrl::fromStringList(results.split("\n"));

    QList<QUrl> clipboardFileUrls;
    for (QUrl url : urls) {
        //链接文件的inode不加入clipbordFileinode，只用url判断clip，避免多个同源链接文件的逻辑误判
        if (!url.toString().startsWith(Global::Scheme::kFile))
            continue;

        QString path = url.path();
        path = path.replace(QRegularExpression("/*/"), "/");
        if (path.isEmpty() || path == "/")
            continue;
        QUrl temp = QUrl::fromLocalFile(path);
        clipboardFileUrls << temp;
    }

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
}
