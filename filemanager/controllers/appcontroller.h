#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H


#include <QObject>

class FileController;
class FileMonitor;
class FMEvent;

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = 0);
    ~AppController();

    void initConnect();

public slots:
    void actionOpen(const FMEvent& event);
    void actionOpenInNewWindow(const FMEvent& event);
    void actionOpenWithCustom(const FMEvent& event);
    void actionOpenFileLocation(const FMEvent& event);
    void actionCompress(const FMEvent& event);
    void actionDecompress(const FMEvent& event);
    void actionDecompressHere(const FMEvent& event);
    void actionCut(const FMEvent& event);
    void actionCopy(const FMEvent& event);
    void actionPaste(const FMEvent& event);
    void actionRename(const FMEvent& event);
    void actionRemove(const FMEvent& event);
    void actionDelete(const FMEvent& event);
    void actionCompleteDeletion(const FMEvent& event);
    void actionCreateSoftLink(const FMEvent& event);
    void actionSendToDesktop(const FMEvent& event);
    void actionAddToBookMark(const FMEvent& event);
    void actionNewFolder(const FMEvent& event);
    void actionNewFile(const FMEvent& event);
    void actionSelectAll(const FMEvent& event);
    void actionClearRecent(const FMEvent& event);
    void actionClearTrash(const FMEvent& event);
    void actionNewWord(const FMEvent& event);
    void actionNewExcel(const FMEvent& event);
    void actionNewPowerpoint(const FMEvent& event);
    void actionNewText(const FMEvent& event);
    void actionMount(const FMEvent& event);
    void actionUnmount(const FMEvent& event);
    void actionRestore(const FMEvent& event);
    void actionRestoreAll(const FMEvent& event);
    void actionEject(const FMEvent& event);
    void actionOpenInTerminal(const FMEvent& event);
    void actionProperty(const FMEvent& event);
    void actionNewWindow(const FMEvent& event);
    void actionHelp(const FMEvent& event);
    void actionAbout(const FMEvent& event);
    void actionExit(const FMEvent& event);
    void actionSetAsWallpaper(const FMEvent& event);
};

#endif // APPCONTROLLER_H
