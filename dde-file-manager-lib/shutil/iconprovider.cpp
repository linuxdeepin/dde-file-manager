#include "iconprovider.h"
#include "fileutils.h"
#include "desktopfile.h"
//#include "thumbnailmanager.h"
#include "thumbnailer/thumbnailmanager.h"

#include "app/define.h"

#include "controllers/pathmanager.h"
#include "controllers/appcontroller.h"

#include "shutil/mimesappsmanager.h"
#include "shutil/mimetypedisplaymanager.h"

#include "widgets/singleton.h"

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

#define ICON_BORDER 5

#undef signals
extern "C" {
  #include <gtk/gtk.h>
  #include <gio/gio.h>
}
#define signals public


static GtkIconTheme* them = NULL;
QIcon IconProvider::DefaultIcon;
QString IconProvider::CurrentTheme = "Deepin";

IconProvider::IconProvider(QObject *parent) : QObject(parent)
{
    m_gsettings = new QGSettings("com.deepin.dde.appearance",
                                 "/com/deepin/dde/appearance/");
    m_mimeDatabase = new QMimeDatabase;
    m_iconSizes << QSize(48, 48) << QSize(64, 64) << QSize(96, 96) << QSize(128, 128) << QSize(256, 256);

    for (const QByteArray &mime : QImageReader::supportedMimeTypes()) {
        m_supportImageMimeTypesSet << mime;
    }

    // init gtk
    gtk_init(NULL, NULL);
    gdk_error_trap_push();

    initConnect();
//    setCurrentTheme();

    connect(thumbnailManager, &ThumbnailManager::pixmapChanged, this, &IconProvider::iconChanged);
}

IconProvider::~IconProvider()
{

}

char *IconProvider::icon_name_to_path(const char *name, int size)
{
    qDebug() << name;
    if (g_path_is_absolute(name))
            return g_strdup(name);

    g_return_val_if_fail(name != NULL, NULL);

    int pic_name_len = strlen(name);
    const char* ext = strrchr(name, '.');
    if (ext != NULL) {
        if (g_ascii_strcasecmp(ext+1, "png") == 0 || g_ascii_strcasecmp(ext+1, "svg") == 0 || g_ascii_strcasecmp(ext+1, "jpg") == 0) {
            pic_name_len = ext - name;
            g_debug("desktop's Icon name should an absoulte path or an basename without extension");
        }
    }

    // In pratice, default icon theme may not gets the right icon path when program starting.
    if (them == NULL)
        them = gtk_icon_theme_new();
    char* icon_theme_name = get_icon_theme_name();
    gtk_icon_theme_set_custom_theme(them, icon_theme_name);
    g_free(icon_theme_name);

    char* pic_name = g_strndup(name, pic_name_len);
    GtkIconInfo* info = gtk_icon_theme_lookup_icon(them, pic_name, size, GTK_ICON_LOOKUP_GENERIC_FALLBACK);

    if (info == NULL) {
        g_warning("get gtk icon theme info failed for %s", pic_name);
        g_free(pic_name);
        return NULL;
    }
    g_free(pic_name);

    char* path = g_strdup(gtk_icon_info_get_filename(info));

#if GTK_MAJOR_VERSION >= 3
    g_object_unref(info);
#elif GTK_MAJOR_VERSION == 2
    gtk_icon_info_free(info);
#endif
    g_debug("get icon from icon theme is: %s", path);
    return path;
}

char *IconProvider::get_icon_for_file(char *giconstr, int size)
{
    if (giconstr == NULL) {
        return NULL;
    }

    char* icon = NULL;
    char** icon_names = g_strsplit(giconstr, " ", -1);

    for (int i = 0; icon_names[i] != NULL && icon == NULL; ++i) {
        qDebug() << icon_names[i];
        icon = icon_name_to_path(icon_names[i], size);
    }

    g_strfreev(icon_names);

    return icon;
}

char *IconProvider::get_icon_theme_name()
{
    GtkSettings* gs = gtk_settings_get_default();
    char* name = NULL;
    g_object_get(gs, "gtk-icon-theme-name", &name, NULL);
    return name;
}

QString IconProvider::getFileIcon(const QString &path, int size)
{
    GFile* gfile = g_file_new_for_commandline_arg(path.toUtf8().constData());
    if (gfile){
        GFileInfo* gfileinfo = g_file_query_info(gfile, G_FILE_ATTRIBUTE_STANDARD_ICON, G_FILE_QUERY_INFO_NONE, NULL, NULL);
        if (gfileinfo){
            GIcon* icon = g_file_info_get_icon(gfileinfo);
            gchar* iconString = g_icon_to_string(icon);
            char* fileIcon = get_icon_for_file(iconString, size);
            return QString(fileIcon);
        }
    }
    return QString("");
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
        QString path =  getThemeIconPath(iconPath);
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

    if (g_path_is_absolute(name))
        return QString(g_strdup(name));

    g_return_val_if_fail(name != NULL, NULL);

    int pic_name_len = strlen(name);
    const char* ext = strrchr(name, '.');
    if (ext != NULL) {
        if (g_ascii_strcasecmp(ext+1, "png") == 0 || g_ascii_strcasecmp(ext+1, "svg") == 0 || g_ascii_strcasecmp(ext+1, "jpg") == 0) {
            pic_name_len = ext - name;
//            g_debug("desktop's Icon name should an absoulte path or an basename without extension");
        }
    }

    // In pratice, default icon theme may not gets the right icon path when program starting.
    if (them == NULL)
        them = gtk_icon_theme_new();
//    char* icon_theme_name = get_icon_theme_name();
    auto icon_theme_name = CurrentTheme.toStdString().c_str();
    gtk_icon_theme_set_custom_theme(them, icon_theme_name);
//    g_free(icon_theme_name);

    char* pic_name = g_strndup(name, pic_name_len);
    GtkIconInfo* info = gtk_icon_theme_lookup_icon(them, pic_name, size, GTK_ICON_LOOKUP_GENERIC_FALLBACK);

    if (info == NULL) {
//        g_warning("get gtk icon theme info failed for %s", pic_name);
        g_free(pic_name);
        return QString("");
    }
    g_free(pic_name);

    char* path = g_strdup(gtk_icon_info_get_filename(info));

#if GTK_MAJOR_VERSION >= 3
    g_object_unref(info);
#elif GTK_MAJOR_VERSION == 2
    gtk_icon_info_free(info);
#endif
    g_debug("get icon from icon theme is: %s", path);

    return QString(path);
}


void IconProvider::initConnect()
{
    connect(m_gsettings, SIGNAL(changed(QString)),
            this, SLOT(handleThemeChanged(QString)));
}

void IconProvider::gtkInit()
{
    gtk_init(NULL, NULL);
    gdk_error_trap_push();
}

QString IconProvider::getCurrentTheme()
{
    CurrentTheme = m_gsettings->get("icon-theme").toString();
    if (CurrentTheme.isEmpty()){
        CurrentTheme = QIcon::themeName();
    }
    return CurrentTheme;
}

QMap<int, QMap<QString, QIcon> > IconProvider::getDesktopIcons()
{
    return m_desktopIcons;
}

QMap<int, QMap<QString, QString> > IconProvider::getDesktopIconPaths()
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

void IconProvider::handleThemeChanged(const QString &key)
{
    qDebug() << key;
    if (key == "iconTheme"){
        QString theme = getCurrentTheme();
        if (QIcon::themeName() == theme){
            return;
        }
        qDebug() << "Theme change from" << QIcon::themeName() << "to" << theme;
        setTheme(theme);
        m_mimeIcons.clear();
        m_desktopIcons.clear();
        m_desktopIconPaths.clear();
        emit themeChanged(theme);
    }
}

QIcon IconProvider::getDesktopIcon(const QString &iconName, int size)
{
    if (m_desktopIcons[size].contains(iconName)){
        return m_desktopIcons[size].value(iconName);
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

            m_desktopIcons[size][iconName] = icon;

            return icon;
        }else {
            // try to read the iconPath as a icon name.

            QString path;
            if (m_desktopIconPaths[size].contains(iconName)){
                path = m_desktopIconPaths[size].value(iconName);
            }else{
                path = getThemeIconPath(iconName, size);
                if (path.isEmpty()){
//                    path = getThemeIconPath("application-default-icon");
                }else{
                    m_desktopIconPaths[size][iconName] = path;
                }
            }

            QIcon icon;
            if (!path.isEmpty()){
                icon = QIcon(path);
            }else{
                icon = QIcon::fromTheme(iconName);
                if (icon.isNull()){
                    if (DefaultIcon.isNull())
                        DefaultIcon = QIcon(getThemeIconPath("application-default-icon"));
                    return DefaultIcon;
                }
            }

            m_desktopIcons[size][iconName] = icon;

            return icon;
        }
    }
}

QIcon IconProvider::getIconByMimeType(const QUrl &url, const QMimeType &mimeType)
{
    const QString &mimeTypeName = mimeType.name();
    const QString filePath = url.toLocalFile();
    QString _mimeType = mimeTypeName;

    if (systemPathManager->isSystemPath(filePath)) {
        _mimeType = systemPathManager->getSystemPathIconNameByPath(filePath);
    }

    // If type of file is directory, return icon of directory
    QIcon theIcon = m_mimeIcons.value(_mimeType);

    if (!theIcon.isNull())
        return theIcon;

    QString iconName = mimeType.iconName();

    /*todo add whitelists for especial mimetype*/
    if (iconName == "application-wps-office.docx"){
        iconName = "application-wps-office.doc";
    }else if (iconName == "application-vnd.debian.binary-package"){
        iconName = "application-x-deb";
    }else if (iconName == "application-vnd.ms-htmlhelp"){
        iconName = "chmsee";
    }else if (systemPathManager->isSystemPath(filePath)) {
        iconName = _mimeType;
    }

    QString path = getThemeIconPath(iconName, 256);

    if (path.isEmpty()){
        path = getFileIcon(filePath, 256);
    }

    if (path.isEmpty()) {
        path = getThemeIconPath(mimeTypeDisplayManager->defaultIcon(mimeTypeName), 256);
    }

    if (!path.isEmpty()){
        theIcon = QIcon(path);
    }else{
        theIcon = QIcon::fromTheme(iconName);

        if (theIcon.isNull()){
            if (DefaultIcon.isNull())
                DefaultIcon = QIcon(getThemeIconPath("application-default-icon"));
            return DefaultIcon;
        }
    }

    m_mimeIcons.insert(_mimeType, theIcon);

    return theIcon;
}

QIcon IconProvider::getThumbnail(const DUrl &url)
{
    const QPixmap &pixmap = thumbnailManager->getThumbnailPixmap(url, ThumbnailGenerator::THUMBNAIL_LARGE);

    if (pixmap.isNull()) {
        return QIcon();
    }

    QIcon *ico = m_icons.object(pixmap.cacheKey());

    if (ico)
        return *ico;

    QImage img(pixmap.width() + ICON_BORDER*2, pixmap.height() + ICON_BORDER*2,
               QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter pa(&img);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(QColor(0,0,0,0.35*255));
    pa.setPen(pen);
    pa.setRenderHint(pa.Antialiasing);
    QVector<QLine> lines;
    lines << QLine(QPoint(0,0),QPoint(img.width(),0))
          << QLine(QPoint(img.width(),1),QPoint(img.width(),img.height()))
          << QLine(QPoint(0,img.height()),QPoint(img.width()-1,img.height()))
          << QLine(QPoint(0,1),QPoint(0,img.height() -1));
    pa.drawLines(lines);

    pa.drawPixmap(ICON_BORDER, ICON_BORDER, pixmap);

    QIcon *icon = new QIcon(QPixmap::fromImage(img));
    m_icons.insert(pixmap.cacheKey(), icon);

    return *icon;
}

void IconProvider::requestThumbnail(const DUrl &url)
{
    thumbnailManager->requestThumbnailPixmap(url, ThumbnailGenerator::THUMBNAIL_LARGE,100);
}

void IconProvider::abortRequestThumbnail(const DUrl &url)
{
    thumbnailManager->abortGetThumbnailPixmap(url, ThumbnailGenerator::THUMBNAIL_LARGE, 100);
}

QIcon IconProvider::getFileIcon(const DUrl& fileUrl, const QMimeType &mimeType)
{
//    qDebug() << absoluteFilePath << m_mimeDatabase->mimeTypeForFile(absoluteFilePath).iconName() << FileUtils::getFileMimetype(absoluteFilePath) << getMimeTypeByFile(absoluteFilePath) << mimeType << getFileIcon(absoluteFilePath, 256);
    if (thumbnailManager->canGenerateThumbnail(fileUrl)) {
        return getThumbnail(fileUrl);
    }

    return getIconByMimeType(fileUrl, mimeType);
}


QString IconProvider::getMimeTypeByFile(const QString &file)
{
    QString mimeTypeName;
    if (systemPathManager->isSystemPath(file)){
        mimeTypeName = systemPathManager->getSystemPathIconNameByPath(file);
    }else{
        QMimeType mimeType = m_mimeDatabase->mimeTypeForFile(file);
        mimeTypeName = mimeType.name();
        qDebug() << mimeType.aliases();
    }
    return mimeTypeName;
}
