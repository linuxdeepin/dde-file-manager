#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>
#include "utils/debugobejct.h"

class BookMark : public DebugObejct
{
    Q_OBJECT
public:
    explicit BookMark(QObject *parent = 0);
    ~BookMark();

signals:

public slots:

};

#endif // BOOKMARK_H
