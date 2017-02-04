#ifndef QMOUNT_H
#define QMOUNT_H

#include <QString>
#include <QMap>
#include <QDebug>

class QMount
{
public:
    QMount();

    QString name() const;
    QString uuid() const;

    QString default_location() const;
    void setDefault_location(const QString &default_location);

    QStringList icons() const;

    QStringList symbolic_icons() const;
    void setSymbolic_icons(const QStringList &symbolic_icons);

    bool can_unmount() const;
    void setCan_unmount(bool can_unmount);

    bool can_eject() const;
    void setCan_eject(bool can_eject);

    bool is_shadowed() const;
    void setIs_shadowed(bool is_shadowed);

    QString sort_key() const;
    void setSort_key(const QString &sort_key);

    void setName(const QString &name);

    void setIcons(const QStringList &icons);

    QString mounted_root_uri() const;
    void setMounted_root_uri(const QString &mounted_root_uri);

private:
    QString m_name;
    QString m_mounted_root_uri;
    QString m_uuid;
    QString m_default_location;
    QStringList m_icons;
    QStringList m_symbolic_icons;
    bool m_can_unmount = false;
    bool m_can_eject = false;
    bool m_is_shadowed = false;
    QString m_sort_key;
};

QDebug operator<<(QDebug dbg, const QMount& mount);

#endif // QMOUNT_H
