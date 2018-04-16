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


    virtual bool makeFilesTags(const QSharedPointer<DFMMakeFilesTagsEvent>& event) const override;
    virtual bool removeTagsOfFiles(const QSharedPointer<DFMRemoveTagsOfFilesEvent>& event) const override;
};


#endif // TAGCONTROLLER_H
