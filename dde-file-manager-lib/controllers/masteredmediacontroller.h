#ifndef MASTEREDMEDIACONTROLLER_H
#define MASTEREDMEDIACONTROLLER_H

#include "dabstractfilecontroller.h"
#include "dabstractfilewatcher.h"
#include "durl.h"

class MasteredMediaFileWatcherPrivate;
class MasteredMediaFileWatcher : public DAbstractFileWatcher
{
    Q_OBJECT

public:
    explicit MasteredMediaFileWatcher(const DUrl &url, QObject *parent = nullptr);

private slots:
    void onFileDeleted(const DUrl &url);
    void onFileAttributeChanged(const DUrl &url);
    void onFileMoved(const DUrl &fromUrl, const DUrl &toUrl);
    void onSubfileCreated(const DUrl &url);

private:
    Q_DECLARE_PRIVATE(MasteredMediaFileWatcher)
};

class MasteredMediaController : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit MasteredMediaController(QObject *parent = nullptr);

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool compressFiles(const QSharedPointer<DFMCompressEvent> &event) const Q_DECL_OVERRIDE;
    bool decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const Q_DECL_OVERRIDE;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const Q_DECL_OVERRIDE;

    bool shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const Q_DECL_OVERRIDE;
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const Q_DECL_OVERRIDE;

    bool addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const override;
    bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const override;

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

public:
    static DUrl getStagingFolder(DUrl dst);
    static QFileDevice::Permissions getPermissionsCopyToLocal();
};

#endif // MASTEREDMEDIACONTROLLER_H
