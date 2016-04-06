#ifndef FILEJOB_H
#define FILEJOB_H

#include <QObject>

#define TRANSFER_RATE 5
#define MSEC_FOR_DISPLAY 1000

class FileJob : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        Started,
        Paused,
        Cancelled,
    };

    explicit FileJob(QObject *parent = 0);
    void setJobId(const QString &id);
    QString getJobId();
signals:
    void progressPercent(int value);
    void error(QString content);
    void result(QString content);
    void finished();
public slots:
    void doCopy(const QString &source, const QString &destination);
    void doDelete(const QString &source);
    void doMoveToTrash(const QString &source);
    void paused();
    void started();
    void cancelled();
    void jobUpdated();
private:
    Status m_status;
    QString m_trashLoc;
    QString m_id;
    bool copyFile(const QString &srcFile, const QString &tarFile);
    bool copyDir(const QString &srcPath, const QString &tarPath);
    bool deleteFile(const QString &file);
    bool deleteDir(const QString &dir);
    bool moveDirToTrash(const QString &dir);
    bool moveFileToTrash(const QString &file);
    void writeTrashInfo(const QString &name, const QString &path, const QString &time);
    QString baseName( QString path );
};

#endif // FILEJOB_H
