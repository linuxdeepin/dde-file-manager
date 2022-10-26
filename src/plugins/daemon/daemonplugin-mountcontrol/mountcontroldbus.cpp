/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "mountcontroldbus.h"
#include "private/mountcontroldbus_p.h"
#include "private/mountcontrol_adapter.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>

#include <polkit-qt5-1/PolkitQt1/Authority>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <libmount/libmount.h>

static constexpr char kMountControlObjPath[] { "/com/deepin/filemanager/daemon/MountControl" };
static constexpr char kPolicyKitActionId[] { "com.deepin.filemanager.daemon.MountController" };

DAEMONPMOUNTCONTROL_USE_NAMESPACE

MountControlDBus::MountControlDBus(QObject *parent)
    : QObject(parent), QDBusContext(), d(new MountControlDBusPrivate(this))
{
    // NOTE: A PROBLEM cannot be resolved in short time.
    // if samba is mounted via CIFS and network disconnected between server and client
    // any invokation on the mounted samba file will be blocked forever.
    QDBusConnection::systemBus().registerObject(kMountControlObjPath, this);
}

MountControlDBus::~MountControlDBus() { }

QString MountControlDBus::Mount(const QString &path, const QVariantMap &opts)
{
    if (!path.startsWith("smb://")) {
        qWarning() << "can only mount samba for now.";
        return "";
    }

    QString aPath = path;
    aPath.remove("smb:");   // smb://1.2.3.4/share  ==>  //1.2.3.4/share

    QString mpt;
    int ret = d->checkMount(aPath, mpt);
    if (ret == MountControlDBusPrivate::kAlreadyMounted) {
        qDebug() << path << "is already mounted at" << mpt;
        return mpt;
    }

    auto mntPath = d->genMntPath(path);
    if (mntPath.isEmpty())
        return "";

    qDebug() << "try to mkdir: " << mntPath;
    if (!d->mkdir(mntPath)) {
        qDebug() << "cannot mkdir for" << path;
        return "";
    } else {
        qDebug() << "try to mount" << path << "on" << mntPath;
    }

    auto params(opts);

    while (true) {
        auto arg = d->convertArg(params);
        static QRegularExpression regxLocalhost("^//localhost/");
        if (aPath.contains(regxLocalhost))
            arg = "ip=127.0.0.1," + arg;

        QString args(arg.c_str());
        static QRegularExpression regxCheckPasswd(",pass=.*,dom");
        args.replace(regxCheckPasswd, ",pass=******,dom");
        qInfo() << "mount: trying mount" << aPath << "on" << mntPath << "with opts:" << args;

        ret = ::mount(aPath.toStdString().c_str(), mntPath.toStdString().c_str(), "cifs", 0,
                      arg.c_str());

        if (ret == 0) {
            return mntPath;
        } else {
            // if params contains 'timeout', remove and retry.
            if (params.value(MountOpts::kTimeout, 0).toInt() != 0) {
                params.remove(MountOpts::kTimeout);
                qInfo() << "mount: remove timeout param and remount...";
                continue;
            } else {
                qWarning() << "mount: failed: " << path << strerror(errno) << errno;
                qInfo() << "mount: clean dir" << mntPath;
                d->rmdir(mntPath);
                break;
            }
        }
    }

    return "";
}

bool MountControlDBus::Unmount(const QString &path)
{
    QString aPath = path;
    if (aPath.startsWith("smb://"))
        aPath.remove("smb:");   // smb://1.2.3.4/share ==> //1.2.3.4/share which is the same format
                                // with infos in /proc/mounts

    QString mpt;
    int ret = d->checkMount(path, mpt);
    if (ret == MountControlDBusPrivate::kNotExist) {
        qDebug() << "mount is not exist: " << path;
        return false;
    }
    if (ret == MountControlDBusPrivate::kNotOwner && !d->checkAuth()) {
        qDebug() << "check auth failed: " << path;
        return false;
    }

    ret = ::umount(mpt.toStdString().c_str());
    if (ret != 0)
        qWarning() << "unmount failed: " << path << strerror(errno) << errno;
    else
        d->rmdir(mpt);

    return ret == 0;
}

MountControlDBusPrivate::MountControlDBusPrivate(MountControlDBus *qq)
    : q(qq), adapter(new MountControlAdapter(qq))
{
    clean();
}

MountControlDBusPrivate::~MountControlDBusPrivate()
{
    if (adapter)
        delete adapter;
    clean();
}

bool MountControlDBusPrivate::mkdir(const QString &path)
{
    auto aPath = path.toStdString();
    int ret = ::mkdir(aPath.c_str(), 0755);
    if (ret != 0)
        qWarning() << "mkdir failed: " << path << strerror(errno) << errno;
    return ret == 0;
}

bool MountControlDBusPrivate::rmdir(const QString &path)
{
    auto aPath = path.toStdString();
    int ret = ::rmdir(aPath.c_str());
    if (ret != 0)
        qWarning() << "rmdir failed: " << path << strerror(errno) << errno;
    return ret == 0;
}

uint MountControlDBusPrivate::invokerUid()
{
    // referenced from Qt, default value is -2 for uid.
    // On Windows and on systems where files do not have owners this function returns ((uint) -2).
    uint uid = -2;
    QDBusConnection c =
            QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (c.isConnected())
        uid = c.interface()->serviceUid(q->message().service()).value();
    return uid;
}

std::string MountControlDBusPrivate::convertArg(const QVariantMap &opts)
{
    QString param;
    using namespace MountOpts;

    if (opts.contains(kUser) && opts.contains(kPasswd) && !opts.value(kUser).toString().isEmpty()
        && !opts.value(kPasswd).toString().isEmpty()) {
        const QString &user = opts.value(kUser).toString();
        const QString &passwd = opts.value(kPasswd).toString();
        param += QString("user=%1,pass=%2,").arg(user).arg(decryPasswd(passwd));
    } else {
        param += "guest,user=nobody,";   // user is necessary even for anonymous mount
    }
    if (opts.contains(kDomain) && !opts.value(kDomain).toString().isEmpty())
        param += QString("dom=%1,").arg(opts.value(kDomain).toString());

    // this param is supported by cifs only.
    if (opts.contains(kTimeout) /* && isTimeoutSupported()*/)
        param += QString("echo_interval=1,wait_reconnect_timeout=%1,")
                         .arg(opts.value(kTimeout).toString());

    auto user = getpwuid(invokerUid());
    if (user) {
        param += QString("uid=%1,").arg(user->pw_uid);
        param += QString("gid=%1,").arg(user->pw_gid);
    }
    param += "vers=default";
    return param.toStdString();
}

QString MountControlDBusPrivate::decryPasswd(const QString &passwd)
{
    // TODO(xust): encrypt and decrypt passwd
    return passwd;
}

void MountControlDBusPrivate::clean()
{
    QDir d("/media/");
    auto &&children = d.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs);
    for (const auto &child : children) {
        QDir dd(child.absoluteFilePath() + "/smbmounts");
        if (!dd.exists())
            continue;

        auto &&mnts = dd.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs);
        for (const auto &mnt : mnts) {
            auto &&path = mnt.absoluteFilePath();
            QDir ddd(path);
            if (ddd.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0) {
                qDebug() << ddd.path() << "was cleaned";
                rmdir(path);
            }
        }
    }
}

bool MountControlDBusPrivate::isTimeoutSupported()
{
    QProcess p;
    p.start("bash", QStringList { "-c", "modinfo cifs | grep ^version | awk '{print $2}'" });
    p.waitForFinished(-1);
    auto &&version = QString(p.readAll().trimmed());
    if (version > "")   // TODO(xust) TODO(wangrong)
        return true;
    return false;
}

bool MountControlDBusPrivate::mkdirMntRoot()
{
    // if /media/$user/smbmounts does not exist
    auto user = getpwuid(invokerUid());
    if (!user)
        return false;

    auto userName = QString(user->pw_name);
    auto mntRoot = QString("/media/%1/smbmounts").arg(userName).toStdString();
    if (!opendir(mntRoot.c_str())) {
        int ret = ::mkdir(mntRoot.c_str(), 0755);
        qInfo() << "mkdir mntRoot: " << mntRoot.c_str() << "failed: " << strerror(errno) << errno;
        return ret == 0;
    } else {
        return true;
    }
}

bool MountControlDBusPrivate::checkAuth()
{
    qint64 pid = 0;
    QDBusConnection c =
            QDBusConnection::connectToBus(QDBusConnection::SystemBus, "org.freedesktop.DBus");
    if (c.isConnected())
        pid = c.interface()->servicePid(q->message().service()).value();

    if (pid > 0) {
        using namespace PolkitQt1;
        Authority::Result result = Authority::instance()->checkAuthorizationSync(
                kPolicyKitActionId,
                UnixProcessSubject(pid),   /// 第一个参数是需要验证的action，和规则文件写的保持一致
                Authority::AllowUserInteraction);
        return result == Authority::Yes;
    }
    return false;
}

MountControlDBusPrivate::MntCheckErr MountControlDBusPrivate::checkMount(const QString &path,
                                                                         QString &mpt)
{
    class Helper
    {
    public:
        Helper() { tab = mnt_new_table(); }
        ~Helper() { mnt_free_table(tab); }
        libmnt_table *tab { nullptr };
    };
    Helper d;
    auto tab = d.tab;
    int ret = mnt_table_parse_mtab(tab, nullptr);
    qDebug() << "parse mtab: " << ret;

    std::string aPath = path.toStdString();
    auto fs = mnt_table_find_source(tab, aPath.c_str(), MNT_ITER_BACKWARD);
    if (!fs)
        fs = mnt_table_find_target(tab, aPath.c_str(), MNT_ITER_BACKWARD);

    qDebug() << "find mount: " << fs << aPath.c_str();
    if (fs) {
        mpt = mnt_fs_get_target(fs);
        qDebug() << "find mounted at: " << mpt << path;
        if (!mpt.contains(QRegularExpression("^/media/.*/smbmounts/")))
            return kNotMountByDaemon;

        QString fsType = mnt_fs_get_fstype(fs);
        if (fsType != "cifs")
            return kNotCifs;

        QStringList opts = QString(mnt_fs_get_options(fs)).split(",");
        qDebug() << "mount opts:" << opts;

        auto iter = std::find_if(opts.cbegin(), opts.cend(),
                                 [](const QString &opt) { return opt.startsWith("uid="); });
        if (iter == opts.cend())
            return kNotOwner;
        QString uidArg = *iter;
        uidArg.remove("uid=");
        if (uidArg != QString::number(invokerUid()))
            return kNotOwner;
        else
            return kNoErr;
    }

    return kNotExist;
}

QString MountControlDBusPrivate::genMntPath(const QString &address)
{
    clean();

    // assume that all address is like 'smb://1.2.3.4/share'
    QString addr(address);
    addr.remove("smb://");
    auto frags = addr.split("/");
    if (frags.count() < 2)
        return "";
    QString host = frags.first();
    QString path = frags.at(1);

    if (!mkdirMntRoot())
        return "";

    auto user = getpwuid(invokerUid());
    if (!user)
        return "";

    // make path in /media/$USER/smbmounts/$dirName
    auto userName = QString(user->pw_name);
    QString dirName = QString("%1 on %2").arg(path).arg(host);
    QString fullPath = QString("/media/%1/smbmounts/%2").arg(userName).arg(dirName);

    int cnt = 2;
    QString checkPath = fullPath;
    while (QDir(checkPath).exists()) {   // find a not exist mount path
        checkPath += QString("_%1").arg(cnt);
        cnt++;
    }

    return checkPath;
}
