// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cifsmounthelper.h"
#include "daemonplugin_mountcontrol_global.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusContext>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QUrl>

#include <polkit-qt5-1/PolkitQt1/Authority>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <libmount/libmount.h>

DAEMONPMOUNTCONTROL_USE_NAMESPACE

static constexpr char kPolicyKitActionId[] { "com.deepin.filemanager.daemon.MountController" };

QVariantMap CifsMountHelper::mount(const QString &path, const QVariantMap &opts)
{
    using namespace MountReturnField;
    if (!path.startsWith("smb://")) {
        qWarning() << "can only mount samba for now.";
        return { { kMountPoint, "" },
                 { kResult, false },
                 { kErrorCode, -kNotSupportedScheme },
                 { kErrorMessage, "smb is only supported scheme now" } };
    }

    QUrl smbUrl(path);
    int port = smbUrl.port();
    const QString &share = smbUrl.path();
    const QString &host = smbUrl.host();
    QString aPath = QString("//%1%2").arg(host).arg(share);

    QString mpt;
    int ret = checkMount(aPath, mpt);
    if (ret == kAlreadyMounted) {
        qDebug() << path << "is already mounted at" << mpt;
        return { { kMountPoint, mpt },
                 { kResult, true },
                 { kErrorCode, 0 },
                 { kErrorMessage, QString("%1 is already mounted at %2").arg(path).arg(mpt) } };
    }

    auto mntPath = generateMountPath(path);
    if (mntPath.isEmpty())
        return { { kMountPoint, "" },
                 { kResult, false },
                 { kErrorCode, -kCannotGenerateMountPath },
                 { kErrorMessage, "cannot generate mount point" } };

    qDebug() << "try to mkdir: " << mntPath;
    if (!mkdir(mntPath)) {
        qDebug() << "cannot mkdir for" << path;
        return { { kMountPoint, "" },
                 { kResult, false },
                 { kErrorCode, -kCannotMkdirMountPoint },
                 { kErrorMessage, "cannot create mount point" + mntPath } };
    } else {
        qDebug() << "try to mount" << path << "on" << mntPath;
    }

    auto params(opts);
    if (port != -1)
        params.insert(MountOptionsField::kPort, port);

    int errNum = 0;
    QString errMsg;
    while (true) {
        auto arg = convertArgs(params);
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
            return { { kMountPoint, mntPath }, { kResult, true }, { kErrorCode, 0 } };
        } else {
            // if params contains 'timeout', remove and retry.
            // the if branch is for compatibility with cifs which does not support the timeout param
            if (params.contains(MountOptionsField::kTimeout)) {
                params.remove(MountOptionsField::kTimeout);
                qInfo() << "mount: remove timeout param and remount...";
                continue;
            } else {
                errNum = errno;
                errMsg = strerror(errNum);
                qWarning() << "mount: failed: " << path << errNum << errMsg;
                qInfo() << "mount: clean dir" << mntPath;
                rmdir(mntPath);
                break;
            }
        }
    }

    return { { kMountPoint, "" }, { kResult, false }, { kErrorCode, errNum }, { kErrorMessage, errMsg } };
}

QVariantMap CifsMountHelper::unmount(const QString &path, const QVariantMap &opts)
{
    Q_UNUSED(opts);
    using namespace MountReturnField;

    QUrl smbUrl(path);
    QString aPath = QString("//%1%2").arg(smbUrl.host()).arg(smbUrl.path());

    QString mpt;
    int ret = checkMount(aPath, mpt);
    if (ret == kNotExist) {
        qDebug() << "mount is not exist: " << path;
        return { { kResult, false },
                 { kErrorCode, -kMountNotExist },
                 { kErrorMessage, path + " is not mounted" } };
    }
    if (ret == kNotOwner && !checkAuth()) {
        qDebug() << "check auth failed: " << path;
        return { { kResult, false },
                 { kErrorCode, -kNotOwnerOfMount },
                 { kErrorMessage, "invoker is not the owner of mount" } };
    }

    ret = ::umount(mpt.toStdString().c_str());
    int err = errno;
    QString errMsg = strerror(errno);
    if (ret != 0)
        qWarning() << "unmount failed: " << path << err << errMsg;
    else
        rmdir(mpt);

    return { { kResult, ret == 0 }, { kErrorCode, err }, { kErrorMessage, errMsg } };
}

CifsMountHelper::MountStatus CifsMountHelper::checkMount(const QString &path, QString &mpt)
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
            return kOkay;
    }

    return kNotExist;
}

QString CifsMountHelper::generateMountPath(const QString &address)
{
    cleanMountPoint();
    if (!mkdirMountRootPath())
        return "";

    // assume that all address is like 'smb://1.2.3.4/share'
    QUrl smbUrl(address);
    QString host = smbUrl.host();
    QString path = smbUrl.path().mid(1);   // remove first /
    int port = smbUrl.port();

    //    smb-share:port=448,server=1.2.3.4,share=share
    QString dirName;
    if (port == -1)
        dirName = QString("smb-share:server=%1,share=%2").arg(host).arg(path);
    else
        dirName = QString("smb-share:port=%1,server=%2,share=%3").arg(port).arg(host).arg(path);
    QString fullPath = QString("%1/%2").arg(mountRoot()).arg(dirName);

    int cnt = 2;
    QString finalFullPath = fullPath;
    while (QDir(finalFullPath).exists()) {   // find a not exist mount path
        finalFullPath += QString(",%1").arg(cnt);
        cnt++;
    }

    return finalFullPath;
}

QString CifsMountHelper::mountRoot()
{
    // if /media/$user/smbmounts does not exist
    auto user = getpwuid(invokerUid());
    if (!user) {
        qWarning() << "cifs: mount root doesn't exist";
        return "";
    }

    const auto &userName = QString(user->pw_name);
    const auto &mntRoot = QString("/media/%1/smbmounts").arg(userName);
    return mntRoot;
}

QString CifsMountHelper::decryptPasswd(const QString &passwd)
{
    // TODO(xust): encrypt and decrypt passwd
    return passwd;
}

uint CifsMountHelper::invokerUid()
{
    Q_ASSERT(context);

    // referenced from Qt, default value is -2 for uid.
    // On Windows and on systems where files do not have owners this function returns ((uint) -2).
    uint uid = -2;
    QDBusConnection c = QDBusConnection::connectToBus(QDBusConnection::SystemBus,
                                                      "org.freedesktop.DBus");
    if (c.isConnected())
        uid = c.interface()->serviceUid(context->message().service()).value();
    return uid;
}

std::string CifsMountHelper::convertArgs(const QVariantMap &opts)
{
    QString param;
    using namespace MountOptionsField;

    if (opts.contains(kUser) && opts.contains(kPasswd) && !opts.value(kUser).toString().isEmpty()
        && !opts.value(kPasswd).toString().isEmpty()) {
        const QString &user = opts.value(kUser).toString();
        const QString &passwd = opts.value(kPasswd).toString();
        param += QString("user=%1,pass=%2,").arg(user).arg(decryptPasswd(passwd));
    } else {
        param += "guest,user=nobody,";   // user is necessary even for anonymous mount
    }

    if (opts.contains(kDomain) && !opts.value(kDomain).toString().isEmpty())
        param += QString("dom=%1,").arg(opts.value(kDomain).toString());

    if (opts.value(kPort, -1).toInt() != -1)
        param += QString("port=%1,").arg(opts.value(kPort).toInt());

    // this param is supported by cifs only.
    if (opts.contains(kTimeout) /* && isTimeoutSupported()*/)
        param += QString("echo_interval=1,wait_reconnect_timeout=%1,")
                         .arg(/*opts.value(kTimeout).toString()*/ 0);

    auto user = getpwuid(invokerUid());
    if (user) {
        param += QString("uid=%1,").arg(user->pw_uid);
        param += QString("gid=%1,").arg(user->pw_gid);
    }
    param += "iocharset=utf8,vers=default";
    return param.toStdString();
}

bool CifsMountHelper::checkAuth()
{
    Q_ASSERT(context);
    QString appBusName = context->message().service();

    if (!appBusName.isEmpty()) {
        using namespace PolkitQt1;
        Authority::Result result = Authority::instance()->checkAuthorizationSync(
                kPolicyKitActionId,
                SystemBusNameSubject(appBusName),   /// 第一个参数是需要验证的action，和规则文件写的保持一致
                Authority::AllowUserInteraction);
        return result == Authority::Yes;
    }
    return false;
}

bool CifsMountHelper::mkdir(const QString &path)
{
    auto aPath = path.toStdString();
    int ret = ::mkdir(aPath.c_str(), 0755);
    if (ret != 0)
        qWarning() << "mkdir failed: " << path << strerror(errno) << errno;
    return ret == 0;
}

bool CifsMountHelper::rmdir(const QString &path)
{
    auto aPath = path.toStdString();
    int ret = ::rmdir(aPath.c_str());
    if (ret != 0)
        qWarning() << "rmdir failed: " << path << strerror(errno) << errno;
    return ret == 0;
}

bool CifsMountHelper::mkdirMountRootPath()
{
    auto mntRoot = mountRoot();
    if (mntRoot.isEmpty()) {
        qWarning() << "cifs: mount root is empty";
        return false;
    }

    auto dir = opendir(mntRoot.toStdString().c_str());
    if (!dir) {
        int ret = ::mkdir(mntRoot.toStdString().c_str(), 0755);
        qInfo() << "mkdir mntRoot: " << mntRoot << "failed: " << strerror(errno) << errno;
        return ret == 0;
    } else {
        closedir(dir);
        return true;
    }
}

void CifsMountHelper::cleanMountPoint()
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
