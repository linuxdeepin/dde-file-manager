#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>


#include <shutil/danythingmonitor.h>

constexpr const char* const PATH{ "/proc/vfs_changes" };


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DAnythingMonitor monitor{};

    while(true){

        if(!QFileInfo::exists(PATH)){
            std::this_thread::yield();
        }

        monitor.workSignal();
        monitor.doWork();
        std::this_thread::sleep_for(std::chrono::duration<std::size_t, std::ratio<1, 1000>>{500});
    }

    return app.exec();
}
