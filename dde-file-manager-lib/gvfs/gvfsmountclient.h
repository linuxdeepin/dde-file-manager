#ifndef GVFSMOUNTCLIENT_H
#define GVFSMOUNTCLIENT_H

#include <QObject>
#include "dfmevent.h"

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

class MountAskPasswordDialog;

class GvfsMountClient : public QObject
{
    Q_OBJECT

public:
    explicit GvfsMountClient(QObject *parent = 0);
    ~GvfsMountClient();

    void initConnect();

    enum MountOpState{
      MOUNT_OP_NONE,
      MOUNT_OP_ASKED,
      MOUNT_OP_ABORTED
    };

    static bool AskingPassword;
    static QJsonObject SMBLoginObj;
    static DFMUrlBaseEvent MountEvent;
    static MountAskPasswordDialog* AskPasswordDialog;

    static void mount (GFile *file);
    static GMountOperation *new_mount_op(void);
    static void mount_done_cb(GObject *object, GAsyncResult *res, gpointer user_data);
    static void ask_password_cb (GMountOperation *op,
                     const char      *message,
                     const char      *default_user,
                     const char      *default_domain,
                     GAskPasswordFlags flags);

signals:

public slots:
    void mountByPath(const QString& path);
    void mountByEvent(const DFMUrlBaseEvent &event);

};

#endif // GVFSMOUNTCLIENT_H
