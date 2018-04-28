#ifndef TAGCONTROLLER_H
#define TAGCONTROLLER_H


#include "dabstractfilecontroller.h"

class TagController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit TagController(QObject* const parent = nullptr);
    virtual ~TagController()=default;

    virtual const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet>& event) const override;
    virtual const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;

    virtual DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    virtual bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const;
    virtual bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const;

    virtual bool makeFileTags(const QSharedPointer<DFMMakeFileTagsEvent> &event) const override;
    virtual bool removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const override;
    virtual QList<QString> getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const override;
};


#endif // TAGCONTROLLER_H
