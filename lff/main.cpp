// g++ -fPIC --std=c++11 `pkg-config --cflags --libs Qt5Core Qt5Widgets` -o run main.cpp

#include <QApplication>
#include <QStandardPaths>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QTime>
#include <QProcess>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QProcess::startDetached("gsettings set org.mate.background show-desktop-icons false");

    QProcess::startDetached("dde-desktop");

    const int warnCount = 110;
    const int warnSkipTime = 60;
    QTime lastWarnDialogShownTime = QTime::currentTime().addSecs(-1 - warnSkipTime);
    QString warnTitle("整理桌面提醒"), warnContent("您的桌面上的文件太多了，有可能会导致桌面图标重叠，请考虑整理一下您的桌面。");
    QString btn0Text("好的"), btn1Text("本次启动不再提醒");
    QString desktopPathStr = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QDir desktopDir(desktopPathStr);
    desktopDir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    int entryCount = desktopDir.count();

    if (entryCount > warnCount) {
        lastWarnDialogShownTime = QTime::currentTime();
        int ret = QMessageBox::warning(nullptr, warnTitle, warnContent, btn0Text, btn1Text);
        lastWarnDialogShownTime = QTime::currentTime();
        if (ret == 1) {
            quick_exit(0);
        }
    }

    QFileSystemWatcher liangfeifan({ desktopPathStr });
    QObject::connect(&liangfeifan, &QFileSystemWatcher::directoryChanged,
                     [ =, &desktopDir, &entryCount, &a, &lastWarnDialogShownTime ](){
        QDir desktopDir(desktopPathStr);
        desktopDir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
        int newCount = desktopDir.count();
        if (newCount > entryCount && newCount > warnCount
                && lastWarnDialogShownTime.secsTo(QTime::currentTime()) > warnSkipTime) {
            lastWarnDialogShownTime = QTime::currentTime();
            int ret = QMessageBox::warning(nullptr, warnTitle, warnContent, btn0Text, btn1Text);
            lastWarnDialogShownTime = QTime::currentTime();
            if (ret == 1) {
                a.exit();
            }
        }
        entryCount = newCount;
    });

    return a.exec();
}
