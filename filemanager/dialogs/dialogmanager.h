#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>
#include <QMap>

class DTaskDialog;
class FileJob;
class AbstractFileInfo;
class DUrl;

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
    void handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response);
    void addJob(FileJob * job);
    void removeJob(const QString &jobId);

    void abortJob(const QMap<QString, QString> &jobDetail);

    void showUrlWrongDialog(const DUrl &url);
    void showOpenWithDialog(const DUrl &url);
    void showPropertyDialog(const DUrl &url);

private:
    DTaskDialog* m_taskDialog = NULL;
    QMap<QString, FileJob*> m_jobs;
};

#endif // DIALOGMANAGER_H
