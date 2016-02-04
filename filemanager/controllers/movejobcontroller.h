#ifndef MOVEJOBCONTROLLER_H
#define MOVEJOBCONTROLLER_H


#include <QtCore>
#include <QtDBus>

class MoveJobInterface;
class MovejobWorker;

class MoveJobController : public QObject
{
    Q_OBJECT
public:
    explicit MoveJobController(QObject *parent = 0);
    ~MoveJobController();

    void initConnect();

signals:

public slots:
    void createMoveJob(QStringList files, QString destination);
    void handleConflictConfirmResponse(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);

private:
    QList<MovejobWorker*> m_works;
};

#endif // MOVEJOBCONTROLLER_H
