#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QApplication>

#define FreePointer(x) if(x){delete x;x = nullptr;}

class TestHelper
{
public:
    TestHelper();

    static QString createTmpSymlinkFile(const QString &src) {
        int index = src.lastIndexOf("/");
        QString symFilePath(src);
        symFilePath.insert(index + 1, "sym_");

        QProcess::execute("ln -s " + src + " " + symFilePath);
        return symFilePath;
    }

    static QString createTmpFileName(QString name, QString dirPath = "/tmp") {
        QString tempFilePath =  dirPath + "/" + name;
        QProcess::execute("touch " + tempFilePath);
        return tempFilePath;
    }

    static QString createTmpFile(QString suffix = "", QStandardPaths::StandardLocation location = QStandardPaths::TempLocation) {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        int random = qrand()%1000;
        QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) +  QString::number(random) + suffix;
        QString tempFilePath =  QStandardPaths::standardLocations(location).first() + "/" + fileName;
        QProcess::execute("touch " + tempFilePath);
        return tempFilePath;
    }

    static QString createTmpDir() {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        int random = qrand()%1000;
        QString dirName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) + QString::number(random);
        QString tempFilePath =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + dirName;
        QProcess::execute("mkdir " + tempFilePath);
        return tempFilePath;
    }

    static void deleteTmpFiles(QStringList paths) {
        for(QString path : paths) {
            deleteTmpFile(path);
        }
    }
    static void deleteTmpFile(QString path) {
         QProcess::execute("rm -rf  " + path);
    }
    static void rename(QString fromPath, QString toPath) {
         QProcess::execute(QString("mv %1 %2").arg(fromPath).arg(toPath));
    }
    template <typename Fun>
    static void runInLoop(Fun fun, int msc = 2000)
    {
        QEventLoop loop;
        QTimer timer;
        timer.start(msc);
        fun();
        QObject::connect(&timer, &QTimer::timeout, [&]{
            timer.stop();
            loop.exit();
        });
        loop.exec();
        qApp->processEvents();
    }

};

#endif // TESTHELPER_H
