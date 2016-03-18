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
#include "fileutils.h"
#include "desktopfile.h"

#undef signals
extern "C" {
  #include <gtk/gtk.h>
}
#define signals public


IconProvider::IconProvider(QObject *parent) : QObject(parent)
{
    m_mimeUtilsPtr = new MimeUtils;
    m_iconProvider = new QFileIconProvider;
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

QString IconProvider::getThemeIconPath(QString iconName)
{
    QByteArray bytes = iconName.toUtf8();
    const char *name = bytes.constData();

//    GtkIconTheme* theme = gtk_icon_theme_get_default();

    GtkSettings* gs = gtk_settings_get_default();
    char* aname = NULL;
    g_object_get(gs, "gtk-icon-theme-name", &aname, NULL);

    auto theme = gtk_icon_theme_new();
    gtk_icon_theme_set_custom_theme(theme, aname);

    g_free(aname);

    GtkIconInfo* info = gtk_icon_theme_lookup_icon(theme, name, 48, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
    g_object_unref(theme);
    if (info) {
        char* path = g_strdup(gtk_icon_info_get_filename(info));
#if GTK_MAJOR_VERSION >= 3
        g_object_unref(info);
#elif GTK_MAJOR_VERSION == 2
        gtk_icon_info_free(info);
#endif
        return QString(path);
    } else {
        GtkIconTheme* theme = gtk_icon_theme_get_default();
        GtkIconInfo* info = gtk_icon_theme_lookup_icon(theme, name, 48, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
        if (info) {
            char* path = g_strdup(gtk_icon_info_get_filename(info));
    #if GTK_MAJOR_VERSION >= 3
            g_object_unref(info);
    #elif GTK_MAJOR_VERSION == 2
            gtk_icon_info_free(info);
    #endif
            return QString(path);
        } else {
            qDebug() << iconName << "no info";
        }
        return "";
    }
}

void IconProvider::gtkInit()
{
    gtk_init(NULL, NULL);
    gdk_error_trap_push();
}

void IconProvider::loadMimeTypes() const
{
    // Open file with mime/suffix associations
    QFile mimeInfo("/usr/share/mime/globs");
    mimeInfo.open(QIODevice::ReadOnly);
    QTextStream out(&mimeInfo);

    // Read associations
    do {
      QStringList line = out.readLine().split(":");
      if (line.count() == 2) {
        QString suffix = line.at(1);
        suffix.remove("*.");
        QString mimeName = line.at(0);
        mimeName.replace("/","-");
        m_mimeGlob.insert(suffix, mimeName);
      }
    } while (!out.atEnd());
    mimeInfo.close();

    // Open file with mime/generic-mime associations
    mimeInfo.setFileName("/usr/share/mime/generic-icons");
    mimeInfo.open(QIODevice::ReadOnly);
    out.setDevice(&mimeInfo);

    // Read associations
    do {
      QStringList line = out.readLine().split(":");
      if (line.count() == 2) {
        QString mimeName = line.at(0);
        mimeName.replace("/","-");
        QString icon = line.at(1);
        m_mimeGeneric.insert(mimeName, icon);
      }
    } while (!out.atEnd());
    mimeInfo.close();
}

QByteArray IconProvider::getThumb(const QString &imageFile)
{
    // Thumbnail image
    QImage theThumb, background;
    QImageReader pic(imageFile);
    int w = pic.size().width();
    int h = pic.size().height();

    // Background
    background = QImage(128, 128, QImage::Format_RGB32);
    background.fill(QApplication::palette().color(QPalette::Base).rgb());

    // Scale image and create its shadow template (background.png)
    if (w > 128 || h > 128) {
      pic.setScaledSize(QSize(123, 93));
      QImage temp = pic.read();
      theThumb.load(":/images/background.png");
      QPainter painter(&theThumb);
      painter.drawImage(QPoint(0, 0), temp);
    } else {
      pic.setScaledSize(QSize(64, 64));
      theThumb = pic.read();
    }

    // Draw thumbnail picture
    QPainter painter(&background);
    painter.drawImage(QPoint((123 - theThumb.width()) / 2,
                             (115 - theThumb.height()) / 2), theThumb);

    // Write it to buffer
    QBuffer buffer;
    QImageWriter writer(&buffer, "jpg");
    writer.setQuality(50);
    writer.write(background);
    return buffer.buffer();
}


void IconProvider::setTheme(const QString &themeName)
{
    QIcon::setThemeName(themeName);
}

void IconProvider::setCurrentTheme()
{
    QString temp;
    if(temp.isNull())
    {
        //get theme from system (works for gnome/kde)
        temp = QIcon::themeName();

        //Qt doesn't detect the theme very well for non-DE systems,
        //so try reading the '~/.gtkrc-2.0' or '~/.config/gtk-3.0/settings.ini'

        if(temp == "hicolor")
        {
            //check for gtk-2.0 settings
            if(QFile::exists(QDir::homePath() + "/" + ".gtkrc-2.0"))
            {
                QSettings gtkFile(QDir::homePath() + "/.gtkrc-2.0",QSettings::IniFormat,this);
                temp = gtkFile.value("gtk-icon-theme-name").toString().remove("\"");
            }
            else
            {
                //try gtk-3.0
                QSettings gtkFile(QDir::homePath() + "/.config/gtk-3.0/settings.ini",QSettings::IniFormat,this);
                temp = gtkFile.value("gtk-fallback-icon-theme").toString().remove("\"");
            }

            //fallback
            if(temp.isNull())
            {
                if(QFile::exists("/usr/share/icons/gnome")) temp = "gnome";
                else if(QFile::exists("/usr/share/icons/oxygen")) temp = "oxygen";
                else temp = "hicolor";
            }
        }
    }
    QIcon::setThemeName(temp);
}


QIcon IconProvider::getFileIcon(const QString &file) const
{
    return findIcon(file);
}

QIcon IconProvider::getDesktopIcon(const QString &iconName, int size) const
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
            QString path = getThemeIconPath(iconName);

            if (path.isEmpty())
                path = getThemeIconPath("application-default-icon");

            QIcon icon = QIcon(path);

            m_desktopIcons[iconName] = icon;

            return icon;
        }
    }
}


QIcon IconProvider::findIcon(const QString &file) const
{
    // If type of file is directory, return icon of directory

    QFileInfo type(file);
    QString absoluteFilePath = type.absoluteFilePath();

    if (type.isDir()) {
      if (m_folderIcons.contains(type.fileName())) {
        return m_folderIcons.value(type.fileName());
      }

      return m_iconProvider->icon(type);
    }

    // If thumbnails are allowed and current file has it, show it
    if (false) {
      if (m_icons.contains(absoluteFilePath)) {
        return *m_icons.object(absoluteFilePath);
      } else if (m_thumbs.contains(absoluteFilePath)) {
        QPixmap pic;
        pic.loadFromData(m_thumbs.value(absoluteFilePath));
        m_icons.insert(absoluteFilePath, new QIcon(pic), 1);
        return *m_icons.object(absoluteFilePath);
      }
    }

    // NOTE: Suffix is resolved using method getRealSuffix instead of suffix()
    // method. It is because files can contain version suffix e.g. .so.1.0.0

    // If there is icon for current suffix then return it
    QString suffix = FileUtils::getRealSuffix(type.fileName()); /*type.suffix();*/
    if (m_mimeIcons.contains(suffix)) {
      return m_mimeIcons.value(suffix);
    }

    // The icon
    QIcon theIcon;

    // If file has not suffix
    if (suffix.isEmpty()) {

      // If file is not executable, read mime type info from the system and create
      // an icon for it
      // NOTE: the icon cannot be cached because this file has not any suffix,
      // however operation 'getMimeType' could cause slowdown
      if (!type.isExecutable()) {
        QString mime = m_mimeUtilsPtr->getMimeType(absoluteFilePath);
        return FileUtils::searchMimeIcon(mime);
      }

      // If file is executable, set suffix to exec and find/create icon for it
      suffix = "exec";
      if (m_mimeIcons.contains(suffix)) {
        theIcon = m_mimeIcons.value(suffix);
      } else {
        theIcon = QIcon::fromTheme("application-x-executable", theIcon);
      }
    }
    // If file has unknown suffix (icon hasn't been assigned)
    else {

      // Load mime/suffix associations if they aren't loaded yet
      if (m_mimeGlob.count() == 0) loadMimeTypes();

      // Retrieve mime type for current suffix, if suffix is not present in list
      // from '/usr/share/mime/globs', its mime has to be detected manually
      QString mimeType = m_mimeGlob.value(suffix.toLower(), "");
      if (mimeType.isEmpty()) {
        mimeType = m_mimeUtilsPtr->getMimeType(absoluteFilePath);
        m_mimeGlob.insert(suffix.toLower(), mimeType);
      }

      // Load the icon
      theIcon = FileUtils::searchMimeIcon(mimeType);
    }

    // Insert icon to the list of icons
    m_mimeIcons.insert(suffix, theIcon);
    return theIcon;
}

QIcon IconProvider::findMimeIcon(const QString &file)
{
    QFileInfo type(file);
    // Retrieve mime and search cache for it
    QString mime = m_mimeUtilsPtr->getMimeType(type.absoluteFilePath());
    if (m_mimeIcons.contains(mime)) {
      return m_mimeIcons.value(mime);
    }

    // Search file system for icon
    QIcon theIcon = FileUtils::searchMimeIcon(mime);
    m_mimeIcons.insert(mime, theIcon);
    return theIcon;
}

