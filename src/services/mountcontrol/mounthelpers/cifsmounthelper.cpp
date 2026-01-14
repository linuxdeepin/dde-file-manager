// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cifsmounthelper.h"
#include "cifsmounthelper_p.h"
#include "service_mountcontrol_global.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusContext>
#include <QDBusUnixFileDescriptor>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QUrl>

#include <DConfig>

#include <polkit-qt6-1/PolkitQt1/Authority>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <libmount/libmount.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

SERVICEMOUNTCONTROL_USE_NAMESPACE

static constexpr char kPolicyKitActionIdCIFS[] { "org.deepin.Filemanager.MountController.CIFS" };

CifsMountHelper::CifsMountHelper(QDBusContext *context)
    : AbstractMountHelper(context), d(new CifsMountHelperPrivate()) { }

QVariantMap CifsMountHelper::mount(const QString &path, const QVariantMap &opts)
{
    using namespace MountReturnField;
    if (!path.startsWith("smb://")) {
        fmWarning() << "can only mount samba for now.";
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
        fmDebug() << path << "is already mounted at" << mpt;
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

    fmDebug() << "try to mkdir: " << mntPath;
    if (!mkdir(mntPath)) {
        fmDebug() << "cannot mkdir for" << path;
        return { { kMountPoint, "" },
                 { kResult, false },
                 { kErrorCode, -kCannotMkdirMountPoint },
                 { kErrorMessage, "cannot create mount point" + mntPath } };
    } else {
        fmDebug() << "try to mount" << path << "on" << mntPath;
    }

    auto params(opts);
    if (port != -1)
        params.insert(MountOptionsField::kPort, port);

    if (params.contains(MountOptionsField::kTimeout))
        params.insert(MountOptionsField::kTryWaitReconn, true);

    static const QRegularExpression ipRegx(R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    auto matchIp = ipRegx.match(host);
    if (!matchIp.hasMatch()) {
        const QString &ip = d->parseIP(host, port == -1 ? 0 : port);
        if (!ip.isEmpty()) {
            params.insert(MountOptionsField::kIp, ip);
            fmInfo() << "mount: got ip" << ip << "of host" << host;
        }
    }

    const QString &version = d->probeVersion(host, port == -1 ? 0 : port);
    params.insert(MountOptionsField::kVersion, version);

    int errNum = 0;
    QString errMsg;
    while (true) {
        auto arg = convertArgs(params);

        QString args(arg.c_str());
        static QRegularExpression regxCheckPasswd(",pass=.*,dom");
        args.replace(regxCheckPasswd, ",pass=******,dom");
        fmInfo() << "mount: trying mount" << aPath << "on" << mntPath << "with opts:" << args;

        // Improve security
        // MS_NOSUID: Disables the SUID and SGID bits on this file system.
        // MS_NODEV: Disallows access to device files (character devices or block devices) on this file system.
        ret = ::mount(aPath.toStdString().c_str(), mntPath.toStdString().c_str(), "cifs", MS_NOSUID | MS_NODEV,
                      arg.c_str());

        if (ret == 0) {
            fmInfo() << "mount: mount cifs success, params are: " << args;
            return { { kMountPoint, mntPath }, { kResult, true }, { kErrorCode, 0 } };
        } else {
            // if params contains 'timeout', first try mount with `handletimeout` param,
            // if failed, try with `wait_reconnect_timeout` again,
            // if failed, try without any timeout param.
            if (params.contains(MountOptionsField::kTimeout)) {
                if (params.contains(MountOptionsField::kTryWaitReconn)) {
                    fmInfo() << "mount: try with handletimeout";
                    params.remove(MountOptionsField::kTryWaitReconn);
                } else {
                    fmInfo() << "mount: try without timeout param";
                    params.remove(MountOptionsField::kTimeout);
                }
                continue;
            } else {
                errNum = errno;
                errMsg = strerror(errNum);
                fmWarning() << "mount: failed: " << path << errNum << errMsg;
                fmInfo() << "mount: clean dir" << mntPath;
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
        fmDebug() << "mount is not exist: " << path;
        return { { kResult, false },
                 { kErrorCode, -kMountNotExist },
                 { kErrorMessage, path + " is not mounted" } };
    }
    if (ret == kNotOwner) {
        fmInfo() << "invoker is not the owner of mount: " << path;
        return { { kResult, false },
                 { kErrorCode, -kNotOwnerOfMount },
                 { kErrorMessage, "invoker is not the owner of mount" } };
    }

    ret = ::umount(mpt.toStdString().c_str());
    int err = errno;
    QString errMsg = strerror(errno);
    if (ret != 0)
        fmWarning() << "unmount failed: " << path << err << errMsg;
    else
        rmdir(mpt);

    return { { kResult, ret == 0 }, { kErrorCode, err }, { kErrorMessage, errMsg } };
}

bool CifsMountHelper::checkAuthentication(const QString &appName)
{
    return ServiceCommon::PolicyKitHelper::instance()->checkAuthorization(kPolicyKitActionIdCIFS, appName);
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
    fmDebug() << "parse mtab: " << ret;

    std::string aPath = path.toStdString();
    auto fs = mnt_table_find_source(tab, aPath.c_str(), MNT_ITER_BACKWARD);
    if (!fs)
        fs = mnt_table_find_target(tab, aPath.c_str(), MNT_ITER_BACKWARD);

    fmDebug() << "find mount: " << fs << aPath.c_str();
    if (fs) {
        mpt = mnt_fs_get_target(fs);
        fmDebug() << "find mounted at: " << mpt << path;
        if (!mpt.contains(QRegularExpression("^(?:/run/media|/media)/.*")))
            return kNotMountByDaemon;

        QString fsType = mnt_fs_get_fstype(fs);
        if (fsType != "cifs")
            return kNotCifs;

        QStringList opts = QString(mnt_fs_get_options(fs)).split(",");
        fmDebug() << "mount opts:" << opts;

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
        fmWarning() << "cifs: mount root doesn't exist";
        return "";
    }

    const auto &userName = QString(user->pw_name);
    const auto &mntRoot = QString("/media/%1/smbmounts").arg(userName);
    return mntRoot;
}

QString CifsMountHelper::preparePasswd(const QVariant &passwdVar)
{
    // 1. 从 QVariant 中提取 QDBusUnixFileDescriptor
    if (!passwdVar.canConvert<QDBusUnixFileDescriptor>()) {
        fmWarning() << "password is not a file descriptor.";
        return "";
    }

    QDBusUnixFileDescriptor passwdFd = passwdVar.value<QDBusUnixFileDescriptor>();
    if (!passwdFd.isValid()) {
        fmWarning() << "received an invalid file descriptor.";
        return "";
    }

    // 2. 从 QDBusUnixFileDescriptor 中获取原生的 int fd
    int fd = passwdFd.fileDescriptor();

    // 3. 从 FD 中读取密码数据
    QByteArray passwdBytes;
    char buffer[256];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        passwdBytes.append(buffer, bytesRead);
    }

    if (bytesRead == -1) {
        fmWarning() << "fail to read from fd:" << strerror(errno);
    }

    // 4. 关闭 FD
    close(fd);

    // 5. 逗号转义
    QString pwd = QString::fromUtf8(passwdBytes);
    if (pwd.contains(","))
        pwd.replace(",", ",,");

    return pwd;
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
    QStringList params;
    using namespace MountOptionsField;

    QString passwd;
    if (opts.contains(kUser) && opts.contains(kPasswd)
        && !opts.value(kUser).toString().isEmpty()
        && !(passwd = preparePasswd(opts.value(kPasswd))).isEmpty()) {
        const QString &user = opts.value(kUser).toString();
        params.append(QString("user=%1").arg(user));
        params.append(QString("pass=%1").arg(passwd));
    } else {
        params.append("user=");
    }

    if (opts.contains(kDomain) && !opts.value(kDomain).toString().isEmpty())
        params.append(QString("dom=%1").arg(opts.value(kDomain).toString()));

    if (opts.value(kPort, -1).toInt() != -1)
        params.append(QString("port=%1").arg(opts.value(kPort).toInt()));

    if (opts.contains(kIp))
        params.append(QString("ip=%1").arg(opts.value(kIp).toString()));

    auto user = getpwuid(invokerUid());
    if (user) {
        params.append(QString("uid=%1").arg(user->pw_uid));
        params.append(QString("gid=%1").arg(user->pw_gid));
    }

    // params below can be override >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    auto overrides = overrideOptions();

    if (opts.contains(kTimeout)) {
        params.append(option("echo_interval", overrides, "1"));
        if (opts.contains(kTryWaitReconn))
            params.append(option("wait_reconnect_timeout", overrides, "1" /*opts.value(kTimeout).toString()*/));   // w_r_t = ?? s
        else
            params.append(option("handletimeout", overrides, QString::number(opts.value(kTimeout).toInt() * 1000)));   // handletimeout = ?? ms
    }

    params.append(option("iocharset", overrides, "utf8"));
    params.append(option("actimeo", overrides, "5"));   // bug 211337

    QString version = opts.value(MountOptionsField::kVersion, "default").toString();
    params.append(option("vers", overrides, version));

    return params.join(",").toStdString();
}

QVariantMap CifsMountHelper::overrideOptions()
{
    auto config = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                             "org.deepin.dde.file-manager.mount");
    if (!config)
        return {};
    auto overrides = config->value("cifsMountOptionOverride", "{}").toMap();
    config->deleteLater();
    fmInfo() << "override configs:" << overrides;
    return overrides;
}

QString CifsMountHelper::option(const QString &key, const QVariantMap &override, const QString &def)
{
    QString val = def;
    if (override.contains(key)) {
        val = override.value(key).toString();
        fmInfo() << key << "is override with" << val << "while default is" << def;
    }
    return QString("%1=%2").arg(key).arg(val);
}



bool CifsMountHelper::mkdir(const QString &path)
{
    auto aPath = path.toStdString();
    int ret = ::mkdir(aPath.c_str(), 0755);
    if (ret != 0)
        fmWarning() << "mkdir failed: " << path << strerror(errno) << errno;
    return ret == 0;
}

bool CifsMountHelper::rmdir(const QString &path)
{
    auto aPath = path.toStdString();
    int ret = ::rmdir(aPath.c_str());
    if (ret != 0)
        fmWarning() << "rmdir failed: " << path << strerror(errno) << errno;
    return ret == 0;
}

bool CifsMountHelper::mkdirMountRootPath()
{
    auto mntRoot = mountRoot();   // 获取挂载根目录
    if (mntRoot.isEmpty()) {
        fmWarning() << "cifs: mount root is empty";
        return false;
    }

    QDir dir;
    if (dir.exists(mntRoot)) {
        fmInfo() << "cifs: mount root already exists: " << mntRoot;
        return true;
    }

    if (dir.mkpath(mntRoot)) {
        fmInfo() << "cifs: mount root created successfully: " << mntRoot;
        return true;
    } else {
        fmWarning() << "cifs: failed to create mount root: " << mntRoot;
        return false;
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
                fmDebug() << ddd.path() << "was cleaned";
                rmdir(path);
            }
        }
    }
}

SmbcAPI::SmbcAPI()
{
    init();
}

SmbcAPI::~SmbcAPI()
{
    if (smbcCtx && smbcFreeContext) {
        int ret = smbcFreeContext(smbcCtx, true);
        fmInfo() << "free smbc client: " << ret;
    }

    if (libSmbc) {
        if (!libSmbc->unload())
            fmCritical() << "cannot unload smbc";
        delete libSmbc;
    }
    initialized = false;
}

bool SmbcAPI::isInitialized() const
{
    return initialized;
}

void SmbcAPI::init()
{
    if (initialized)
        return;
    libSmbc = new QLibrary("libsmbclient.so.0");
    if (!libSmbc->load()) {
        fmCritical() << "cannot load smbc";
        delete libSmbc;
        libSmbc = nullptr;
        return;
    }

    smbcNewContext = (FnSmbcNewContext)libSmbc->resolve("smbc_new_context");
    smbcFreeContext = (FnSmbcFreeContext)libSmbc->resolve("smbc_free_context");
    smbcNegprot = (FnSmbcNegprot)libSmbc->resolve("smbc_negprot");
    smbcResolveHost = (FnSmbcResolveHost)libSmbc->resolve("smbc_resolve_host");

    smbcCtx = smbcNewContext ? smbcNewContext() : nullptr;

    initialized = (smbcNewContext && smbcFreeContext && smbcNegprot && smbcResolveHost
                   && smbcCtx);

    fmInfo() << "smbc initialized: " << initialized;
}

FnSmbcNegprot SmbcAPI::getSmbcNegprot() const
{
    return smbcNegprot;
}

FnSmbcResolveHost SmbcAPI::getSmbcResolveHost() const
{
    return smbcResolveHost;
}

QMap<QString, QString> SmbcAPI::versionMapper()
{
    static QMap<QString, QString> mapper {
        { "SMB3_11", "3.11" },
        { "SMB3_10", "3.02" },
        { "SMB3_02", "3.02" },
        { "SMB3_00", "3.0" },
        { "SMB2_24", "2.1" },
        { "SMB2_22", "2.1" },
        { "SMB2_10", "2.1" },
        { "SMB2_02", "2.0" },
        { "NT1", "1.0" },
        { "DEFAULT", "default" },
    };
    return mapper;
}

QString CifsMountHelperPrivate::probeVersion(const QString &host, ushort port)
{
    if (!smbcAPI.isInitialized() || !smbcAPI.getSmbcNegprot())
        return "default";

    QString verName = smbcAPI.getSmbcNegprot()(host.toStdString().c_str(),
                                               port,
                                               3000,
                                               "NT1",
                                               "SMB3_11");
    return SmbcAPI::versionMapper().value(verName, "default");
}

QString CifsMountHelperPrivate::parseIP(const QString &host, uint16_t port)
{
    if (!smbcAPI.isInitialized() || !smbcAPI.getSmbcResolveHost())
        return parseIP_old(host);

    char ip[INET6_ADDRSTRLEN];
    int ret = smbcAPI.getSmbcResolveHost()(host.toStdString().c_str(),
                                           port,
                                           3000,
                                           ip,
                                           sizeof(ip));
    if (ret != 0)
        fmWarning() << "cannot resolve ip address for" << host;
    return QString(ip);
}

QString CifsMountHelperPrivate::parseIP_old(const QString &host)
{
    if (host.isEmpty())
        return "";

    addrinfo *result;
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;   // either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    char addressString[INET6_ADDRSTRLEN];
    QString ipAddr;
    if (0 != getaddrinfo(host.toUtf8().toStdString().c_str(), nullptr, &hints, &result))
        return "";

    for (addrinfo *addr = result; addr != nullptr; addr = addr->ai_next) {
        switch (addr->ai_family) {
        case AF_INET:
            ipAddr = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in *>(addr->ai_addr)->sin_addr), addressString, INET_ADDRSTRLEN);
            break;
        case AF_INET6:
            ipAddr = inet_ntop(addr->ai_family, &(reinterpret_cast<sockaddr_in6 *>(addr->ai_addr)->sin6_addr), addressString, INET6_ADDRSTRLEN);
            break;
        default:
            break;
        }
        if (!ipAddr.isEmpty())
            break;
    }

    freeaddrinfo(result);
    return ipAddr;
}
