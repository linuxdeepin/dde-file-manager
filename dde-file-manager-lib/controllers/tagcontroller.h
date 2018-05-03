#ifndef TAGCONTROLLER_H
#define TAGCONTROLLER_H


#include "dabstractfilecontroller.h"

class TagController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit TagController(QObject* const parent = nullptr);
    ~TagController()=default;

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet>& event) const override;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const override;
    bool compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const override;
    bool decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const override;
    bool decompressFileHere(const QSharedPointer<DFMDecompressEvnet> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;

    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const override;
    bool createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const override;
    bool shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const override;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const;

    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const override;
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const override;
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const override;
};


#endif // TAGCONTROLLER_H
