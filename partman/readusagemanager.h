#ifndef READUSAGEMANAGER_H
#define READUSAGEMANAGER_H

#include <QObject>

namespace PartMan {

class ReadUsageManager : public QObject
{
    Q_OBJECT
public:
    explicit ReadUsageManager(QObject *parent = 0);

    static qlonglong ParseBtrfsUnit(const QString& value);

signals:

public slots:
    bool readUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readUsage(const QString& path, const QString& fs, qlonglong& freespace, qlonglong& total);
    bool readBtrfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readEfiUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readExt2Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readExt3Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readExt4Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readF2fsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readFat16Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readFat32Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readHfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readHfsplusUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readJfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readLinuxswapUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readLvm2pvUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readNilfs2Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readNtfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readReiser4Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readReiserfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readXfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readUnknownUsage(const QString& path, qlonglong& freespace, qlonglong& total);
};
}
#endif // READUSAGEMANAGER_H
