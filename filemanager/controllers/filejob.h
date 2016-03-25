#ifndef FILEJOB_H
#define FILEJOB_H

#include <QObject>

class FileJob : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        Started,
        Paused,
        Cancelled,
    };

    explicit FileJob(QObject *parent = 0);
signals:
    void progressPercent(int value);
    void error(QString content);
    void result(QString content);
public slots:
    void doJob(const QString &source, const QString &destination);
    void paused();
    void started();
    void cancelled();
private:
    Status m_status;
    bool copyFile(const QString &srcFile, const QString &tarFile);
    bool copyDir(const QString &srcPath, const QString &tarPath);
};

#endif // FILEJOB_H
