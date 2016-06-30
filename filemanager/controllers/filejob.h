#ifndef FILEJOB_H
#define FILEJOB_H

#include <QObject>
#include <QMap>
#include <QElapsedTimer>
#include <QUrl>
#include "../models/durl.h"
#include <QStorageInfo>

#define TRANSFER_RATE 5
#define MSEC_FOR_DISPLAY 1000
#define DATA_BLOCK_SIZE 65536
#define ONE_MB_SIZE 1048576
#define ONE_KB_SIZE 1024

class FileJob : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        Started,
        Paused,
        Cancelled,
        Run,
        Conflicted,
    };
    static QMap<DUrl, int> SelectedFiles;
    void setStatus(Status status);
    explicit FileJob(const QString &title, QObject *parent = 0);
    void setJobId(const QString &id);
    QString getJobId();
    QString checkDuplicateName(const QString &name);
    void setApplyToAll(bool v);
    void setReplace(bool v);
signals:
    void progressPercent(int value);
    void error(QString content);
    void result(QString content);
    void finished();
public slots:
    void doCopy(const QList<QUrl> &files, const QString &destination);
    void doDelete(const QList<QUrl> &files);
    void doMoveToTrash(const QList<QUrl> &files);
    void doMove(const QList<QUrl> &files, const QString &destination);
    void paused();
    void started();
    void cancelled();
    void handleJobFinished();
private:
    Status m_status;
    QString m_trashLoc;
    QString m_id;
    QMap<QString, QString> m_jobDetail;
    qint64 m_bytesCopied;
    qint64 m_totalSize;
    double m_bytesPerSec;
    float m_factor;
    bool m_isJobAdded = false;
    QString m_srcFileName;
    QString m_tarFileName;
    QString m_srcPath;
    QString m_tarPath;
    QElapsedTimer m_timer;
    qint64 lastMsec;
    qint64 currentMsec;
    bool m_applyToAll = false;
    bool m_isReplaced = false;
    QString m_title;

    void jobUpdated();
    void jobAdded();
    void jobRemoved();
    void jobAborted();
    void jobPrepared();
    void jobConflicted();

    bool copyFile(const QString &srcFile, const QString &tarDir);
    bool copyDir(const QString &srcPath, const QString &tarPath);
    bool moveFile(const QString &srcFile, const QString &tarDir);
    bool moveDir(const QString &srcFile, const QString &tarDir);
    bool deleteFile(const QString &file);
    bool deleteDir(const QString &dir);
    bool moveDirToTrash(const QString &dir);
    bool moveFileToTrash(const QString &file);
    bool writeTrashInfo(const QString &fileBaseName, const QString &path, const QString &time);

    QString getNotExistsTrashFileName(QString fileName);
};

#endif // FILEJOB_H
