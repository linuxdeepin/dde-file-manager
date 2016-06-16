#include "networkmanager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{

}

NetworkManager::~NetworkManager()
{

}

void NetworkManager::fetch_networks(const QString& path)
{
    GFile *network_file;
    network_file = g_file_new_for_uri (path.toStdString().c_str());

    g_file_enumerate_children_async (network_file,
                                      "standard::type,standard::target-uri,standard::name,standard::display-name,standard::icon,mountable::can-mount",
                                      G_FILE_QUERY_INFO_NONE,
                                      G_PRIORITY_DEFAULT,
                                      NULL,
                                      network_enumeration_finished,
                                     NULL);
}

void NetworkManager::network_enumeration_finished(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GFileEnumerator *enumerator;
    GError *error;

    error = NULL;
    enumerator = g_file_enumerate_children_finish (G_FILE (source_object), res, &error);

    if (error)
    {
        if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
            !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
            qWarning ("Failed to fetch network locations: %s", error->message);
        g_clear_error (&error);
    }
    else
    {
        g_file_enumerator_next_files_async (enumerator,
                                          G_MAXINT32,
                                          G_PRIORITY_DEFAULT,
                                          NULL,
                                          network_enumeration_next_files_finished,
                                          NULL);
    }
}

void NetworkManager::network_enumeration_next_files_finished(GObject *source_object, GAsyncResult *res, gpointer user_data)
{

    GList *detected_networks;
    GError *error;

    error = NULL;

    detected_networks = g_file_enumerator_next_files_finish (G_FILE_ENUMERATOR (source_object),
                                                           res, &error);

    if (error)
    {
        if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            qWarning ("Failed to fetch network locations: %s", error->message);
        g_clear_error (&error);
    }
    else
    {
        populate_networks (G_FILE_ENUMERATOR (source_object), detected_networks);

        g_list_free_full (detected_networks, g_object_unref);
    }
}

void NetworkManager::populate_networks(GFileEnumerator *enumerator, GList *detected_networks)
{
    GList *l;
    GFile *file;
    GFile *activatable_file;
    gchar *uri;
    GFileType type;
    GIcon *icon;
    gchar *name;
    gchar *display_name;
    gchar* iconPath;

    for (l = detected_networks; l != NULL; l = l->next)
    {
        GFileInfo* fileInfo = static_cast<GFileInfo*>(l->data);
        file = g_file_enumerator_get_child (enumerator, fileInfo);
        type = g_file_info_get_file_type (fileInfo);
        if (type == G_FILE_TYPE_SHORTCUT || type == G_FILE_TYPE_MOUNTABLE)
            uri = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
        else
            uri = g_file_get_uri (file);

        activatable_file = g_file_new_for_uri (uri);
        name = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_NAME);
        display_name = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
        icon = g_file_info_get_icon (fileInfo);
        iconPath = g_icon_to_string(icon);


        qDebug() << uri  << display_name << iconPath << type;

        g_free (uri);
        g_free (display_name);
        g_free (iconPath);
        g_clear_object (&file);
        g_clear_object (&activatable_file);
    }
}
