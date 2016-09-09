#ifndef FMSTATEMANAGER_H
#define FMSTATEMANAGER_H

#include "basemanager.h"

#include <QObject>
#include "models/fmstate.h"
#include "durl.h"


class FMStateManager : public QObject, public BaseManager
{
    Q_OBJECT

public:
    static QMap<DUrl, QPair<int, int>> SortStates;

    explicit FMStateManager(QObject *parent = 0);
    ~FMStateManager();

    void initConnect();

    static QString cacheFilePath();
    static QString sortCacheFilePath();

    FMState *fmState() const;
    void setFmState(FMState *fmState);

signals:

public slots:
    void loadCache();
    void saveCache();
    static void loadSortCache();
    static void saveSortCache();
    static void cacheSortState(const DUrl& url, int role, Qt::SortOrder order);

private:
    FMState *m_fmState = NULL;
};

#endif // FMSTATEMANAGER_H
