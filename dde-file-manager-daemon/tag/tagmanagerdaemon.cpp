

#include "app/define.h"
#include "tagmanagerdaemon.h"
#include "shutil/dsqlitehandle.h"
#include "dbusadaptor/tagmanagerdaemon_adaptor.h"

#include <QDebug>

static constexpr const char* ObjectPath{"/com/deepin/filemanager/daemon/TagManagerDaemon"};

std::atomic<int> counter{ 0 };


TagManagerDaemon::TagManagerDaemon(QObject* const parent)
                 :QObject{parent}
{
    adaptor = new TagManagerDaemonAdaptor{ this };

    if(!QDBusConnection::systemBus().registerObject(ObjectPath, this)){
        qFatal("Failed to register object."); //###: log!
    }
}

QDBusVariant TagManagerDaemon::disposeClientData(const QMap<QString, QVariant>& filesAndTags, const unsigned long long& type)
{

    QDBusVariant dbusVar{};

    if(!filesAndTags.isEmpty()){
        QMap<QString, QVariant>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QVariant>::const_iterator cend{ filesAndTags.cend() };
        QMap<QString, QList<QString>> filesAndTagsName{};

        for(; cbeg != cend; ++cbeg){
            filesAndTagsName[cbeg.key()] = cbeg.value().toStringList();
        }

        QVariant var{ DSqliteHandle::instance()->disposeClientData(filesAndTagsName, type) };
        dbusVar.setVariant(var);
    }

    return dbusVar;
}
