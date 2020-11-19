#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QProcess>
#include <QStandardPaths>
#include <QDateTime>
#include <QUrl>
#include <QFile>

class TestHelper
{
public:
    TestHelper();

    static QString createTmpFile(QString suffix = "") {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        int random = qrand()%1000;
        QString fileName = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) +  QString::number(random) + suffix;
        QString tempFilePath =  QStandardPaths::standardLocations(QStandardPaths::TempLocation).first() + "/" + fileName;
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
};

#endif // TESTHELPER_H
