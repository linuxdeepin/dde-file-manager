#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = 0);
    ~NetworkManager();

    static void fetch_networks(const QString& path);
    static void network_enumeration_finished (GObject      *source_object,
                                  GAsyncResult *res,
                                  gpointer      user_data);

    static void network_enumeration_next_files_finished (GObject      *source_object,
                                             GAsyncResult *res,
                                             gpointer      user_data);

    static void populate_networks (GFileEnumerator *enumerator, GList *detected_networks);

signals:

public slots:
};

#endif // NETWORKMANAGER_H
