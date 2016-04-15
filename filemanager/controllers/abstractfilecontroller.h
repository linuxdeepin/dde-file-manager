#ifndef ABSTRACTFILECONTROLLER_H
#define ABSTRACTFILECONTROLLER_H

#include <QObject>
#include <QDir>

#include "durl.h"

class AbstractFileInfo;
class FMEvent;

class AbstractFileController : public QObject
{
    Q_OBJECT

public:
    enum PasteType {
        CutType,
        CopyType
    };

    explicit AbstractFileController(QObject *parent = 0);

    virtual bool openFile(const DUrl &fileUrl, bool &accepted) const;
    virtual bool copyFiles(const DUrlList &urlList, bool &accepted) const;
    virtual bool renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const;
    virtual bool deleteFiles(const DUrlList &urlList, bool &accepted) const;
    virtual bool moveToTrash(const DUrlList &urlList, bool &accepted) const;
    virtual bool cutFiles(const DUrlList &urlList, bool &accepted) const;
    virtual bool pasteFile(PasteType type, const DUrlList &urlList,
                           const FMEvent &event, bool &accepted) const;
    virtual bool newFolder(const DUrl &toUrl, bool &accepted) const;
    virtual bool newFile(const DUrl &toUrl, bool &accepted) const;
    virtual bool newDocument(const DUrl &toUrl, bool &accepted) const;

    virtual bool addUrlMonitor(const DUrl &url, bool &accepted) const;
    virtual bool removeUrlMonitor(const DUrl &url, bool &accepted) const;

    virtual bool openFileLocation(const DUrl &fileUrl, bool &accepted) const;

    virtual const QList<AbstractFileInfo*> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const;
    virtual AbstractFileInfo *createFileInfo(const DUrl &fileUrl, bool &accepted) const;

signals:
    void childrenAdded(const DUrl &fileUrl) const;
    void childrenRemoved(const DUrl &fileUrl) const;
};

#endif // ABSTRACTFILECONTROLLER_H
