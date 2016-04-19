#ifndef UDISKDEVICEINFO_H
#define UDISKDEVICEINFO_H

#include "../filemanager/models/abstractfileinfo.h"

#include <QDBusInterface>
#include <QString>
#include <QDebug>
#include <QDBusArgument>

#define BOOL_SETTER(MEMBER) { bool res=(text != MEMBER); MEMBER = text; return res; }

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

    UDiskDeviceInfo(UDiskDeviceInfo * info);
    UDiskDeviceInfo(const DUrl &url);
    UDiskDeviceInfo(const QString &url);
    UDiskDeviceInfo(const QDBusObjectPath &path);
    bool mount();
    bool unmount();
    bool eject();

    bool update();

    QString uDiskPath();

    QString devFile();
    QString label();
    QString vendor();
    QString model();
    QString fileSystem();
    QString mountPath() const;
    QString iconName();

    bool setDevFile(const QString &text) BOOL_SETTER(m_devFile)
    bool setLabel(const QString &text) BOOL_SETTER(m_label)
    bool setVendor(const QString &text) BOOL_SETTER(m_vendor)
    bool setModel(const QString &text) BOOL_SETTER(m_model)
    bool setFileSystem(const QString &text) BOOL_SETTER(m_fileSystem)
    bool setMountPath(const QString &text) BOOL_SETTER(m_mountPath)
    bool setIconName(const QString &text) BOOL_SETTER(m_iconName)

    bool setSize(qint64 text)      BOOL_SETTER(m_size)
    bool setType(Type text)        BOOL_SETTER(m_type)

    bool setIsValid(bool text)         BOOL_SETTER(m_isValid)
    bool setIsExternal(bool text)      BOOL_SETTER(m_isExternal)
    bool setIsMounted(bool text)       BOOL_SETTER(m_isMounted)
    bool setIsEjectable(bool text)     BOOL_SETTER(m_isEjectable)


    qint64 size();
    QString displayName() const;
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
    QDBusObjectPath m_path;
private:
    QDBusInterface *m_dbus;
private:
    QDBusInterface *m_blockIface = NULL;
    QDBusInterface *m_driveIface = NULL;
    QDBusInterface *m_fsIface = NULL;


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
    bool isCanRename() const Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    DUrl parentUrl() const Q_DECL_OVERRIDE;

signals:
    void changed();
    void error(const QString &msg);
    void mounted();
    void unmounted();
public slots:
    void dbusError(const QDBusError &err, const QDBusMessage &msg);
    void propertiesChanged(const QString &interface,
                           const QVariantMap &changedProp,
                           const QStringList &invalidatedProp);



public:
    QVector<MenuAction> menuActionList(MenuType type) const Q_DECL_OVERRIDE;
};

#endif // UDISKDEVICEINFO_H
