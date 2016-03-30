#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include <QThread>
#include "../controllers/filejob.h"
#include <QDebug>

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{

}

WindowManager::~WindowManager()
{

}

void WindowManager::showNewWindow()
{
    DFileManagerWindow* window = new DFileManagerWindow;
    window->show();
    m_windows.insert(m_windows.count() + 1, window);
//    FileJob * job = new FileJob;
//    QThread * thread = new QThread;
//    job->moveToThread(thread);
//    connect(this, &WindowManager::start, job, &FileJob::doMoveToTrash);
//    connect(job, &FileJob::progressPercent, this, &WindowManager::progressPercent);
//    connect(job, &FileJob::result, this, &WindowManager::result);
//    connect(job, &FileJob::error, this, &WindowManager::error);
//    thread->start();
//    QThread::sleep(1);
//    emit start("/home/wang/你好");
}

void WindowManager::progressPercent(int value)
{
    qDebug() << "progress = " << value;
}

void WindowManager::error(QString content)
{
    qDebug() << "error = " << content;
}

void WindowManager::result(QString content)
{
    qDebug() << "result = " << content;
}
