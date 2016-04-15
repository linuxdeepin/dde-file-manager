#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QObject>
#include <QIcon>
#include <QCache>
#include <QHash>
#include <QFileIconProvider>
#include <QGSettings>
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
    void loadMimeTypes() const;
    QByteArray getThumb(const QString& imageFile);

    QMap<QString,QIcon> getDesktopIcons();

signals:
    void themeChanged(const QString& theme);

public slots:

    void setTheme(const QString &themeName);
    void setCurrentTheme();
    void handleWmValueChanged(const QString &key);

    QIcon getFileIcon(const QString& file) const;
    QIcon getDesktopIcon(const QString& iconName, int size) const;
private:
    QIcon findIcon(const QString& file) const;
    QIcon findMimeIcon(const QString& file);

private:
    mutable QMap<QString,QIcon> m_mimeIcons;
    QMap<QString,QIcon> m_folderIcons;
    mutable QMap<QString,QIcon> m_desktopIcons;
    mutable QCache<QString,QIcon> m_icons;

    mutable QMap<QString,QString> m_mimeGlob;
    mutable QMap<QString,QString> m_mimeGeneric;
    QHash<QString,QByteArray> m_thumbs;

    MimeUtils* m_mimeUtilsPtr;
    QFileIconProvider* m_iconProvider;
    QGSettings* m_gsettings;
};

#endif // ICONPROVIDER_H
