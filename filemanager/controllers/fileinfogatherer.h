#ifndef FILEINFOGATHERER_H
#define FILEINFOGATHERER_H

#include <QDir>

class FileInfo;

class FileInfoGatherer : public QObject
{
    Q_OBJECT
public:
    explicit FileInfoGatherer(QObject *parent = 0);

public slots:
    void fetchFileInformation(const QString &path,
                              int filter = int(QDir::AllEntries | QDir::NoDotDot));

signals:
    void updates(const QString &directory, const QList<FileInfo*> &infoList);
    void addFileInfo(const QString &path, FileInfo* info);
};

#endif // FILEINFOGATHERER_H
