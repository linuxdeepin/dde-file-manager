#ifndef ABSTRACTFILECONTROLLER_H
#define ABSTRACTFILECONTROLLER_H

#include "dabstractfileinfo.h"
#include "ddiriterator.h"

#include <QObject>
#include <QDir>
#include <QDirIterator>

class DFMEvent;
class DFMOpenFileEvent;
class DFMOpenFileByAppEvent;
class DFMCompressEvnet;
class DFMDecompressEvnet;
class DFMWriteUrlsToClipboardEvent;
class DFMRenameEvent;
class DFMDeleteEvent;
class DFMMoveToTrashEvent;
class DFMRestoreFromTrashEvent;
class DFMPasteEvent;
class DFMNewFolderEvent;
class DFMNewFileEvent;
class DFMOpenFileLocation;
class DFMCreateSymlinkEvent;
class DFMGetChildrensEvent;
class DFMCreateDiriterator;
class DFMCreateFileInfoEvnet;
class DFMCreateFileWatcherEvent;
class DFMOpenInTerminalEvent;
class DFMFileShareEvnet;
class DFMCancelFileShareEvent;
class DUrl;
class DAbstractFileWatcher;
typedef QList<DUrl> DUrlList;
class DAbstractFileController : public QObject
{
    Q_OBJECT

public:
    explicit DAbstractFileController(QObject *parent = 0);

    virtual bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const;
    virtual bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const;
    virtual bool compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const;
    virtual bool decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const;
    virtual bool decompressFileHere(const QSharedPointer<DFMDecompressEvnet> &event) const;
    virtual bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const;
    virtual bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const;
    virtual bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const;
    virtual DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const;
    virtual DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const;
    virtual bool restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const;
    virtual bool newFolder(const QSharedPointer<DFMNewFolderEvent> &event) const;
    virtual bool newFile(const QSharedPointer<DFMNewFileEvent> &event) const;

    virtual bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const;

    virtual const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const;
    virtual const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const;
    virtual const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const;

    virtual bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const;
    virtual bool shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const;
    virtual bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const;
    virtual bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const;

    virtual DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const;
};

#endif // ABSTRACTFILECONTROLLER_H
