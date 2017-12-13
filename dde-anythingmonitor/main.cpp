
#include <QCoreApplication>

#include <shutil/danythingmonitor.h>

constexpr const char* const PATH{ "/proc/vfs_changes" };


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DAnythingMonitor monitor{ PATH, nullptr };

    return app.exec();
}
