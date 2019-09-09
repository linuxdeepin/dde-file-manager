#ifndef DFMROOTCONTROLLER_H
#define DFMROOTCONTROLLER_H

#include "dabstractfilecontroller.h"
#include "dabstractfilewatcher.h"
#include "durl.h"
#include <dgiomount.h>

class DFMRootFileWatcherPrivate;
class DFMRootFileWatcher : public DAbstractFileWatcher
{
    Q_OBJECT

public:
    explicit DFMRootFileWatcher(const DUrl &url, QObject *parent = nullptr);

private:
    Q_DECLARE_PRIVATE(DFMRootFileWatcher)
};

class DFMRootController : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit DFMRootController(QObject *parent = nullptr);
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;

    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;
};

#endif // DFMROOTCONTROLLER_H
