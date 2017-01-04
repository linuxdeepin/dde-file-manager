#ifndef USBFORMATTER_H
#define USBFORMATTER_H

#include <QObject>
#include "../partman/partitionmanager.h"

class UsbFormatterAdaptor;

class UsbFormatter : public QObject
{
    Q_OBJECT
public:
    static QString ObjectPath;
    explicit UsbFormatter(QObject *parent = 0);

signals:

public slots:
   bool mkfs(const QString& path, const QString& fs, const QString& label);

private:
    UsbFormatterAdaptor* m_usbFormatterAdaptor = NULL;
    PartMan::PartitionManager* m_partitionManager = NULL;
};

#endif // USBFORMATTER_H
