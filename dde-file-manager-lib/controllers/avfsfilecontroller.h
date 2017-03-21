#ifndef AVFSFILECONTROLLER_H
#define AVFSFILECONTROLLER_H

#include <QObject>

#include "dabstractfilecontroller.h"

class AVFSFileController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit AVFSFileController(QObject* parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;

    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const Q_DECL_OVERRIDE;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const Q_DECL_OVERRIDE;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const Q_DECL_OVERRIDE;
    bool openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const Q_DECL_OVERRIDE;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const Q_DECL_OVERRIDE;

    bool openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const Q_DECL_OVERRIDE;

    static DUrl realUrl(const DUrl& url);
    static QString findArchFileRootPath(const DUrl& url);
private:
};

#endif // AVFSFILECONTROLLER_H
