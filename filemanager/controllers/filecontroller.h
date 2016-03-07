#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include <QObject>
#include <QMap>

#include "dbusinterface/dbustype.h"

class ListJobInterface;

class FileController : public QObject
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = 0);
    ~FileController();

    void initConnect();

public slots:
    void getChildren(const QString &url);
    void getIcon(const QString &url) const;

private:
    QMap<QString, FileItemInfo> m_fileItemInfos;
};

#endif // FILECONTROLLER_H
