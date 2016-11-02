#ifndef TRASHMANAGER_H
#define TRASHMANAGER_H

#include "dabstractfilecontroller.h"

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QFileInfo>

class DAbstractFileInfo;
class FileMonitor;

class TrashManager : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit TrashManager(QObject *parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openFileLocation(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFilesToClipboard(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(PasteType type, const DUrl &targetUrl, const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool deleteFiles(const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                bool &accepted) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const Q_DECL_OVERRIDE;

    bool restoreTrashFile(const DUrlList &fileUrl, const DFMEvent &event) const;
    bool restoreAllTrashFile(const DFMEvent &event);
    void cleanTrash(const DFMEvent &event) const;

    static bool isEmpty();
};

#endif // TRASHMANAGER_H
