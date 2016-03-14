#ifndef SEARCHHISTORY_H
#define SEARCHHISTORY_H

#include <QObject>
#include "utils/debugobejct.h"

class SearchHistory : public DebugObejct
{
    Q_OBJECT
public:
    explicit SearchHistory(QObject *parent = 0);
    ~SearchHistory();

signals:

public slots:
};

#endif // SEARCHHISTORY_H
