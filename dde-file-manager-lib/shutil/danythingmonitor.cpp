
#include "danythingmonitor.h"
#include "tag/tagmanager.h"
#include "dfileinfo.h"


#ifdef __cplusplus
extern "C"
{
#endif

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
}
#endif

#include <QString>



#ifdef __cplusplus
extern "C"
{
#endif


typedef struct __vc_ioctl_readdata_args__ {
    int size;
    char *data;
} ioctl_rd_args;

typedef struct __vc_ioctl_readstat_args__ {
    int total_changes;
    int cur_changes;
    int discarded;
    int cur_memory;
} ioctl_rs_args;

#define VC_IOCTL_MAGIC       0x81
#define VC_IOCTL_READDATA    _IOR(VC_IOCTL_MAGIC, 0, long)
#define VC_IOCTL_READSTAT    _IOR(VC_IOCTL_MAGIC, 1, long)

#ifdef __cplusplus
}
#endif


enum : int {
    ACT_NEW_FILE =  0,
    ACT_NEW_LINK,
    ACT_NEW_SYMLINK,
    ACT_NEW_FOLDER,
    ACT_DEL_FILE,
    ACT_DEL_FOLDER,
    ACT_RENAME_FILE,
    ACT_RENAME_FOLDER

};

//PROTOCOL_HEAD与act_names暂时未使用，为了避免未用警告，暂时屏蔽之
//static const char *const PROTOCOL_HEAD{ "file://" };
static const char *const PROCFS_PATH{ "/proc/vfs_changes" };
//constexpr static const char *const act_names[] {"file_created", "link_created",
//    "symlink_created", "dir_created",
//    "file_deleted", "dir_deleted",
//    "file_renamed", "dir_renamed"
//};



static QString StartPoint{};

DAnythingMonitor::DAnythingMonitor(QObject *const parent)
    : QObject{ parent }
{
    ///###: constructor.
}


void DAnythingMonitor::doWork()
{
    std::unique_lock<std::mutex> raiiLock{ m_mutex };

    m_conditionVar.wait(raiiLock, [this] {
        bool expected{ true };
        return this->m_readyFlag.compare_exchange_strong(expected, false, std::memory_order_seq_cst);
    });

    if (!m_changedFiles.empty()) {
        std::deque<std::pair<QString, QString>>::const_iterator cbeg{ m_changedFiles.cbegin() };
        std::deque<std::pair<QString, QString>>::const_iterator cend{ m_changedFiles.cend() };


        for (; cbeg != cend; ++cbeg) {

            if (cbeg->first.isEmpty()) {

#ifdef QT_DEBUG
                qDebug() << cbeg->second;
#endif
                TagManager::instance()->deleteFiles({DUrl::fromLocalFile(cbeg->second)});

                continue;
            }

            QPair<QByteArray, QByteArray> oldAndNewFileName{ cbeg->first.toLocal8Bit(), cbeg->second.toLocal8Bit() };

#ifdef QT_DEBUG
            qDebug() << oldAndNewFileName;
#endif

            TagManager::instance()->changeFilesName({ oldAndNewFileName });
        }

        m_changedFiles.clear();
    }
}

void DAnythingMonitor::notify() noexcept
{
    bool excepted{ false };
    if (this->m_readyFlag.compare_exchange_strong(excepted, true, std::memory_order_seq_cst)) {
        this->m_conditionVar.notify_one();
    }
}

void DAnythingMonitor::workSignal()
{

    std::lock_guard<std::mutex> raiiLock{ m_mutex };

    int fd = open(PROCFS_PATH, O_RDONLY);
    if (fd < 0) {
        notify();

        return;
    }

    ioctl_rs_args irsa;
    if (ioctl(fd, VC_IOCTL_READSTAT, &irsa) != 0) {
        close(fd);
        notify();

        return;
    }

    if (irsa.cur_changes == 0) {
        close(fd);
        notify();

        return;
    }

    char buf[1 << 20] {};
    ioctl_rd_args ira ;
    ira.data = buf;


    while (true) {
        ira.size = sizeof(buf);
        if (ioctl(fd, VC_IOCTL_READDATA, &ira) != 0) {
            break;
        }

        // no more changes
        if (ira.size == 0) {
            break;
        }

        int off = 0;
        for (int i = 0; i < ira.size; i++) {
            unsigned char action = static_cast<unsigned char>(*(ira.data + off));
            off++;
            char *src = ira.data + off, *dst = nullptr;
            off += strlen(src) + 1;

            switch (action) {
            ///###: do not delete this.
            //                case ACT_NEW_FILE:
            //                case ACT_NEW_SYMLINK:
            //                case ACT_NEW_LINK:
            //                case ACT_NEW_FOLDER:
            //                {
            //                    qDebug()<< act_names[action] << "-------->" << src;
            //                }
            case ACT_DEL_FILE:
            case ACT_DEL_FOLDER: {
                m_changedFiles.emplace_back(QString{}, QString{src});
                //#ifdef QT_DEBUG
                //                    qDebug()<< act_names[action] << "--------->" << src;
                //#endif
                break;
            }
            case ACT_RENAME_FILE:
            case ACT_RENAME_FOLDER: {
                dst = ira.data + off;
                off += strlen(dst) + 1;
                m_changedFiles.emplace_back(QString{src},
                                            QString{dst});
                //#ifdef QT_DEBUG
                //                    qDebug()<< act_names[action] << src << "--------->" << dst;
                //#endif
                break;
            }
            default:
                break;
            }
        }
    }
    close(fd);


    notify();
}







