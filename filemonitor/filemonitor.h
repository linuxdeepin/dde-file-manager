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
    void requestMonitorPath(const QStringList &path);
    void requestRemoveMonitorPath(const QStringList &path);
    void fileCreated(QString path);
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

    static QStringList getPathParentList(const QString &path);
};

#endif // FileMonitor_H
