
#include <mutex>

#include "tagmanagerdaemoncontroller.h"

#include <QDBusVariant>

static constexpr const  char* const service{ "com.deepin.filemanager.daemon" };
static constexpr const char* const path{ "/com/deepin/filemanager/daemon/TagManagerDaemon" };
static std::once_flag flag{};

std::atomic<bool> flagForLockingBackend{ false };

TagManagerDaemonController::TagManagerDaemonController(QObject * const parent)
                           :QObject{ parent },
                            m_daemonInterface{ nullptr}
{
    m_daemonInterface = std::unique_ptr<TagManagerDaemonInterface>{ new TagManagerDaemonInterface {
                                                                    service,
                                                                    path,
                                                                    QDBusConnection::systemBus()
                                                                                                                                     }
                                                                                            };
}

TagManagerDaemonController* TagManagerDaemonController::instance()
{
    static TagManagerDaemonController* the_instance{ new TagManagerDaemonController };
    return the_instance;
}

QVariant TagManagerDaemonController::disposeClientData(const QVariantMap& filesAndTags, Tag::ActionType type)
{
    QDBusVariant var{ m_daemonInterface->disposeClientData(filesAndTags, static_cast<unsigned long long>(type)) };
    QVariant variant{ var.variant() };

    QDBusArgument argument{ variant.value<QDBusArgument>() };
    QDBusArgument::ElementType current_type{ argument.currentType() };
    QMap<QString, QVariant> var_map{};

    if(current_type == QDBusArgument::ElementType::MapType){
        argument >> var_map;
        variant.setValue(var_map);
    }

    return variant;
}
