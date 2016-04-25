#ifndef FMSORTMANAGER_H
#define FMSORTMANAGER_H

#include <QObject>

#include "basemanager.h"
#include "../models/durl.h"
#include "../app/global.h"


class fmsortmanager : public QObject, public BaseManager
{
    Q_OBJECT
public:
    explicit fmsortmanager(QObject *parent = 0);
    ~fmsortmanager();

    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;

signals:

public slots:
    void saveLastSortOrder(const DUrl& url, Global::SortRole role);
};

#endif // FMSORTMANAGER_H
