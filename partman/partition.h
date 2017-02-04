#ifndef PARTITION_H
#define PARTITION_H

#include<QtCore>
#include<QtDBus>
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

    qlonglong freespace() const;
    void setFreespace(const qlonglong &freespace);

    qlonglong total() const;
    void setTotal(const qlonglong &total);

    friend QDBusArgument &operator<<(QDBusArgument &argument, const Partition &obj);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, Partition &obj);
    static void registerMetaType();

private:
    QString m_path;
    QString m_fs;
    QString m_label;
    QString m_name;
    QString m_uuid;
    QString m_mountPoint;
    bool m_isRemovable = false;
    qlonglong m_freespace = 0;
    qlonglong m_total = 0;
};

QDebug operator<<(QDebug dbg, const Partition& partion);

}

Q_DECLARE_METATYPE(PartMan::Partition)

#endif // PARTITION_H
