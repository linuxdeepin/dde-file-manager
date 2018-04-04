

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
    QObject::connect( DSqliteHandle::instance().data(), &DSqliteHandle::backendIsBlocked,
                      adaptor, &TagManagerDaemonAdaptor::backendIsBlocked);

    if(!QDBusConnection::systemBus().registerObject(ObjectPath, this)){
        qFatal("Failed to register object."); //###: log!
    }
}

void TagManagerDaemon::lockBackend()
{
    std::function<void(QSharedPointer<DSqliteHandle> handle)> handle{ &DSqliteHandle::lockBackend };
    std::thread threadForLock{ handle, DSqliteHandle::instance() };
    threadForLock.detach();
}

void TagManagerDaemon::unlockBackend()
{
    DSqliteHandle::instance()->unlockBackend();
}

QDBusVariant TagManagerDaemon::disposeClientData(const QMap<QString, QVariant>& filesAndTags, const QString& userName, const std::size_t& type)
{
    QDBusVariant dbusVar{};

    if(!filesAndTags.isEmpty() && !userName.isEmpty()){
        QMap<QString, QVariant>::const_iterator cbeg{ filesAndTags.cbegin() };
        QMap<QString, QVariant>::const_iterator cend{ filesAndTags.cend() };
        QMap<QString, QList<QString>> filesAndTagsName{};

        for(; cbeg != cend; ++cbeg){
            filesAndTagsName[cbeg.key()] = cbeg.value().toStringList();
        }

//        qDebug()<< filesAndTagsName;

        QVariant var{ DSqliteHandle::instance()->disposeClientData(filesAndTagsName, userName, type) };
        dbusVar.setVariant(var);
    }

    return dbusVar;
}
