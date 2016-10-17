#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>
#include <QMap>

#include "durl.h"

class DTaskDialog;
class FileJob;
class DAbstractFileInfo;
class DUrl;
class DFMEvent;
class PropertyDialog;
class CloseAllDialogIndicator;
class TrashPropertyDialog;
class QTimer;

class DialogManager : public QObject
{
    Q_OBJECT

public:
    explicit DialogManager(QObject *parent = 0);
    ~DialogManager();
    void initTaskDialog();
    void initCloseIndicatorDialog();
    void initConnect();
    QPoint getPerportyPos(int dialogWidth, int dialogHeight, int count, int index);
signals:

public slots:
    void handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response);
    void addJob(FileJob * job);
    void removeJob(const QString &jobId);
    void updateJob();
    void startUpdateJobTimer();
    void stopUpdateJobTimer();

    void abortJob(const QMap<QString, QString> &jobDetail);
    void abortJobByDestinationUrl(const DUrl& url);

    void showUrlWrongDialog(const DUrl &url);
    int showRunExcutableDialog(const DUrl& url);
    int showRenameNameSameErrorDialog(const QString& name, const DFMEvent &event);
    int showDeleteFilesClearTrashDialog(const DFMEvent &event);
    int showRemoveBookMarkDialog(const DFMEvent &event);
    void showOpenWithDialog(const DFMEvent &event);
    void showPropertyDialog(const DFMEvent &event);
    void showTrashPropertyDialog(const DFMEvent &event);
    void showDevicePropertyDialog(const DFMEvent &event);
    void showDiskErrorDialog(const QString &id, const QString &errorText);
    void showBreakSymlinkDialog(const QString &targetName, const DUrl& linkfile);
    void showAboutDialog(const DFMEvent &event);

    void removePropertyDialog(const DUrl& url);
    void closeAllPropertyDialog();
    void updateCloseIndicator();
    void raiseAllPropertyDialog();
    void handleFocusChanged(QWidget* old, QWidget* now);

    void refreshPropertyDialogs(const DUrl& oldUrl, const DUrl& newUrl);
private:
    DTaskDialog* m_taskDialog = NULL;
    CloseAllDialogIndicator* m_closeIndicatorDialog;
    TrashPropertyDialog* m_trashDialog;
    QMap<QString, FileJob*> m_jobs;
    QMap<DUrl, PropertyDialog*> m_propertyDialogs;
    QTimer* m_closeIndicatorTimer = NULL;
    QTimer* m_updateJobTaskTimer = NULL;
};

#endif // DIALOGMANAGER_H
