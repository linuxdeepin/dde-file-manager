#ifndef COPYJOBWORKER_H
#define COPYJOBWORKER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QString>
#include <QTime>
#include <QMap>

class CopyJobInterface;
class FileConflictController;


class CopyjobWorker : public QObject
{
    Q_OBJECT
public:
    explicit CopyjobWorker(QStringList files, QString destination, QObject *parent = 0);
    void initConnect();

    QStringList getFiles();
    QString getDestination();
    QString getJobPath();
    const QMap<QString, QString>& getJobDetail();
    FileConflictController* getFileConflictController();

signals:
    void startJob();
    void finished();

public slots:
    void start();
    void copyFiles(QStringList files, QString destination);

    void connectCopyJobSignal();
    void disconnectCopyJobSignal();
    void copyJobExcuteFinished(QString file);
    void copyJobAbort();
    void copyJobAbortFinished();
    void onCopyingFile(QString file);
    void setTotalAmount(qlonglong amount, ushort type);
    void onCopyingProcessAmount(qlonglong progress, ushort type);
    void onProcessedPercent(qlonglong percent);
    void handleTimeout();
    void handleFinished();
    void handleTaskAborted(const QMap<QString, QString>& jobDetail);
    void stopTimer();
    void restartTimer();
private:
    QStringList m_files;
    QString m_destination;
    QString m_copyjobPath;
    CopyJobInterface* m_copyJobInterface = NULL;
    FileConflictController* m_conflictController = NULL;
    QTimer* m_progressTimer;
    QMap<QString, QString> m_jobDetail;
    QMap<QString, QString> m_jobDataDetail;
    qlonglong m_totalAmout;
    QTime* m_time;
    qlonglong m_lastProgress = 0;
    qlonglong m_currentProgress = 0;
    qlonglong m_processedPercent = 0;
    int m_elapsedTime;
};

#endif // COPYJOBWORKER_H
