#ifndef MASTEREDMEDIACONTROLLER_H
#define MASTEREDMEDIACONTROLLER_H

#include "dabstractfilecontroller.h"
#include "durl.h"

class MasteredMediaController : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit MasteredMediaController(QObject *parent = 0);

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const Q_DECL_OVERRIDE;

    bool shareFolder(const QSharedPointer<DFMFileShareEvnet> &event) const Q_DECL_OVERRIDE;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const Q_DECL_OVERRIDE;
    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const Q_DECL_OVERRIDE;

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

    bool setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const Q_DECL_OVERRIDE;
    bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const Q_DECL_OVERRIDE;
    QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const Q_DECL_OVERRIDE;

private:
    void mkpath(DUrl path) const;

public:
    static DUrl getStagingFolder(DUrl dst);
};

#endif // MASTEREDMEDIACONTROLLER_H
