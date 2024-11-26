#include "indextask.h"

#include <QDebug>
#include <QThread>

SERVICETEXTINDEX_USE_NAMESPACE

IndexTask::IndexTask(Type type, const QString &path, QObject *parent)
    : QObject(parent), m_type(type), m_path(path)
{
}

void IndexTask::start()
{
    if (m_running)
        return;

    m_running = true;
    m_status = Running;
    doTask();
}

void IndexTask::stop()
{
    m_running = false;
}

bool IndexTask::isRunning() const
{
    return m_running;
}

QString IndexTask::taskPath() const
{
    return m_path;
}

IndexTask::Type IndexTask::taskType() const
{
    return m_type;
}

IndexTask::Status IndexTask::status() const
{
    return m_status;
}

void IndexTask::doTask()
{
    // TODO: 实现具体的索引创建/更新逻辑
    // 这里只是示例代码
    bool success = true;
    for (int i = 0; i < 100 && m_running; ++i) {
        QThread::msleep(100);   // 模拟耗时操作
        // emit progress(m_type, m_path, i);
    }

    m_running = false;
    m_status = success ? Finished : Failed;
    //   emit finished(m_type, m_path, success);
}
