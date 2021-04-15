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

#ifndef FILESERVICES_H
#define FILESERVICES_H

#include "dabstractfilecontroller.h"
#include "durl.h"
#include "dfmabstracteventhandler.h"

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>
#include <QDebug>
#include <QtNetwork/qnetworkreply.h>

#include <functional>

DFM_USE_NAMESPACE

typedef QPair<QString, QString> HandlerType;
typedef QPair<QString, std::function<DAbstractFileController*()>> HandlerCreatorType;

class DFMUrlListBaseEvent;
class DAbstractFileInfo;
class JobController;
class DFMCreateGetChildrensJob;
class DFileServicePrivate;

DFM_BEGIN_NAMESPACE
class DFileHandler;
class DFileDevice;
class DStorageInfo;
DFM_END_NAMESPACE

class DFileService : public QObject, public DFMAbstractEventHandler
{
    Q_OBJECT

public:
    enum class AddTextFlags : std::size_t {
        Before = 0,
        After  = 1
    };

    template <class T>
    static void dRegisterUrlHandler(const QString &scheme, const QString &host)
    {
        if (isRegisted<T>(scheme, host)) {
            return;
        }

        insertToCreatorHash(HandlerType(scheme, host), HandlerCreatorType(typeid(T).name(), [ = ] {
            DAbstractFileController *handler = new T();
            DFileService *that = instance();

            if (handler->thread() != that->thread())
            {
                handler->moveToThread(that->thread());
            }

            handler->setParent(that);

            return handler;
        }));
    }
    static bool isRegisted(const QString &scheme, const QString &host, const std::type_info &info);
    template <class T>
    static bool isRegisted(const QString &scheme, const QString &host)
    {
        return isRegisted(scheme, host, typeid(T));
    }
    static bool isRegisted(const QString &scheme, const QString &host);

    static void initHandlersByCreators();

    static void printStacktrace(int level = 4);

    static DFileService *instance();

    static bool setFileUrlHandler(const QString &scheme, const QString &host,
                                  DAbstractFileController *controller);
    static void unsetFileUrlHandler(DAbstractFileController *controller);
    static void clearFileUrlHandler(const QString &scheme, const QString &host);

    static QList<DAbstractFileController *> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                                bool ignoreHost = false,
                                                                bool ignoreScheme = false);

    bool openFile(const QObject *sender, const DUrl &url) const;
    bool openFiles(const QObject *sender, const DUrlList &list, const bool isEnter = false) const;
    bool openFileByApp(const QObject *sender, const QString &appName, const DUrl &url) const;
    bool openFilesByApp(const QObject *sender, const QString &appName, const QList<DUrl> &urllist, const bool isenter = false) const;
    bool compressFiles(const QObject *sender, const DUrlList &list) const;
    bool decompressFile(const QObject *sender, const DUrlList &list) const;
    bool decompressFileHere(const QObject *sender, const DUrlList &list) const;
    bool writeFilesToClipboard(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrlList &list) const;
    bool renameFile(const QObject *sender, const DUrl &from, const DUrl &to, const bool silent = false) const;
    bool multiFilesReplaceName(const QList<DUrl> &urls, const QPair<QString, QString> &pair)const;
    bool multiFilesAddStrToName(const QList<DUrl> &urls, const QPair<QString, DFileService::AddTextFlags> &pair)const;
    bool multiFilesCustomName(const QList<DUrl> &urls, const QPair<QString, QString> &pair)const;
    bool deleteFiles(const QObject *sender, const DUrlList &list, bool confirmationDialog = true, bool slient = false, bool force = false) const;
    DUrlList moveToTrash(const QObject *sender, const DUrlList &list) const;
    void pasteFileByClipboard(const QObject *sender, const DUrl &targetUrl) const;
    DUrlList pasteFile(const QObject *sender, DFMGlobal::ClipboardAction action,
                       const DUrl &targetUrl, const DUrlList &list) const;
    bool restoreFile(const QObject *sender, const DUrlList &list) const;
    bool mkdir(const QObject *sender, const DUrl &targetUrl) const;
    bool touchFile(const QObject *sender, const DUrl &targetUrl) const;
    bool openFileLocation(const QObject *sender, const DUrl &url) const;
    bool setPermissions(const QObject *sender, const DUrl &url, const QFileDevice::Permissions permissions) const;

    bool addToBookmark(const QObject *sender, const DUrl &fileUrl) const;
    bool removeBookmark(const QObject *sender, const DUrl &fileUrl) const;
    bool createSymlink(const QObject *sender, const DUrl &fileUrl) const;
    bool createSymlink(const QObject *sender, const DUrl &fileUrl, const DUrl &linkToUrl, bool force = false) const;
    bool sendToDesktop(const QObject *sender, const DUrlList &urlList) const;
    void sendToBluetooth(const DUrlList &urlList) const;

    bool shareFolder(const QObject *sender, const DUrl &fileUrl, const QString &name, bool isWritable = false, bool allowGuest = false);
    bool unShareFolder(const QObject *sender, const DUrl &fileUrl) const;
    bool openInTerminal(const QObject *sender, const DUrl &fileUrl) const;

    ///###: for tag protocol.
    bool setFileTags(const QObject *sender, const DUrl &url, const QList<QString> &tags) const;
    bool makeTagsOfFiles(const QObject *sender, const DUrlList &urlList, const QStringList &tags, const QSet<QString> dirtyTagFilter = QSet<QString>()) const;
    bool removeTagsOfFile(const QObject *sender, const DUrl &url, const QList<QString> &tags) const;
    QList<QString> getTagsThroughFiles(const QObject *sender, const QList<DUrl> &urls, const bool loopEvent = false) const;

    const DAbstractFileInfoPointer createFileInfo(const QObject *sender, const DUrl &fileUrl, const bool isFromCache = true) const;
    const DDirIteratorPointer createDirIterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, bool silent = false, bool isgvfs = false) const;

    const QList<DAbstractFileInfoPointer> getChildren(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                      QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, bool silent = false, bool canconst = false) const;

    JobController *getChildrenJob(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters,
                                  QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags, bool silent = false, const bool isgfvs = false) const;

    DAbstractFileWatcher *createFileWatcher(const QObject *sender, const DUrl &fileUrl, QObject *parent = nullptr) const;
    bool setExtraProperties(const QObject *sender, const DUrl &fileUrl, const QVariantHash &ep) const;

    DFileDevice *createFileDevice(const QObject *sender, const DUrl &url);
    DFileHandler *createFileHandler(const QObject *sender, const DUrl &url);
    DStorageInfo *createStorageInfo(const QObject *sender, const DUrl &url);


    //set cursor busy status
    void setCursorBusyState(const bool bbusy);
    //check networkfile is busy(or network is unline)
    bool checkGvfsMountfileBusy(const DUrl &url, const bool showdailog = true);
    bool checkGvfsMountfileBusy(const DUrl &rootUrl, const QString &rootFileName, const bool bShowDailog = true);
    //获取是否是正在清空回收站 fix bug 31324,
    bool getDoClearTrashState() const;
    //设置当前是否是在清空回收站 fix bug 31324,
    void setDoClearTrashState(const bool bdoing);
    //处理复制、粘贴和剪切(拷贝)结束后操作 fix bug 35855
    void dealPasteEnd(const QSharedPointer<DFMEvent> &event, const DUrlList &result);
    //判断当前的可访问的smb和ftp中是否包含某个url
    bool isSmbFtpContain(const DUrl &url);

    void onTagEditorChanged(const QStringList &tags, const DUrlList &files);
signals:
    void fileOpened(const DUrl &fileUrl) const;
    void fileCopied(const DUrl &source, const DUrl &target) const;
    void fileDeleted(const DUrl &fileUrl) const;
    void fileMovedToTrash(const DUrl &from, const DUrl &to) const;
    void fileRenamed(const DUrl &from, const DUrl &to) const;

private slots:
    void laterRequestSelectFiles(const DFMUrlListBaseEvent &event) const;

private:
    explicit DFileService(QObject *parent = nullptr);
    ~DFileService() override;

    bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData = nullptr) override;

    static QString getSymlinkFileName(const DUrl &fileUrl, const QDir &targetDir = QDir());
    static void insertToCreatorHash(const HandlerType &type, const HandlerCreatorType &creator);
    static bool checkMultiSelectionFilesCache();

    QScopedPointer<DFileServicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFileService)
};

#endif // FILESERVICES_H
