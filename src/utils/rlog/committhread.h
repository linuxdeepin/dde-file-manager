#ifndef COMMITTHREAD_H
#define COMMITTHREAD_H

#include <QObject>
#include <QLibrary>

class CommitLog : public QObject
{
    Q_OBJECT
public:
    using InitEventLog = bool (*)(const std::string &, bool);
    using WriteEventLog = void (*)(const std::string &);

    explicit CommitLog(QObject *parent = nullptr);
    ~CommitLog();
public slots:
    void commit(const QVariant &args);
    bool init();

private:
    QLibrary m_library;
    InitEventLog m_initEventLog = nullptr;
    WriteEventLog m_writeEventLog = nullptr;
};

#endif   // COMMITTHREAD_H
