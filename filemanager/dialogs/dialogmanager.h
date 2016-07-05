#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>
#include <QMap>
#include "../models/durl.h"
class DTaskDialog;
class FileJob;
class AbstractFileInfo;
class DUrl;
class FMEvent;
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
    void showTaskDialog();
    void abortJob(const QMap<QString, QString> &jobDetail);

    void showUrlWrongDialog(const DUrl &url);
    int showRunExcutableDialog(const DUrl& url);
    int showRenameNameSameErrorDialog(const QString& name, const FMEvent &event);
    int showDeleteFilesClearTrashDialog(const FMEvent &event);
    int showRemoveBookMarkDialog(const FMEvent &event);
    void showOpenWithDialog(const FMEvent &event);
    void showPropertyDialog(const FMEvent &event);
    void showTrashPropertyDialog(const FMEvent &event);
    void showDevicePropertyDialog(const FMEvent &event);
    void showDiskErrorDialog(const QString &id, const QString &errorText);
    void showAboutDialog(const FMEvent &event);

    void removePropertyDialog(const DUrl& url);
    void closeAllPropertyDialog();
    void updateCloseIndicator();
    void raiseAllPropertyDialog();
    void handleFocusChanged(QWidget* old, QWidget* now);
private:
    DTaskDialog* m_taskDialog = NULL;
    CloseAllDialogIndicator* m_closeIndicatorDialog;
    TrashPropertyDialog* m_trashDialog;
    QMap<QString, FileJob*> m_jobs;
    QMap<DUrl, PropertyDialog*> m_propertyDialogs;
    QTimer* m_closeIndicatorTimer = NULL;

};

#endif // DIALOGMANAGER_H
