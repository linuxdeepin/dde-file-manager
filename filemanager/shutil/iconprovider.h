#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QObject>
#include <QIcon>
#include <QCache>
#include <QHash>
#include <QFileIconProvider>
#include <QGSettings>
#include <QMimeDatabase>
#include "mimeutils.h"

class IconProvider : public QObject
{
    Q_OBJECT
public:
    explicit IconProvider(QObject *parent = 0);
    ~IconProvider();

    static QPixmap getIconPixmap(QString iconPath, int width=48, int height=48);
    static QString getThemeIconPath(QString iconName);
    void initConnect();
    void gtkInit();
    QString getCurrentTheme();

    QMap<QString,QIcon> getDesktopIcons();
    QMap<QString,QString> getDesktopIconPaths();

signals:
    void themeChanged(const QString& theme);

public slots:

    void setTheme(const QString &themeName);
    void setCurrentTheme();
    void handleWmValueChanged(const QString &key);

    QIcon getFileIcon(const QString& file);
    QIcon getDesktopIcon(const QString& iconName, int size);

    void setDesktopIconPaths(const QMap<QString,QString>& iconPaths);

private:
    QIcon findIcon(const QString& file);
    QString getMimeTypeByFile(const QString &file);

private:
    mutable QMap<QString,QIcon> m_mimeIcons;
    mutable QMap<QString,QIcon> m_desktopIcons;
    mutable QMap<QString,QString> m_desktopIconPaths;
    mutable QCache<QString,QIcon> m_icons;

    QGSettings* m_gsettings;
    QMimeDatabase* m_mimeDatabase;
};

#endif // ICONPROVIDER_H
