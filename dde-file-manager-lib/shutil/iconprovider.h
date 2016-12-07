#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include "durl.h"

#include <QObject>
#include <QIcon>
#include <QCache>
#include <QMap>
#include <QSet>
#include <QFileIconProvider>
#include <QGSettings>
#include <QMimeDatabase>

class IconProvider : public QObject
{
    Q_OBJECT
public:
    explicit IconProvider(QObject *parent = 0);
    ~IconProvider();

    static char* icon_name_to_path(const char* name, int size);
    static char* get_icon_for_file(char* giconstr, int size);
    static char* get_icon_theme_name();
    static QString getFileIcon(const QString& path, int size);
    static QPixmap getIconPixmap(QString iconPath, int width=48, int height=48);
    static QString getThemeIconPath(QString iconName, int size=48);
    static QIcon DefaultIcon;

    void initConnect();
    void gtkInit();
    QString getCurrentTheme();

    QMap<QString,QIcon> getDesktopIcons();
    QMap<QString,QString> getDesktopIconPaths();

signals:
    void themeChanged(const QString& theme);
    void iconChanged(const QString &filePath);

public slots:
    void setTheme(const QString &themeName);
    void setCurrentTheme();
    void handleThemeChanged(const QString &key);

    QIcon getFileIcon(const DUrl& fileUrl, const QMimeType &mimeType);
    QIcon getDesktopIcon(const QString& iconName, int size);

    void setDesktopIconPaths(const QMap<QString,QString>& iconPaths);

    QIcon getIconByMimeType(const QUrl &url, const QMimeType &mimeType);
    QIcon getThumbnail(const DUrl &url);

    void requestThumbnail(const DUrl &url);
    void abortRequestThumbnail(const DUrl &url);

private:
    QString getMimeTypeByFile(const QString &file);

private:
    mutable QHash<QString,QIcon> m_mimeIcons;
    mutable QMap<QString,QIcon> m_desktopIcons;
    mutable QMap<QString,QString> m_desktopIconPaths;
    mutable QCache<qint64,QIcon> m_icons;
    mutable QMap<QString,QIcon> m_thumbnailIcons;

    QSet<QString> m_supportImageMimeTypesSet;
    QList<QSize> m_iconSizes;
    QGSettings* m_gsettings;
    QMimeDatabase* m_mimeDatabase;
};

#endif // ICONPROVIDER_H
