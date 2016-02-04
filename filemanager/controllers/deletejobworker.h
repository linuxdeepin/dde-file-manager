#ifndef DELETEJOBWORKER_H
#define DELETEJOBWORKER_H

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QMap>
#include <QString>
class DeleteJobInterface;


class DeletejobWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeletejobWorker(const QStringList& files, QObject *parent = 0);
    ~DeletejobWorker();
    void initConnect();

signals:
    void startJob();
    void finished();

public slots:
    void start();
    void deleteFiles(const QStringList& files);
    void connectDeleteJobSignal();
    void disconnectDeleteJobSignal();
    void deleteJobExcuteFinished();
    void deleteJobAbort();
    void deleteJobAbortFinished();
    void onDeletingFile(QString file);
    void setTotalAmount(qlonglong amount, ushort type);
    void onDeletingProcessAmount(qlonglong progress, ushort info);
    void onProcessedPercent(qlonglong percent);
    void handleTimeout();
    void handleFinished();
    void handleTaskAborted(const QMap<QString, QString>& jobDetail);

private:
    DeleteJobInterface* m_deleteJobInterface = NULL;
    QStringList m_deletefiles;

    QString m_deletejobPath;
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

#endif // DELETEJOBWORKER_H
