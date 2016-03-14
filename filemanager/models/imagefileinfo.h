#ifndef IMAGEFILEINFO_H
#define IMAGEFILEINFO_H

#include <QObject>
#include "fileinfo.h"

class ImageFileInfo : public FileInfo
{
    Q_OBJECT
public:
    explicit ImageFileInfo(QObject *parent = 0);
    ~ImageFileInfo();

signals:

public slots:
};

#endif // IMAGEFILEINFO_H
