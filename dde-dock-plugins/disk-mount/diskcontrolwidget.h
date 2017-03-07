#ifndef DISKCONTROLWIDGET_H
#define DISKCONTROLWIDGET_H

#include <QScrollArea>
#include <QVBoxLayout>

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
    void unmountDisk(const QString &diskId) const;
    void addMountDiskInfo(const QDiskInfo &diskInfo);

private:
    QVBoxLayout *m_centralLayout;
    QWidget *m_centralWidget;
    GvfsMountManager *m_gvfsMountManager;
};

#endif // DISKCONTROLWIDGET_H
