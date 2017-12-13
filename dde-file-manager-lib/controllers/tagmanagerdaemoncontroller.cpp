
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
    m_daemonInterface = std::unique_ptr<com::deepin::filemanager::daemon::TagManagerDaemon>{
                                                                            new com::deepin::filemanager::daemon::TagManagerDaemon {
                                                                            service,
                                                                            path,
                                                                             QDBusConnection::systemBus()
                                                                                                                                     }
                                                                                            };
    QObject::connect(m_daemonInterface.get(), &com::deepin::filemanager::daemon::TagManagerDaemon::backendIsBlocked,
                     [&]{ flagForLockingBackend.store(true, std::memory_order_release);});
}

QSharedPointer<TagManagerDaemonController> TagManagerDaemonController::instance()
{
    static QSharedPointer<TagManagerDaemonController> theInstance{ nullptr };
    std::call_once(flag, [&]{theInstance.reset(new TagManagerDaemonController); });
    return theInstance;
}

void TagManagerDaemonController::lockBackend()
{
    m_daemonInterface->lockBackend();
}

bool TagManagerDaemonController::isLocked()const noexcept
{
    return flagForLockingBackend.load(std::memory_order_consume);
}

void TagManagerDaemonController::unlockBackend()
{
    m_daemonInterface->unlockBackend();
    flagForLockingBackend.store(false, std::memory_order_release);
}

QVariant TagManagerDaemonController::disposeClientData(const QVariantMap& filesAndTags, const QString& userName, Tag::ActionType type)
{
    QDBusVariant var{ m_daemonInterface->disposeClientData(filesAndTags, userName, static_cast<unsigned long long>(type)) };
    return var.variant();
}
