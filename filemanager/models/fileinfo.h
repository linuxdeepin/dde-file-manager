#ifndef FILEINFO_H
#define FILEINFO_H

#include <QObject>
#include "utils/debugobejct.h"

class FileInfo : public DebugObejct
{
    Q_OBJECT
public:
    explicit FileInfo(QObject *parent = 0);
    ~FileInfo();

signals:

public slots:
};

#endif // FILEINFO_H
