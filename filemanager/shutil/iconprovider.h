#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QObject>
#include <QIcon>
#include <QCache>
#include <QHash>
#include <QFileIconProvider>
#include "mimeutils.h"

class IconProvider : public QObject
{
    Q_OBJECT
public:
    explicit IconProvider(QObject *parent = 0);
    ~IconProvider();

    static QPixmap getIconPixmap(QString iconPath, int width=48, int height=48);
    static QString getThemeIconPath(QString iconName);
    void gtkInit();
    void loadMimeTypes();
    QByteArray getThumb(const QString& imageFile);

signals:

public slots:
    void setTheme(const QString &themeName);
    void setCurrentTheme();
    QIcon icon(const QString& file);
private:
    QIcon findIcon(const QString& file);
    QIcon findMimeIcon(const QString& file);

private:
    QHash<QString,QIcon> m_mimeIcons;
    QHash<QString,QIcon> m_folderIcons;
    QCache<QString,QIcon> m_icons;

    QHash<QString,QString> m_mimeGlob;
    QHash<QString,QString> m_mimeGeneric;
    QHash<QString,QByteArray> m_thumbs;

    MimeUtils* m_mimeUtilsPtr;
    QFileIconProvider* m_iconProvider;
};

#endif // ICONPROVIDER_H
