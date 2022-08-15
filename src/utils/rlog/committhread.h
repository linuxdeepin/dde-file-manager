#ifndef COMMITTHREAD_H
#define COMMITTHREAD_H

#include <QObject>

class CommitLog : public QObject
{
    Q_OBJECT
public:
    explicit CommitLog(QObject *parent = nullptr);
    ~CommitLog();
public slots:
    void commit(const QVariant& args);
    void init();
private:
    bool m_isInitialized = false;
};

#endif // COMMITTHREAD_H
