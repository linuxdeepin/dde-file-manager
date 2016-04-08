#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>
#include <QMap>

class DTaskDialog;
class FileJob;


class DialogManager : public QObject
{
    Q_OBJECT

public:
    explicit DialogManager(QObject *parent = 0);
    ~DialogManager();
    void initTaskDialog();
    void initConnect();
signals:

public slots:
    void handleDataUpdated();
    void handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response);
    void addJob(FileJob * job);
    void removeJob();
private:
    DTaskDialog* m_taskDialog = NULL;
    QMap<QString, FileJob*> m_jobs;
};

#endif // DIALOGMANAGER_H
