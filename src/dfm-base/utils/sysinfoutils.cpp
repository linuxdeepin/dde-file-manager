// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sysinfoutils.h"
#include <dfm-base/dfm_global_defines.h>

#include <DSysInfo>

#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>

#include <unistd.h>
#include <fcntl.h>
#include <mutex>

DCORE_USE_NAMESPACE
using namespace dfmbase;

QString SysInfoUtils::getUser()
{
    static QString user = QString::fromLocal8Bit(qgetenv("USER"));

    return user;
}

QStringList SysInfoUtils::getAllUsersOfHome()
{
    QDir homeDir { "/home" };
    static QStringList subDirs { homeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot) };
    return subDirs;
}

QString SysInfoUtils::getHostName()
{
    static QString name;
    if (!name.isEmpty())
        return name;

#if (_XOPEN_SOURCE >= 500) || (_POSIX_C_SOURCE >= 200112L)
    char buf[256] { 0 };
    if (::gethostname(buf, sizeof(buf)) == 0) {
        name = QString(buf);
        return name;
    }
#endif

    return {};
}

int SysInfoUtils::getUserId()
{
    return static_cast<int>(getuid());
}

bool SysInfoUtils::isRootUser()
{
    return getUserId() == 0;
}

bool SysInfoUtils::isServerSys()
{
    return DSysInfo::deepinType() == DSysInfo::DeepinServer;
}

bool SysInfoUtils::isDesktopSys()
{
    return !(SysInfoUtils::isServerSys());
}

bool SysInfoUtils::isOpenAsAdmin()
{
    return SysInfoUtils::isRootUser() && SysInfoUtils::isDesktopSys();
}

bool SysInfoUtils::isDeveloperModeEnabled()
{
    // 为了性能，开发者模式仅需获取一次，不必每次请求 dbus，因此此处使用静态变量
    static bool hasAcquireDevMode = false;
    static bool developerModel = false;

    if (Q_UNLIKELY(!hasAcquireDevMode)) {
        hasAcquireDevMode = true;
        QString service("com.deepin.sync.Helper");
        QString path("/com/deepin/sync/Helper");
        QString interfaceName("com.deepin.sync.Helper");

        QDBusInterface interface(service, path, interfaceName, QDBusConnection::systemBus());

        QString func("IsDeveloperMode");
        QDBusReply<bool> reply = interface.call(func);
        developerModel = reply.value();
    }

    return developerModel;
}

bool SysInfoUtils::isProfessional()
{
    return DSysInfo::deepinType() == DSysInfo::DeepinProfessional;
}

bool SysInfoUtils::isSameUser(const QMimeData *data)
{
    const auto &userKey = QString(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey) + "_" + QString::number(SysInfoUtils::getUserId());
    return data->hasFormat(userKey);
}

void SysInfoUtils::setMimeDataUserId(QMimeData *data)
{
    QByteArray userId;
    QString strUserID = QString::number(getUserId());
    userId.append(strUserID.toUtf8());
    data->setData(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey, userId);

    QString strKey = QString(DFMGLOBAL_NAMESPACE::Mime::kDataUserIDKey) + "_" + strUserID;
    data->setData(strKey, userId);
}

float SysInfoUtils::getMemoryUsage(int pid)
{
    float usage { 0 };
    int fd;
    const size_t bsiz = 1024;
    char path[128] {}, buf[bsiz + 1] {};
    ssize_t nr;

    sprintf(path, "/proc/%d/statm", pid);

    // open /proc/[pid]/statm
    if ((fd = open(path, O_RDONLY)) < 0)
        return usage;

    nr = read(fd, buf, bsiz);
    close(fd);
    if (nr < 0)
        return usage;

    buf[nr] = '\0';
    unsigned long long vmsize;   // vm size in kB
    unsigned long long rss;   // resident set size in kB
    unsigned long long shm;   // resident shared size in kB
    // get resident set size & resident shared size in pages
    nr = sscanf(buf, "%llu %llu %llu", &vmsize, &rss, &shm);
    if (nr == 3) {
        static unsigned int kbShift = 0;
        static std::once_flag flag;
        std::call_once(flag, [] {
            int shift = 0;
            long size;

            /* One can also use getpagesize() to get the size of a page */
            if ((size = sysconf(_SC_PAGESIZE)) == -1)
                return;

            size >>= 10; /* Assume that a page has a minimum size of 1 kB */
            while (size > 1) {
                shift++;
                size >>= 1;
            }

            kbShift = uint(shift);
        });
        // convert to kB
        rss <<= kbShift;
        shm <<= kbShift;

        usage = rss - shm;
    }

    return usage;
}
