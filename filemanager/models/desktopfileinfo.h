#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include <QObject>
#include "fileinfo.h"

class DesktopFileInfo : public FileInfo
{
    Q_OBJECT
public:
    explicit DesktopFileInfo(QObject *parent = 0);
    ~DesktopFileInfo();

signals:

public slots:
};

#endif // DESKTOPFILEINFO_H
