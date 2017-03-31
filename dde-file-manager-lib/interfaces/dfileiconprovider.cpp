#include "dfileiconprovider.h"
#include "dfileinfo.h"

#include <QLibrary>
#include <QDebug>

#undef signals // Collides with GTK symbols
#include <gtk/gtk.h>

DFM_BEGIN_NAMESPACE

typedef enum {
  GNOME_ICON_LOOKUP_FLAGS_NONE = 0,
  GNOME_ICON_LOOKUP_FLAGS_EMBEDDING_TEXT = 1<<0,
  GNOME_ICON_LOOKUP_FLAGS_SHOW_SMALL_IMAGES_AS_THEMSELVES = 1<<1,
  GNOME_ICON_LOOKUP_FLAGS_ALLOW_SVG_AS_THEMSELVES = 1<<2
} GnomeIconLookupFlags;

typedef enum {
  GNOME_ICON_LOOKUP_RESULT_FLAGS_NONE = 0,
  GNOME_ICON_LOOKUP_RESULT_FLAGS_THUMBNAIL = 1<<0
} GnomeIconLookupResultFlags;

struct GnomeThumbnailFactory;
typedef gboolean (*Ptr_gnome_vfs_init) (void);
typedef char* (*Ptr_gnome_icon_lookup_sync)  (
        GtkIconTheme *icon_theme,
        GnomeThumbnailFactory *,
        const char *file_uri,
        const char *custom_icon,
        GnomeIconLookupFlags flags,
        GnomeIconLookupResultFlags *result);

typedef GtkIconTheme* (*Ptr_gtk_icon_theme_get_default) (void);

class DFileIconProviderPrivate
{
public:
    DFileIconProviderPrivate();

    void init();
    QIcon getFilesystemIcon(const QFileInfo &info) const;
    QIcon fromTheme(QString iconName) const;

    static Ptr_gnome_icon_lookup_sync gnome_icon_lookup_sync;
    static Ptr_gnome_vfs_init gnome_vfs_init;

    static Ptr_gtk_icon_theme_get_default gtk_icon_theme_get_default;
};

Ptr_gnome_icon_lookup_sync DFileIconProviderPrivate::gnome_icon_lookup_sync;
Ptr_gnome_vfs_init DFileIconProviderPrivate::gnome_vfs_init;
Ptr_gtk_icon_theme_get_default DFileIconProviderPrivate::gtk_icon_theme_get_default;

DFileIconProviderPrivate::DFileIconProviderPrivate()
{
    init();
}

void DFileIconProviderPrivate::init()
{
    gnome_icon_lookup_sync = (Ptr_gnome_icon_lookup_sync)QLibrary::resolve(QLatin1String("gnomeui-2"), 0, "gnome_icon_lookup_sync");
    gnome_vfs_init= (Ptr_gnome_vfs_init)QLibrary::resolve(QLatin1String("gnomevfs-2"), 0, "gnome_vfs_init");

    gtk_icon_theme_get_default = (Ptr_gtk_icon_theme_get_default)QLibrary::resolve(QLatin1String("gtk-x11-2.0"), 0, "gtk_icon_theme_get_default");
}

QIcon DFileIconProviderPrivate::getFilesystemIcon(const QFileInfo &info) const
{
    QIcon icon;
    if (gnome_vfs_init && gnome_icon_lookup_sync && gtk_icon_theme_get_default) {
        gnome_vfs_init();
        GtkIconTheme *theme = gtk_icon_theme_get_default();
        QByteArray fileurl = QUrl::fromLocalFile(info.absoluteFilePath()).toEncoded();
        char * icon_name = gnome_icon_lookup_sync(theme,
                                                  NULL,
                                                  fileurl.data(),
                                                  NULL,
                                                  GNOME_ICON_LOOKUP_FLAGS_NONE,
                                                  NULL);
        QString iconName = QString::fromUtf8(icon_name);
        g_free(icon_name);
        if (iconName.startsWith(QLatin1Char('/')))
            return QIcon(iconName);
        return QIcon::fromTheme(iconName);
    }
    return icon;
}

QIcon DFileIconProviderPrivate::fromTheme(QString iconName) const
{
    QIcon icon = QIcon::fromTheme(iconName);

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    if (iconName == "application-vnd.debian.binary-package") {
        iconName = "application-x-deb";
    } else if (iconName == "application-vnd.rar") {
        iconName = "application-zip";
    } else if (iconName == "application-vnd.ms-htmlhelp") {
        iconName = "chmsee";
    } else {
        return icon;
    }

    icon = QIcon::fromTheme(iconName);

    return icon;
}

Q_GLOBAL_STATIC(DFileIconProvider, globalFIP)

DFileIconProvider::DFileIconProvider()
    : d_ptr(new DFileIconProviderPrivate())
{

}

DFileIconProvider::~DFileIconProvider()
{

}

DFileIconProvider *DFileIconProvider::globalProvider()
{
    return globalFIP;
}

QIcon DFileIconProvider::icon(const QFileInfo &info) const
{
    Q_D(const DFileIconProvider);

    return d->getFilesystemIcon(info);
}

QIcon DFileIconProvider::icon(const QFileInfo &info, const QIcon &feedback) const
{
    const QIcon &icon = this->icon(info);

    if (icon.isNull())
        return feedback;

    return icon;
}

QIcon DFileIconProvider::icon(const DFileInfo &info, const QIcon &feedback) const
{
    Q_D(const DFileIconProvider);

    QIcon icon = d->fromTheme(info.iconName());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = this->icon(info.toQFileInfo());

    if (Q_LIKELY(!icon.isNull()))
        return icon;

    icon = d->fromTheme(info.genericIconName());

    if(icon.isNull())
        icon = d->fromTheme("unknown");

    if (icon.isNull())
        return feedback;

    return icon;
}

DFM_END_NAMESPACE
