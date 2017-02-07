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
    void actionOpen(const DFMEvent& event);
    void actionOpenDisk(const DFMEvent& event);
    void asycOpenDisk(const QString& path);

    void actionOpenInNewWindow(const DFMEvent& event);
    void actionOpenInNewTab(const DFMEvent& event);
    void actionOpenDiskInNewTab(const DFMEvent& event);
    void asycOpenDiskInNewTab(const QString& path);
    void actionOpenDiskInNewWindow(const DFMEvent& event);
    void asycOpenDiskInNewWindow(const QString& path);
    void actionOpenAsAdmain(const DFMEvent& event);

    void actionOpenWithCustom(const DFMEvent& event);
    void actionOpenFileLocation(const DFMEvent& event);
    void actionCompress(const DFMEvent& event);
    void actionDecompress(const DFMEvent& event);
    void actionDecompressHere(const DFMEvent& event);
    void actionCut(const DFMEvent& event);
    void actionCopy(const DFMEvent& event);
    void actionPaste(const DFMEvent& event);
    void actionRename(const DFMEvent& event);
    void actionRemove(const DFMEvent& event);
    void actionDelete(const DFMEvent& event);
    void actionCompleteDeletion(const DFMEvent& event);
    void actionCreateSymlink(const DFMEvent& event);
    void actionSendToDesktop(const DFMEvent& event);
    void actionAddToBookMark(const DFMEvent& event);
    void actionNewFolder(const DFMEvent& event);
    void actionNewFile(const DFMEvent& event);
    void actionSelectAll(const DFMEvent& event);
    void actionClearRecent(const DFMEvent& event);
    void actionClearTrash(const DFMEvent& event);
    void actionNewWord(const DFMEvent& event);
    void actionNewExcel(const DFMEvent& event);
    void actionNewPowerpoint(const DFMEvent& event);
    void actionNewText(const DFMEvent& event);
    void actionMount(const DFMEvent& event);
    void actionUnmount(const DFMEvent& event);
    void actionRestore(const DFMEvent& event);
    void actionRestoreAll(const DFMEvent& event);
    void actionEject(const DFMEvent& event);
    void actionOpenInTerminal(const DFMEvent& event);
    void actionProperty(const DFMEvent& event);
    void actionNewWindow(const DFMEvent& event);
    void actionHelp(const DFMEvent& event);
    void actionAbout(const DFMEvent& event);
    void actionExit(const DFMEvent& event);
    void actionSetAsWallpaper(const DFMEvent& event);
    void actionShare(const DFMEvent& event);
    void actionUnShare(const DFMEvent& event);
    void actionSetUserSharePassword(const DFMEvent& event);
    void actionSettings(const DFMEvent& event);
    void actionFormatDevice(const DFMEvent& event);


    void actionctrlL(const DFMEvent& event);
    void actionctrlF(const DFMEvent& event);

    void actionExitCurrentWindow(const DFMEvent& event);
    void actionShowHotkeyHelp(const DFMEvent& event);
    void actionBack(const DFMEvent& event);
    void actionForward(const DFMEvent& event);

    void actionForgetPassword(const DFMEvent& event);

    void actionOpenFileByApp();

    // Subscriber interface
public:
    void doSubscriberAction(const QString &path);

protected:
    explicit AppController(QObject *parent = 0);

private:
    void initConnect();
    void createGVfSManager();
    void createUserShareManager();

    DFMEvent m_fmEvent;
    static QPair<DUrl, int> selectionAndRenameFile;

    friend class FileController;
    friend class DFileSystemModel;
};

#endif // APPCONTROLLER_H
