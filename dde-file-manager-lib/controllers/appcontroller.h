#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H


#include <QObject>

#include "subscriber.h"
#include "dfmevent.h"

class FileController;
class FileMonitor;

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
    void actionOpen(const QSharedPointer<DFMUrlListBaseEvent>& event);
    void actionOpenDisk(const QSharedPointer<DFMUrlBaseEvent> &event);
    void asycOpenDisk(const QString& path);

    void actionOpenInNewWindow(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionOpenInNewTab(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionOpenDiskInNewTab(const QSharedPointer<DFMUrlBaseEvent> &event);
    void asycOpenDiskInNewTab(const QString& path);
    void actionOpenDiskInNewWindow(const QSharedPointer<DFMUrlBaseEvent> &event);
    void asycOpenDiskInNewWindow(const QString& path);
    void actionOpenAsAdmin(const QSharedPointer<DFMUrlBaseEvent> &event);

    void actionOpenWithCustom(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionOpenFileLocation(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCompress(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionDecompress(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionDecompressHere(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCut(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCopy(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionPaste(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionRename(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionBookmarkRename(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionBookmarkRemove(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionDelete(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCompleteDeletion(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionCreateSymlink(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionSendToDesktop(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionAddToBookMark(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewFolder(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewFile(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionSelectAll(quint64 winId);
    void actionClearRecent(const QSharedPointer<DFMMenuActionEvent>& event);
    void actionClearTrash(const QObject *sender = 0);
    void actionNewWord(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewExcel(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewPowerpoint(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionNewText(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionMount(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionUnmount(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionRestore(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionRestoreAll(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionEject(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionOpenInTerminal(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionProperty(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionNewWindow(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionHelp();
    void actionAbout(quint64 winId);
    void actionExit(quint64 winId);
    void actionSetAsWallpaper(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionShare(const QSharedPointer<DFMUrlListBaseEvent> &event);
    void actionUnShare(const QSharedPointer<DFMUrlBaseEvent> &event);
    void actionSetUserSharePassword(quint64 winId);
    void actionSettings(quint64 winId);
    void actionFormatDevice(const QSharedPointer<DFMUrlBaseEvent> &event);


    void actionctrlL(quint64 winId);
    void actionctrlF(quint64 winId);

    void actionExitCurrentWindow(quint64 winId);
    void actionShowHotkeyHelp(quint64 winId);
    void actionBack(quint64 winId);
    void actionForward(quint64 winId);

    void actionForgetPassword(const QSharedPointer<DFMUrlBaseEvent> &event);

    void actionOpenFileByApp();

#ifdef SW_LABEL
    void actionSetLabel(const DFMEvent& event);
    void actionViewLabel(const DFMEvent& event);
    void actionEditLabel(const DFMEvent& event);
    void actionPrivateFileToPublic(const DFMEvent& event);
    void actionByIds(const DFMEvent& event, QString actionId);
#endif

    // Subscriber interface
public:
    void doSubscriberAction(const QString &path);

    static QString createFile(const QString &sourceFile, const QString &targetDir, const QString &baseFileName, WId windowId);

protected:
    explicit AppController(QObject *parent = 0);

private:
    void initConnect();
    void createGVfSManager();
    void createUserShareManager();

    QSharedPointer<DFMEvent> m_fmEvent;
    static QPair<DUrl, quint64> selectionAndRenameFile;

    friend class FileController;
    friend class DFileSystemModel;
};

#endif // APPCONTROLLER_H
