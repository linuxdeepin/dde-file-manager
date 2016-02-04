#ifndef RENAMEJOBCONTROLLER_H
#define RENAMEJOBCONTROLLER_H

#include <QObject>

class RenameJobInterface;

class RenameJobController : public QObject
{
    Q_OBJECT
public:
    explicit RenameJobController(QObject *parent = 0);
    ~RenameJobController();

    void initConnect();

signals:

public slots:
    void rename(QString url, QString newName);
    void renameJobExcuteFinished(QString message);

private:
    RenameJobInterface* m_renameJobInterface = NULL;
    QString m_newName;
};

#endif // RENAMEJOBCONTROLLER_H
