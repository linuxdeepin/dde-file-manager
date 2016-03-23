#ifndef FileMonitor_H
#define FileMonitor_H

#include <QObject>
#include <QMap>

class FileMonitorWoker;

class FileMonitor : public QObject
{
    Q_OBJECT
public:
    explicit FileMonitor(QObject *parent = 0);
    ~FileMonitor();

    void initConnect();
    void initFileMonitorWoker();

    static bool isGoutputstreamTempFile(QString path);

    void addMonitorPath(const QString &path);
    void removeMonitorPath(const QString &path);

signals:
    void fileCreated(QString path);
    void fileMovedIn(QString path);
    void fileMovedOut(QString out);
    void fileRenamed(QString oldPath, QString newPath);
    void fileDeleted(QString path);
    void fileMetaDataChanged(QString path);

private slots:
    void handleCreated(int cookie, QString path);
    void handleMoveFrom(int cookie, QString path);
    void handleMoveTo(int cookie, QString path);
    void handleDelete(int cookie, QString path);
    void handleMetaDataChanged(int cookie, QString path);

private:
    FileMonitorWoker* m_fileMonitorWorker;
    QThread* m_fileThread;
    QMap<int, QString> m_moveEvent;
    QMap<QString, int> m_pathMonitorConuter;
    QString m_appGroupPath;
};

#endif // FileMonitor_H
