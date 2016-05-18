#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>
#include <QMap>

class DTaskDialog;
class FileJob;
class AbstractFileInfo;
class DUrl;
class FMEvent;

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

    void showTaskDialog();

    void abortJob(const QMap<QString, QString> &jobDetail);

    void showUrlWrongDialog(const DUrl &url);
    int showRenameNameSameErrorDialog(const QString& name, const FMEvent &event);
    int showDeleteFilesClearTrashDialog(const FMEvent &event);
    int showRemoveBookMarkDialog(const FMEvent &event);
    void showOpenWithDialog(const FMEvent &event);
    void showPropertyDialog(const FMEvent &event);
    void showDiskErrorDialog(const QString &id, const QString &errorText);
private:
    DTaskDialog* m_taskDialog = NULL;
    QMap<QString, FileJob*> m_jobs;
};

#endif // DIALOGMANAGER_H
