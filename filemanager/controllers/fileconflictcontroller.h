#ifndef FILECONFLICTCONTROLLER_H
#define FILECONFLICTCONTROLLER_H

#include <QObject>
#include <QString>
#include <dbusinterface/services/conflictdaptor.h>
#include <dbusinterface/dbustype.h>

class FileConflictController : public QObject
{
    Q_OBJECT
public:
    explicit FileConflictController(QObject *parent = 0);
    ~FileConflictController();
    void registerDBusService();
    void unRegisterDBusService();
    static int count;

    QString getObjectPath();
    ConflictAdaptor* getConflictAdaptor();

signals:

public slots:
    ConflictInfo AskDelete(const QString &primaryText, const QString &secondaryText, const QString &detailText, bool retry, bool multi);
    bool AskDeleteConfirmation(const QString &primaryText, const QString &secondaryText, const QString &detailText);
    ConflictInfo AskRetry(const QString &primaryText, const QString &secondaryText, const QString &detailText);
    ConflictInfo AskSkip(const QString &primaryText, const QString &secondaryText, const QString &detailText, bool retry, bool multi);
    ConflictInfo ConflictDialog();

    void setJobDetail(const QMap<QString, QString>& detail);

public slots:

private:
    QMap<QString, QString> m_jobDetail;
    QString m_objectPath;
    ConflictAdaptor* m_conflictAdaptor=NULL;
};

#endif // FILECONFLICTCONTROLLER_H
