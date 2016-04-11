#ifndef ABSTRACTFILECONTROLLER_H
#define ABSTRACTFILECONTROLLER_H

#include <QObject>
#include <QDir>

class AbstractFileInfo;

class AbstractFileController : public QObject
{
    Q_OBJECT

public:
    explicit AbstractFileController(QObject *parent = 0);

    virtual bool openFile(const QString &fileUrl, bool &accepted) const;
    virtual bool renameFile(const QString &oldFile, const QString &newFile, bool &accepted) const;
    virtual bool addUrlMonitor(const QString &url, bool &accepted) const;
    virtual bool removeUrlMonitor(const QString &url, bool &accepted) const;

    virtual const QList<AbstractFileInfo*> getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const;
    virtual AbstractFileInfo *createFileInfo(const QString &fileUrl, bool &accepted) const;

signals:
    void childrenAdded(const QString &fileUrl);
    void childrenRemoved(const QString &fileUrl);
};

#endif // ABSTRACTFILECONTROLLER_H
