#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>


#include <shutil/danythingmonitor.h>

constexpr const char* const PATH{ "/proc/vfs_changes" };


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::shared_ptr<DAnythingMonitor> anything_monitor_ptr{ std::make_shared<DAnythingMonitor>() };
    std::function<void(const std::shared_ptr<DAnythingMonitor>& ptr)> signal_func{ &DAnythingMonitor::workSignal };
    std::function<void(const std::shared_ptr<DAnythingMonitor>& ptr)> work_func{ &DAnythingMonitor::doWork };

    while(true){

        if(!QFileInfo::exists(PATH)){
            std::this_thread::yield();
        }

        std::thread work_thread{work_func, anything_monitor_ptr};
        std::thread signal_thread{signal_func, anything_monitor_ptr};

        std::this_thread::sleep_for(std::chrono::duration<std::size_t, std::ratio<1, 1000>>{60});

        signal_thread.join();
        work_thread.join();
    }

    return app.exec();
}
