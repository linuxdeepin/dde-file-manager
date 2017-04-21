#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include "dabstractfilecontroller.h"

class FileInfoGatherer;
class IconProvider;
class RecentHistoryManager;
class DAbstractFileInfo;
class FileMonitor;

class FileController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = 0);

    static bool findExecutable(const QString & executableName, const QStringList & paths = QStringList());

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const Q_DECL_OVERRIDE;
    bool decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const Q_DECL_OVERRIDE;
    bool decompressFileHere(const QSharedPointer<DFMDecompressEvnet> &event) const Q_DECL_OVERRIDE;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const Q_DECL_OVERRIDE;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const Q_DECL_OVERRIDE;
    bool newFolder(const QSharedPointer<DFMNewFolderEvent> &event) const Q_DECL_OVERRIDE;
    bool newFile(const QSharedPointer<DFMNewFileEvent> &event) const Q_DECL_OVERRIDE;

    bool shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const Q_DECL_OVERRIDE;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const Q_DECL_OVERRIDE;

    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

private:
    QString checkDuplicateName(const QString &name) const;
};

#endif // FILECONTROLLER_H
