#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>

class DTaskDialog;


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

private:
    DTaskDialog* m_taskDialog = NULL;
};

#endif // DIALOGMANAGER_H
