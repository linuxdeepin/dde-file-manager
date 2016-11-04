#ifndef ABSTRACTFILECONTROLLER_H
#define ABSTRACTFILECONTROLLER_H

#include "dabstractfileinfo.h"
#include "ddiriterator.h"

#include <QObject>
#include <QDir>
#include <QDirIterator>

class DFMEvent;
class DUrl;
typedef QList<DUrl> DUrlList;
class DAbstractFileController : public QObject
{
    Q_OBJECT

public:
    enum PasteType {
        CutType,
        CopyType
    };

    explicit DAbstractFileController(QObject *parent = 0);

    virtual bool openFile(const DUrl &fileUrl, bool &accepted) const;
    virtual bool openFileByApp(const DUrl &fileUrl, const QString& app, bool &accepted) const;
    virtual bool compressFiles(const DUrlList &urlList, bool &accepted) const;
    virtual bool decompressFile(const DUrlList &fileUrlList, bool &accepted) const;
    virtual bool decompressFileHere(const DUrlList &fileUrlList, bool &accepted) const;
    virtual bool copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const;
    virtual bool renameFile(const DUrl &oldUrl, const DUrl &newUrl, bool &accepted) const;
    virtual bool deleteFiles(const DFMEvent &event, bool &accepted) const;
    virtual DUrlList moveToTrash(const DFMEvent &event, bool &accepted) const;
    virtual bool cutFilesToClipboard(const DUrlList &urlList, bool &accepted) const;
    virtual DUrlList pasteFile(PasteType type, const DUrl &targetUrl, const DFMEvent &event, bool &accepted) const;
    virtual bool restoreFile(const DUrl &srcUrl, const DUrl &tarUrl, const DFMEvent &event, bool &accepted) const;
    virtual bool newFolder(const DFMEvent &event, bool &accepted) const;
    virtual bool newFile(const DUrl &toUrl, bool &accepted) const;
    virtual bool newDocument(const DUrl &toUrl, bool &accepted) const;

    virtual bool addUrlMonitor(const DUrl &url, bool &accepted) const;
    virtual bool removeUrlMonitor(const DUrl &url, bool &accepted) const;

    virtual bool openFileLocation(const DUrl &fileUrl, bool &accepted) const;

    virtual const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                             QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                             bool &accepted) const;
    virtual const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const;
    virtual const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                        QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                        bool &accepted) const;

    virtual bool createSymlink(const DUrl &fileUrl, const DUrl &linkToUrl, bool &accepted) const;
    virtual bool openInTerminal(const DUrl &fileUrl, bool &accepted) const;

signals:
    void childrenAdded(const DUrl &fileUrl) const;
    void childrenRemoved(const DUrl &fileUrl) const;
    void childrenUpdated(const DUrl &fileUrl) const;
};

#endif // ABSTRACTFILECONTROLLER_H
