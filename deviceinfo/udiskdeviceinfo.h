#ifndef UDISKDEVICEINFO_H
#define UDISKDEVICEINFO_H

#include "../filemanager/models/abstractfileinfo.h"

#include <QDBusInterface>
#include <QString>

class UDiskDeviceInfo : public QObject, public AbstractFileInfo
{
    Q_OBJECT
public:
    enum Type
    {
            Unknown,
            Drive,
            Partition,
            Fdd,
            Optical
    };
    UDiskDeviceInfo(const QDBusObjectPath &path);
    bool mount();
    bool unmount();
    bool eject();

    QString devFile();
    QString label();
    QString vendor();
    QString model();
    QString fileSystem();
    QString mountPath();
    QString iconName();

    void setDevFile(const QString &text);
    void setLabel(const QString &text);
    void setVendor(const QString &text);
    void setModel(const QString &text);
    void setFileSystem(const QString &text);
    void setMountPath(const QString &text);
    void setIconName(const QString &text);


    qint64 size();
    Type type();
    bool isValid();
    bool isExternal();
    bool isMounted();
    bool isEjectable();

    Type findType();
    QString findLabel();
    bool findIsExternal();
    QString findIconName();
    QStringList mountPoints() const;
private:
    QDBusInterface *m_dbus;

    QString m_devFile;
    QString m_label;
    QString m_vendor;
    QString m_model;
    QString m_fileSystem;
    QString m_mountPath;
    QString m_iconName;

    qint64 m_size;
    Type m_type;

    bool m_isValid;
    bool m_isExternal;
    bool m_isMounted;
    bool m_isEjectable;

    QString formatSize( qint64 num ) const;
public:
    bool isCanRename() const;
    QIcon fileIcon() const;
    bool isDir() const;
};

#endif // UDISKDEVICEINFO_H
