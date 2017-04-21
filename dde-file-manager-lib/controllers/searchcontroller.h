#ifndef SEARCHCONTROLLER_H
#define SEARCHCONTROLLER_H

#include "dabstractfilecontroller.h"

#include <QSet>
#include <QPair>

class SearchFileWatcher;
class SearchController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit SearchController(QObject *parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const Q_DECL_OVERRIDE;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const Q_DECL_OVERRIDE;
    bool restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const Q_DECL_OVERRIDE;

    bool compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const Q_DECL_OVERRIDE;
    bool decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const Q_DECL_OVERRIDE;

    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const Q_DECL_OVERRIDE;

    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const Q_DECL_OVERRIDE;

    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

private:
    static DUrl realUrl(const DUrl &searchUrl);
    static DUrlList realUrlList(const DUrlList &searchUrls);

    friend class SearchDiriterator;
    friend class SearchFileWatcher;
};

#endif // SEARCHCONTROLLER_H
