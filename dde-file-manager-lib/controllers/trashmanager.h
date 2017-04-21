#ifndef TRASHMANAGER_H
#define TRASHMANAGER_H

#include "dabstractfilecontroller.h"

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QFileInfo>

class DAbstractFileInfo;
class FileMonitor;
class DFileWatcher;

class TrashManager : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit TrashManager(QObject *parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const Q_DECL_OVERRIDE;
    bool restoreFile(const QSharedPointer<DFMRestoreFromTrashEvent> &event) const Q_DECL_OVERRIDE;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList pasteFile(const QSharedPointer<DFMPasteEvent> &event) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

    static bool restoreTrashFile(const DUrlList &list);
    void cleanTrash(const QObject *sender = 0) const;

    static bool isEmpty();
public slots:
    void trashFilesChanged(const DUrl &url);
private:
    bool m_isTrashEmpty;
    DFileWatcher* m_trashFileWatcher;
};

#endif // TRASHMANAGER_H
