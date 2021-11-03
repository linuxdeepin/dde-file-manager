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

#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <atomic>
#include <QObject>

#include "subscriber.h"
#include "dfmevent.h"
#include "dbusinterface/startmanager_interface.h"
#include "dbusinterface/introspectable_interface.h"


class FileController;
class FileMonitor;
class DRenameBar;
class FileBatchProcess;
class DUMountManager;

/**
 * @brief The UnmountWorker class 卸载操作类 用于在子线程执行卸载操作
 */
class UnmountWorker : public QObject
{
    Q_OBJECT

public slots:
    void doUnmount(const QString &blkStr);
    void doSaveRemove(const QString &blkStr);

signals:
    void unmountResult(const QString &title, const QString &msg);
};

class DesktopInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.dde.desktop"; }
public:
    DesktopInterface(const QString &service, const QString &path,
                     const QDBusConnection &connection, QObject *parent = nullptr);
    ~DesktopInterface();
};


class AppController : public QObject, public Subscriber
{
    Q_OBJECT

public:
    enum ActionType {
        Open,
        OpenNewWindow,
        OpenNewTab
    };

    static AppController *instance();
    static void registerUrlHandle();

public slots:
    void actionOpen(const QSharedPointer<DFMUrlListBaseEvent> &event, const bool isEnter = false);
    void actionOpenDisk(const QSharedPointer<DFMUrlBaseEvent> &event);
    void asyncOpenDisk(const QString &path);

    void actionOpenInNewWindow(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionOpenInNewTab(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionOpenDiskInNewTab(const QSharedPointer<DFMUrlBaseEvent> &event);
    void asyncOpenDiskInNewTab(const QString &path);
    void actionOpenDiskInNewWindow(const QSharedPointer<DFMUrlBaseEvent> &event);
    void asyncOpenDiskInNewWindow(const QString &path);
    void actionOpenAsAdmin(const QSharedPointer<DFMUrlBaseEvent> &event);

    void actionOpenWithCustom(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionOpenFilesWithCustom(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionOpenFileLocation(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCompress(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionDecompress(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionDecompressHere(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCut(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCopy(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionPaste(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionRename(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionBookmarkRename(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionBookmarkRemove(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionDelete(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCompleteDeletion(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCreateSymlink(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionSendToDesktop(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionSendToBluetooth();
    void actionAddToBookMark(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewFolder(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionSelectAll(quint64 winId);
    void actionClearRecent(const QSharedPointer<DFMMenuActionEvent> &event);
    void actionClearRecent();
    void actionClearTrash(const QObject *sender = nullptr);
    void actionNewWord(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewExcel(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewPowerpoint(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewText(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionMount(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionMountImage(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionUnmount(const QSharedPointer<DFMUrlBaseEvent> &event);
    void doActionUnmount(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionRestore(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionRestoreAll(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionEject(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionSafelyRemoveDrive(const QSharedPointer<DFMUrlBaseEvent> &event);
    void doSafelyRemoveDrive(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionOpenInTerminal(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionProperty(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionNewWindow(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionExit(quint64 winId);
    void actionSetAsWallpaper(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionShare(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionUnShare(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionConnectToServer(quint64 winId);
    void actionSetUserSharePassword(quint64 winId);
    void actionSettings(quint64 winId);
    void actionFormatDevice(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionOpticalBlank(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionRemoveStashedMount(const QSharedPointer<DFMUrlBaseEvent> &event);


    void actionctrlL(quint64 winId);
    void actionctrlF(quint64 winId);

    void actionExitCurrentWindow(quint64 winId);
    void actionShowHotkeyHelp(quint64 winId);
    void actionBack(quint64 winId);
    void actionForward(quint64 winId);

    void actionForgetPassword(const QSharedPointer<DFMUrlBaseEvent> &event);

    void actionOpenFileByApp();
    void actionSendToRemovableDisk();
    void actionStageFileForBurning();


    ///###: tag protocol
    QList<QString> actionGetTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event);
    bool actionRemoveTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event);
    void actionChangeTagColor(const QSharedPointer<DFMChangeTagColorEvent> &event);
    void showTagEdit(const QRect &parentRect, const QPoint &globalPos, const DUrlList &fileList);

#ifdef SW_LABEL
    void actionSetLabel(const DFMEvent &event);
    void actionViewLabel(const DFMEvent &event);
    void actionEditLabel(const DFMEvent &event);
    void actionPrivateFileToPublic(const DFMEvent &event);
    void actionByIds(const DFMEvent &event, QString actionId);
#endif

    // Subscriber interface
public:
    void doSubscriberAction(const QString &path) override;

    static QString createFile(const QString &sourceFile, const QString &targetDir, const QString &baseFileName, WId windowId);

signals:
    void doUnmount(const QString &blk);
    void doSaveRemove(const QString &blk);

protected:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

private:
    void initConnect();
    void createGVfSManager();
    void createUserShareManager();
    void createDBusInterface();
    void showErrorDialog(const QString &title, const QString &content);
    void popQueryScanningDialog(QObject *object, std::function<void()> onStop);
    void refreshDesktop();

    QSharedPointer<DFMEvent> m_fmEvent;
    static QPair<DUrl, quint64> selectionAndRenameFile;        //###: for creating new file.
    static QPair<DUrl, quint64> selectionFile;                //###: rename a file which must be existance.

    StartManagerInterface *m_startManagerInterface = nullptr;
    IntrospectableInterface *m_introspectableInterface = nullptr;
    QThread m_unmountThread;
    UnmountWorker *m_unmountWorker;
    QScopedPointer<DUMountManager> m_umountManager;

    volatile enum {UnkownIFS,NoneIFS,CreatingIFS,VaildIFS} m_statDBusInterface = UnkownIFS; //dbus接口创建状态

    friend class FileController;
    friend class MergedDesktopController;
    friend class DFileSystemModel;
    friend class DFileViewHelper;
    friend class DRenameBar;
    friend class FileBatchProcess;
    friend class VaultController;

public:
    static QPair<QList<DUrl>, quint64> multiSelectionFilesCache;  //###: for multi selection.
    static std::atomic<quint64> multiSelectionFilesCacheCounter;
    static std::atomic<bool> flagForDDesktopRenameBar;

    StartManagerInterface *startManagerInterface() const;
    bool checkLaunchAppInterface();
};

#endif // APPCONTROLLER_H
