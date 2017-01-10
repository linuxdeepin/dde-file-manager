#ifndef PARTITION_H
#define PARTITION_H

#include<QString>
#include<QDebug>

namespace PartMan {

class Partition;

class Partition
{
public:
    Partition();

    static Partition getPartitionByDevicePath(const QString& devicePath);

    QString path() const;
    void setPath(const QString &path);

    QString fs() const;
    void setFs(const QString &fs);

    QString label() const;
    void setLabel(const QString &label);

    QString name() const;
    void setName(const QString &name);

    QString uuid() const;
    void setUuid(const QString &uuid);

    QString mountPoint() const;
    void setMountPoint(const QString &mountPoint);

    bool getIsRemovable() const;
    void setIsRemovable(bool isRemovable);

private:
    QString m_path;
    QString m_fs;
    QString m_label;
    QString m_name;
    QString m_uuid;
    QString m_mountPoint;
    bool m_isRemovable = false;
};

QDebug operator<<(QDebug dbg, const Partition& partion);

}
#endif // PARTITION_H
