#ifndef DISKGLOBAL_H
#define DISKGLOBAL_H
#include <QObject>
#include <dfmglobal.h>

class DiskGlobal : public QObject
{
public:
    explicit DiskGlobal(QObject *parent = nullptr);

    static bool isWayLand();
};

#endif // DFMGLOBAL_H
