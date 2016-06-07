#include "iconprovider.h"
#include <QImage>
#include <QImageReader>
#include <QSvgRenderer>
#include <QImageWriter>
#include <QFileInfo>
#include <QTextStream>
#include <QCache>
#include <QApplication>
#include <QPalette>
#include <QPainter>
#include <QBuffer>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QImageReader>

#include "fileutils.h"
#include "desktopfile.h"
#include "../app/global.h"
#include "../controllers/appcontroller.h"

#undef signals
extern "C" {
  #include <gtk/gtk.h>
}
#define signals public


IconProvider::IconProvider(QObject *parent) : QObject(parent)
{
    m_gsettings = new QGSettings("com.deepin.wrap.gnome.desktop.wm.preferences",
                                 "/com/deepin/wrap/gnome/desktop/wm/preferences/");
    m_mimeDatabase = new QMimeDatabase;
    m_iconSizes << QSize(48, 48) << QSize(64, 64) << QSize(96, 96) << QSize(128, 128) << QSize(256, 256);
    initConnect();
    setCurrentTheme();
}

IconProvider::~IconProvider()
{

}

QPixmap IconProvider::getIconPixmap(QString iconPath, int width, int height)
{
    QPixmap pixmap(width, height);
    // iconPath is an absolute path of the system.
    if (QFile::exists(iconPath)) {
        pixmap = QPixmap(iconPath);
    } else if (iconPath.startsWith("data:image/")){
        // iconPath is a string representing an inline image.
        QStringList strs = iconPath.split("base64,");
        if (strs.length() == 2) {
            QByteArray data = QByteArray::fromBase64(strs.at(1).toLatin1());
            pixmap.loadFromData(data);
        }
    } else {
        // try to read the iconPath as a icon name.
        QString path = getThemeIconPath(iconPath);
        qDebug() << path << path.isEmpty();
        if (path.isEmpty())
            path = getThemeIconPath("application-default-icon");
        if (path.endsWith(".svg")) {
            QSvgRenderer renderer(path);
            pixmap.fill(Qt::transparent);

            QPainter painter;
            painter.begin(&pixmap);

            renderer.render(&painter);

            painter.end();
        } else {
            pixmap.load(path);
        }
    }

    return pixmap;
}

QString IconProvider::getThemeIconPath(QString iconName, int size)
{
    QByteArray bytes = iconName.toUtf8();
    const char *name = bytes.constData();

//    GtkIconTheme* theme = gtk_icon_theme_get_default();

//    GtkSettings* gs = gtk_settings_get_default();
//    char* aname = NULL;
//    g_object_get(gs, "gtk-icon-theme-name", &aname, NULL);

//    auto theme = gtk_icon_theme_new();
//    gtk_icon_theme_set_custom_theme(theme, aname);

//    g_free(aname);

//    GtkIconInfo* info = gtk_icon_theme_lookup_icon(theme, name, size, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
//    g_object_unref(theme);
//    if (info) {
//        char* path = g_strdup(gtk_icon_info_get_filename(info));
//#if GTK_MAJOR_VERSION >= 3
//        g_object_unref(info);
//#elif GTK_MAJOR_VERSION == 2
//        gtk_icon_info_free(info);
//#endif
//        return QString(path);
//    } else {
        GtkIconTheme* theme = gtk_icon_theme_get_default();
        GtkIconInfo* info = gtk_icon_theme_lookup_icon(theme, name, size, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
        if (info) {
            char* path = g_strdup(gtk_icon_info_get_filename(info));
    #if GTK_MAJOR_VERSION >= 3
            g_object_unref(info);
    #elif GTK_MAJOR_VERSION == 2
            gtk_icon_info_free(info);
    #endif
            return QString(path);
        } else {
//            qDebug() << iconName << "no info";
        }
        return "";
//    }
}


void IconProvider::initConnect()
{
    connect(m_gsettings, SIGNAL(changed(QString)),
            this, SLOT(handleWmValueChanged(QString)));
}

void IconProvider::gtkInit()
{
    gtk_init(NULL, NULL);
    gdk_error_trap_push();
}

QString IconProvider::getCurrentTheme()
{
    QString temp = m_gsettings->get("theme").toString();
    if (!temp.isEmpty()){
        return temp;
    }else{
        return QIcon::themeName();
    }
}

QMap<QString, QIcon> IconProvider::getDesktopIcons()
{
    return m_desktopIcons;
}

QMap<QString, QString> IconProvider::getDesktopIconPaths()
{
    return m_desktopIconPaths;
}


void IconProvider::setTheme(const QString &themeName)
{
    QIcon::setThemeName(themeName);
}

void IconProvider::setCurrentTheme()
{
    QString temp = getCurrentTheme();
    setTheme(temp);
}

void IconProvider::handleWmValueChanged(const QString &key)
{
    if (key == "theme"){
        QString theme = getCurrentTheme();
        if (QIcon::themeName() == theme){
            return;
        }
        qDebug() << "Theme change from" << QIcon::themeName() << "to" << theme;
        setTheme(theme);
        emit themeChanged(theme);
    }
}


QIcon IconProvider::getFileIcon(const QString &absoluteFilePath, const QString &mimeType)
{
    return findIcon(absoluteFilePath, mimeType);
}

QIcon IconProvider::getDesktopIcon(const QString &iconName, int size)
{
    if (m_desktopIcons.contains(iconName)){
        return m_desktopIcons.value(iconName);
    }else{
        QPixmap pixmap(size, size);
        if (iconName.startsWith("data:image/")){
            // iconPath is a string representing an inline image.
            QStringList strs = iconName.split("base64,");
            if (strs.length() == 2) {
                QByteArray data = QByteArray::fromBase64(strs.at(1).toLatin1());
                pixmap.loadFromData(data);
            }

            QIcon icon = QIcon(pixmap);

            m_desktopIcons[iconName] = icon;

            return icon;
        }else {
            // try to read the iconPath as a icon name.

            QString path;
            if (m_desktopIconPaths.contains(iconName)){
                path = m_desktopIconPaths.value(iconName);
            }else{
                path = getThemeIconPath(iconName, size);
                if (path.isEmpty())
                    path = getThemeIconPath("application-default-icon");
                m_desktopIconPaths[iconName] = path;
            }
            QIcon icon = QIcon(path);

            m_desktopIcons[iconName] = icon;

            return icon;
        }
    }
}

void IconProvider::setDesktopIconPaths(const QMap<QString, QString> &iconPaths)
{
    m_desktopIconPaths = iconPaths;
}

QIcon IconProvider::findIcon(const QString &absoluteFilePath, const QString &mimeType)
{
    // If type of file is directory, return icon of directory
    QIcon theIcon;

    if (mimeType == "application/x-desktop")
        return IconProvider::getDesktopIcon(DesktopFile(absoluteFilePath).getIcon(), 48);
    else if (QImageReader::supportedMimeTypes().contains(mimeType.toLocal8Bit())){
        theIcon = thumbnailManager->thumbnailIcon(absoluteFilePath);

        if (!theIcon.isNull())
            return theIcon;
    }

    theIcon = m_mimeIcons.value(mimeType);

    if (!theIcon.isNull())
        return theIcon;

    QString _mimeType = mimeType;
    QString iconName = _mimeType.replace("/", "-");
    QString path = getThemeIconPath(iconName, 256);

    if (path.isEmpty()){
        path = getThemeIconPath(mimeTypeDisplayManager->defaultIcon(mimeType));
    }

    theIcon = QIcon(path);

//    theIcon = FileUtils::searchMimeIcon(mimeType);

    m_mimeIcons.insert(mimeType, theIcon);

    return theIcon;
}


QString IconProvider::getMimeTypeByFile(const QString &file)
{
    QString mimeTypeName;
    if (systemPathManager->isSystemPath(file)){
        mimeTypeName = systemPathManager->getSystemPathIconNameByPath(file);
    }else{
        QMimeType mimeType = m_mimeDatabase->mimeTypeForFile(file);
        mimeTypeName = mimeType.name();
    }
    return mimeTypeName;
}
