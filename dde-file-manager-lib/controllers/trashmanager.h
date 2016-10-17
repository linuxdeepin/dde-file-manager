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

    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;

    bool openFile(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool openFileLocation(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool addUrlMonitor(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    bool removeUrlMonitor(const DUrl &url, bool &accepted) const Q_DECL_OVERRIDE;
    bool copyFiles(const DUrlList &urlList, bool &accepted) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(PasteType type, const DUrlList &urlList,
                       const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    bool deleteFiles(const DUrlList &urlList, const DFMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                                QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                bool &accepted) const Q_DECL_OVERRIDE;

    bool restoreTrashFile(const DUrlList &fileUrl, const DFMEvent &event) const;
    bool restoreAllTrashFile(const DFMEvent &event);

    static bool isEmpty();

private slots:
    void onFileCreated(const QString &filePath) const;
    void onFileRemove(const QString &filePath) const;

private:
    mutable FileMonitor *fileMonitor = Q_NULLPTR;
};

#endif // TRASHMANAGER_H
