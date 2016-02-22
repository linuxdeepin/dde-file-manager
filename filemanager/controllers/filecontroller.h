#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include "dbusinterface/dbustype.h"
class ListJobInterface;

class FileController : public QObject
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = 0);
    void initConnect();

public slots:
    void getChildren(const QUrl &url);
    void getIcon(const QUrl &url) const;

private:
    QMap<QUrl, ListJobInterface*> m_urlToListJobInterface;
    QMap<QString, FileItemInfo> m_fileItemInfos;
};

#endif // FILECONTROLLER_H
