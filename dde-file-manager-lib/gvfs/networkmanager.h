#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QSize>
#include <QDebug>
#include "debugobejct.h"
#include "dfmevent.h"

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

class NetworkNode
{

public:
    explicit NetworkNode();
    ~NetworkNode();

    QString url() const;
    void setUrl(const QString &url);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString iconType() const;
    void setIconType(const QString &iconType);

signals:

public slots:

private:
    QString m_url;
    QString m_displayName;
    QString m_iconType;


    friend QDebug operator<<(QDebug dbg, const NetworkNode &node);
};


QDebug operator<<(QDebug dbg, const NetworkNode &node);


typedef QList<NetworkNode> NetworkNodeList;

class NetworkManager : public QObject
{
    Q_OBJECT

public:


    explicit NetworkManager(QObject *parent = 0);
    ~NetworkManager();

    void initData();
    void initConnect();

    static QStringList SupportScheme;
    static QMap<DUrl, NetworkNodeList> NetworkNodes;
    static GCancellable * m_networks_fetching_cancellable;
    static void fetch_networks(gchar* url, DFMEvent* e);
    static void network_enumeration_finished (GObject      *source_object,
                                  GAsyncResult *res,
                                  gpointer      user_data);

    static void network_enumeration_next_files_finished (GObject      *source_object,
                                             GAsyncResult *res,
                                             gpointer      user_data);

    static void populate_networks (GFileEnumerator *enumerator, GList *detected_networks, gpointer user_data);
    static void restartGVFSD();

signals:

public slots:
    void fetchNetworks(const DFMUrlBaseEvent &event);

private:

};

#endif // NETWORKMANAGER_H
