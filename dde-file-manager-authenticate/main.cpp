#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <iostream>

//for avodi deepin system pkexec interface bug
//绕开获取权限窗口为显卡字样 的bug

int main(int argc, char *argv[])
{
    if ( argc < 2 ||
        QString(argv[1]) == "--help" || \
        QString(argv[1]) == "--h" ||\
        QString(argv[1]) == "-h")
    {
        std::cout << "deepin-vault-authenticateProxy is a Proxy for replace polkit gui interface!" << std::endl;
        std::cout << "Usage: " << std::endl;
        std::cout << "\tdeepin-vault-authenticateProxy --help(--h): " << std::endl;
        std::cout << "\tpkexec deepin-vault-authenticateProxy [cmd]" << std::endl;

        std::cout << "Examples: " << std::endl;
        std::cout << "\tpkexec deepin-vault-authenticateProxy ls -l" << std::endl;
        std::cout << "\tpkexec deepin-vault-authenticateProxy dmidecode" << std::endl;

        return 0;
    }

    QString defaultLanguage = getenv("LANGUAGE");
    setenv ("LANGUAGE", "en_US", 1);    //for aviod translate in lscpu...

    QProcess proc;

    proc.start( argv[1] );
    bool res = proc.waitForFinished(-1);

    std::cout << proc.readAllStandardOutput().data();

    proc.close();

    if(res == false)
    {
        setenv ("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);
        return -1;
    }

    setenv ("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);
    return 0;
}
