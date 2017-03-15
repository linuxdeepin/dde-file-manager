#ifndef DISKCONTROLITEM_H
#define DISKCONTROLITEM_H

#include <dimagebutton.h>

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QIcon>
#include "qdiskinfo.h"

class DiskControlItem : public QFrame
{
    Q_OBJECT

public:
    explicit DiskControlItem(const QDiskInfo &info, QWidget *parent = 0);

signals:
    void requestUnmount(const QString &diskId) const;

private slots:
    void updateInfo(const QDiskInfo &info);
    static QString sizeString(const QString &str);
    static qreal dRound64(qreal num, int count = 1);
    const QString formatDiskSize(const quint64 num) const;

private:
    QDiskInfo m_info;
    QIcon m_unknowIcon;

    QLabel *m_diskIcon;
    QLabel *m_diskName;
    QLabel *m_diskCapacity;
    QProgressBar *m_capacityValueBar;
    Dtk::Widget::DImageButton *m_unmountButton;
};

#endif // DISKCONTROLITEM_H
