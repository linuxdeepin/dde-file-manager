#ifndef JOBCONTROLLER_H
#define JOBCONTROLLER_H

#include "ddiriterator.h"

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QQueue>
#include <QDir>

class JobController : public QThread
{
    Q_OBJECT

public:
    enum State {
        Started,
        Paused,
        Stoped
    };

    explicit JobController(const DDirIteratorPointer &iterator, QObject *parent = 0);
    explicit JobController(const DUrl &fileUrl, QDir::Filters filters, QObject *parent = 0);

    State state() const;

public slots:
    void start();
    void pause();
    void stop();
    void stopAndDeleteLater();

signals:
    void stateChanged(State state);
    void addChildren(const AbstractFileInfoPointer &info);
    void childrenUpdated(const QList<AbstractFileInfoPointer> &list);
    void finished();

private:
    DDirIteratorPointer m_iterator;
    DUrl m_fileUrl;
    QDir::Filters m_filters;

    State m_state = Stoped;
    QWaitCondition waitCondition;
    QMutex mutex;

    bool autoDestroy = false;

    void run() Q_DECL_OVERRIDE;
    void setState(State state);
};

#endif // JOBCONTROLLER_H
