#ifndef RENAMEJOB_H
#define RENAMEJOB_H

#include <QObject>

class RenameJob : public QObject
{
    Q_OBJECT
public:
    explicit RenameJob(QObject *parent = 0);
    ~RenameJob();

signals:
    void Done(const QString& message);

public slots:
    void Execute();
};

#endif // RENAMEJOB_H
