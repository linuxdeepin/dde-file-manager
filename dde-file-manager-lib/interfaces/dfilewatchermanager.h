#ifndef DFILEWATCHERMANAGER_H
#define DFILEWATCHERMANAGER_H

#include <QObject>

#include "dfmglobal.h"

class DFileWatcher;
DFM_BEGIN_NAMESPACE

class DFileWatcherManagerPrivate;
class DFileWatcherManager : public QObject
{
    Q_OBJECT

public:
    explicit DFileWatcherManager(QObject *parent = 0);
    ~DFileWatcherManager();

    DFileWatcher *add(const QString &filePath);
    void remove(const QString &filePath);

signals:
    void fileDeleted(const QString &filePath);
    void fileAttributeChanged(const QString &filePath);
    void fileMoved(const QString &fromFilePath, const QString &toFilePath);
    void subfileCreated(const QString &filePath);
    void fileModified(const QString &filePath);
    void fileClosed(const QString &filePath);

private:
    QScopedPointer<DFileWatcherManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFileWatcherManager)
    Q_DISABLE_COPY(DFileWatcherManager)
};

DFM_END_NAMESPACE

#endif // DFILEWATCHERMANAGER_H
