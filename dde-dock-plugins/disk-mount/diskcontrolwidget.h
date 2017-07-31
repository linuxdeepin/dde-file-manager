#ifndef DISKCONTROLWIDGET_H
#define DISKCONTROLWIDGET_H

#include <QScrollArea>
#include <QVBoxLayout>

class DFMSetting;
class GvfsMountManager;
class QDiskInfo;


class DiskControlWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit DiskControlWidget(QWidget *parent = 0);
    void initConnect();

    void startMonitor();
    void unmountAll();

signals:
    void diskCountChanged(const int count) const;

private slots:
    void onDiskListChanged();
    void onMount_added(const QDiskInfo &diskInfo);
    void onMount_removed(const QDiskInfo &diskInfo);
    void onVolume_added(const QDiskInfo &diskInfo);
    void onVolume_removed(const QDiskInfo &diskInfo);
    void onVolume_changed(const QDiskInfo &diskInfo);
    void unmountDisk(const QString &diskId) const;

private:
    QVBoxLayout *m_centralLayout;
    QWidget *m_centralWidget;
    DFMSetting* m_dfmsettings;
    GvfsMountManager *m_gvfsMountManager;
};

#endif // DISKCONTROLWIDGET_H
