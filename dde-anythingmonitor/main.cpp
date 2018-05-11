#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>
#include <QtConcurrent>

#include <interfaces/dfmglobal.h>
#include <shutil/danythingmonitor.h>

constexpr const char* const PATH{ "/proc/vfs_changes" };

void do_work()
{
    QScopedPointer<DAnythingMonitor> anything_monitor_ptr{ new DAnythingMonitor{} };
    QScopedPointer<QThreadPool> thread_pool_ptr{ new QThreadPool };

    thread_pool_ptr->setMaxThreadCount(2);

    while(true){

        if(!QFileInfo::exists(PATH)){
            std::this_thread::yield();
        }

        QFuture<void> work_future{ QtConcurrent::run(thread_pool_ptr.data() ,anything_monitor_ptr.data(), &DAnythingMonitor::doWork) };
        QFuture<void> signal_future{ QtConcurrent::run(thread_pool_ptr.data(), anything_monitor_ptr.data(), &DAnythingMonitor::workSignal) };

        QThread::msleep(200);

        work_future.waitForFinished();
        signal_future.waitForFinished();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};
    QtConcurrent::run(&do_work);

    return app.exec();
}
