#include "networkmanager.h"

#include "app/filesignalmanager.h"
#include "app/define.h"

#include "widgets/singleton.h"

NetworkNode::NetworkNode()
{

}

NetworkNode::~NetworkNode()
{

}

QString NetworkNode::url() const
{
    return m_url;
}

void NetworkNode::setUrl(const QString &url)
{
    m_url = url;
}

QString NetworkNode::displayName() const
{
    return m_displayName;
}

void NetworkNode::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}
QString NetworkNode::iconType() const
{
    return m_iconType;
}

void NetworkNode::setIconType(const QString &iconType)
{
    m_iconType = iconType;
}

QDebug operator<<(QDebug dbg, const NetworkNode &node)
{
    dbg.nospace() << "NetworkNode{"
                  << "url: " << node.url() << ", "
                  << "displayName: " << node.displayName() << ", "
                  << "iconType: " << node.iconType() << ", "
                  << "}";
    return dbg;
}


QMap<DUrl, NetworkNodeList> NetworkManager::NetworkNodes = {};


NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    qDebug() << "Create NetworkManager";
    qRegisterMetaType<NetworkNodeList>(QT_STRINGIFY(NetworkNodeList));
    initData();
    initConnect();
}

NetworkManager::~NetworkManager()
{
    initData();
    initConnect();
}

void NetworkManager::initData()
{

}

void NetworkManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestFetchNetworks, this, &NetworkManager::fetchNetworks);
}


void NetworkManager::fetch_networks(gchar* url, DFMEvent* e)
{
    GFile *network_file;
    network_file = g_file_new_for_uri (url);

    g_file_enumerate_children_async (network_file,
                                      "standard::type,standard::target-uri,standard::name,standard::display-name,standard::icon,mountable::can-mount",
                                      G_FILE_QUERY_INFO_NONE,
                                      G_PRIORITY_DEFAULT,
                                      NULL,
                                      network_enumeration_finished,
                                      e);
}

void NetworkManager::network_enumeration_finished(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GFileEnumerator *enumerator;
    GError *error;

    error = NULL;
    enumerator = g_file_enumerate_children_finish (G_FILE (source_object), res, &error);

    qDebug() << "network_enumeration_finished";

    if (error)
    {
        if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) &&
            !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
            qWarning ("Failed to fetch network locations: %s", error->message);
        qDebug() << error->message;
        DFMEvent* event = static_cast<DFMEvent*>(user_data);
        emit fileSignalManager->requestSMBMount(*event);
        g_clear_error (&error);
    }
    else
    {
        g_file_enumerator_next_files_async (enumerator,
                                          G_MAXINT32,
                                          G_PRIORITY_DEFAULT,
                                          NULL,
                                          network_enumeration_next_files_finished,
                                          user_data);
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
        populate_networks (G_FILE_ENUMERATOR (source_object), detected_networks, user_data);

        g_list_free_full (detected_networks, g_object_unref);
    }
}

void NetworkManager::populate_networks(GFileEnumerator *enumerator, GList *detected_networks, gpointer user_data)
{
    GList *l;
    GFile *file;
    GFile *activatable_file;
    gchar *uri;
    GFileType type;
    GIcon *icon;
//    gchar *name;
    gchar *display_name;
    gchar* iconPath;

    NetworkNodeList nodeList;

    for (l = detected_networks; l != NULL; l = l->next)
    {
        GFileInfo* fileInfo = static_cast<GFileInfo*>(l->data);
        file = g_file_enumerator_get_child (enumerator, fileInfo);
        type = g_file_info_get_file_type (fileInfo);
        if (type == G_FILE_TYPE_SHORTCUT || type == G_FILE_TYPE_MOUNTABLE)
            uri = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
        else
            uri = g_file_get_uri (file);

        if (!QString(uri).startsWith("smb://")){
            continue;
        }

        activatable_file = g_file_new_for_uri (uri);
//        name = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_NAME);
        display_name = g_file_info_get_attribute_as_string (fileInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
        icon = g_file_info_get_icon (fileInfo);
        iconPath = g_icon_to_string(icon);


        NetworkNode node;
        node.setUrl(QString(uri));
        node.setDisplayName(QString(display_name));
        node.setIconType(QString(iconPath));

        qDebug() << node;

        nodeList.append(node);

        g_free (uri);
        g_free (display_name);
        g_free (iconPath);
        g_clear_object (&file);
        g_clear_object (&activatable_file);
    }

    DFMEvent* event = static_cast<DFMEvent*>(user_data);
    NetworkNodes.remove(event->fileUrl());
    NetworkNodes.insert(event->fileUrl(), nodeList);
    qDebug() << "request NetworkNodeList successfully";
    emit fileSignalManager->fetchNetworksSuccessed(*event);
}

void NetworkManager::fetchNetworks(const DFMEvent &event)
{
    qDebug() << event;
    DFMEvent* e = new DFMEvent(event);
    QString path = event.fileUrl().toString();
    std::string stdPath = path.toStdString();
    gchar *url = const_cast<char*>(stdPath.c_str());
    fetch_networks(url, e);
}
