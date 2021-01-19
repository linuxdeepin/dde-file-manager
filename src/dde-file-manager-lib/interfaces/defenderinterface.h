#ifndef DEFENDER_INTERFACE_H
#define DEFENDER_INTERFACE_H

#include <QObject>
#include <QMap>
#include <QUrl>
class QDBusInterface;

class DefenderInterface : public QObject
{
    Q_OBJECT

public:
    DefenderInterface(QObject *parent = nullptr);
    ~DefenderInterface();

    // 检查当前扫描路径中是否存在url或url的子目录
    bool isScanning(const QUrl &url);
    // 停止扫描url及url的子目录，返回false为停止失败
    bool stopScanning(const QUrl &url);

public Q_SLOTS:
    void scanningUsbPathsChanged(QStringList list);

private:
    // 从当前扫描路径选取url的子目录，若url本身也被扫描，也会在返回列表中
    QList<QUrl> getScanningPaths(const QUrl &url = QUrl("/"));

private:
    QDBusInterface *interface;
    QList<QUrl> scanningPaths;
};

#endif // DEFENDER_INTERFACE_H
