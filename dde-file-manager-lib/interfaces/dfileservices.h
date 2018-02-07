/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <functional>

DFM_USE_NAMESPACE

typedef QPair<QString,QString> HandlerType;
typedef QPair<QString, std::function<DAbstractFileController*()>> HandlerCreatorType;

class DFMUrlListBaseEvent;
class DAbstractFileInfo;
class JobController;
class DFMCreateGetChildrensJob;
class DFileServicePrivate;
class DFileService : public QObject, public DFMAbstractEventHandler
{
    Q_OBJECT

public:
    enum class AddTextFlags : std::size_t
    {
        Before = 0,
        After  = 1
    };

    template <class T>
    static void dRegisterUrlHandler(const QString &scheme, const QString &host)
    {
        if (isRegisted<T>(scheme, host))
            return;

        insertToCreatorHash(HandlerType(scheme, host), HandlerCreatorType(typeid(T).name(), [=] {
            return (DAbstractFileController*)new T(instance());
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

    static DFileService *instance();

    static bool setFileUrlHandler(const QString &scheme, const QString &host,
                                  DAbstractFileController *controller);
    static void unsetFileUrlHandler(DAbstractFileController *controller);
    static void clearFileUrlHandler(const QString &scheme, const QString &host);

    static QList<DAbstractFileController*> getHandlerTypeByUrl(const DUrl &fileUrl,
                                                               bool ignoreHost = false,
                                                               bool ignoreScheme = false);

    bool openFile(const QObject *sender, const DUrl &url) const;
    bool openFileByApp(const QObject *sender, const QString &appName, const DUrl &url) const;
    bool compressFiles(const QObject *sender, const DUrlList &list) const;
    bool decompressFile(const QObject *sender, const DUrlList &list) const;
    bool decompressFileHere(const QObject *sender, const DUrlList &list) const;
    bool writeFilesToClipboard(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrlList &list) const;
    bool renameFile(const QObject *sender, const DUrl &from, const DUrl &to) const;
    bool multiFilesReplaceName(const QList<DUrl>& urls, const QPair<QString, QString>& pair)const;
    bool multiFilesAddStrToName(const QList<DUrl>& urls, const QPair<QString, DFileService::AddTextFlags>& pair)const;
    bool multiFilesCustomName(const QList<DUrl>& urls, const QPair<QString, QString> &pair)const;
    bool deleteFiles(const QObject *sender, const DUrlList &list, bool slient = false) const;
    DUrlList moveToTrash(const QObject *sender, const DUrlList &list) const;
    void pasteFileByClipboard(const QObject *sender, const DUrl &targetUrl) const;
    DUrlList pasteFile(const QObject *sender, DFMGlobal::ClipboardAction action,
                       const DUrl &targetUrl, const DUrlList &list) const;
    bool restoreFile(const QObject *sender, const DUrlList &list) const;
    bool mkdir(const QObject *sender, const DUrl &targetUrl) const;
    bool touchFile(const QObject *sender, const DUrl &targetUrl) const;
    bool openFileLocation(const QObject *sender, const DUrl &url) const;

    bool createSymlink(const QObject *sender, const DUrl &fileUrl) const;
    bool createSymlink(const QObject *sender, const DUrl &fileUrl, const DUrl &linkToUrl) const;
    bool sendToDesktop(const QObject *sender, const DUrlList &urlList) const;

    bool shareFolder(const QObject *sender, const DUrl &fileUrl, const QString &name, bool isWritable = false, bool allowGuest = false);
    bool unShareFolder(const QObject *sender, const DUrl &fileUrl) const;
    bool openInTerminal(const QObject *sender, const DUrl &fileUrl) const;

    const DAbstractFileInfoPointer createFileInfo(const QObject *sender, const DUrl &fileUrl) const;
    const DDirIteratorPointer createDirIterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags) const;

    const QList<DAbstractFileInfoPointer> getChildren(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters, QDir::Filters filters,
                                                      QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    JobController *getChildrenJob(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags) const;

    DAbstractFileWatcher *createFileWatcher(const QObject *sender, const DUrl &fileUrl, QObject *parent = 0) const;

signals:
    void fileOpened(const DUrl &fileUrl) const;

private slots:
    void laterRequestSelectFiles(const DFMUrlListBaseEvent &event) const;

private:
    explicit DFileService(QObject *parent = 0);
    ~DFileService();

    bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData = 0) Q_DECL_OVERRIDE;

    static QString getSymlinkFileName(const DUrl &fileUrl, const QDir &targetDir = QDir());
    static void insertToCreatorHash(const HandlerType &type, const HandlerCreatorType &creator);
    static bool checkMultiSelectionFilesCache();

    QScopedPointer<DFileServicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFileService)
};

#endif // FILESERVICES_H
