#include <QDebug>
#include <QCoreApplication>


#include <shutil/danythingmonitor.h>

constexpr const char* const PATH{ "/proc/vfs_changes" };


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    std::shared_ptr<DAnythingMonitor> sharedPtr{ std::make_shared<DAnythingMonitor>(PATH) };

    while(true){
        std::function<void(std::shared_ptr<DAnythingMonitor> thisPtr)> workSignal{ &DAnythingMonitor::workSignal };
        std::function<void(std::shared_ptr<DAnythingMonitor> thisPtr)> doWork{ &DAnythingMonitor::doWork };

        std::thread theSignal{ workSignal, sharedPtr->shared_from_this() };
        std::thread theWorker{ doWork, sharedPtr->shared_from_this() };

        theSignal.join();
        theWorker.join();

        std::this_thread::sleep_for(std::chrono::duration<std::size_t, std::ratio<1, 1000>>{200});
    }

    return app.exec();
}
