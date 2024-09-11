// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceutils.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-io/dfile.h>
#include <dfm-burn/dburn_global.h>
#include <dfm-io/dfmio_utils.h>

#include <QVector>
#include <QDebug>
#include <QRegularExpressionMatch>
#include <QMutex>
#include <QSettings>

#include <libmount.h>
#include <fstab.h>
#include <sys/stat.h>

using namespace dfmbase;
using namespace GlobalServerDefines::DeviceProperty;
DFM_BURN_USE_NS

QString DeviceUtils::getBlockDeviceId(const QString &deviceDesc)
{
    QString dev(deviceDesc);
    if (dev.startsWith("/dev/"))
        dev.remove("/dev/");
    return kBlockDeviceIdPrefix + dev;
}

/*!
 * \brief DeviceUtils::getMountInfo
 * \param in: the mount src or target, /dev/sda is a source and /media/$USER/sda is target, e.g.
 * \param lookForMpt: if setted to true, then treat 'in' like a mount source
 * \return if lookForMpt is setted to true, then returns the mount target, otherwise returns mount source.
 */
QString DeviceUtils::getMountInfo(const QString &in, bool lookForMpt)
{
    if (in.isEmpty())
        return {};
    libmnt_table *tab { mnt_new_table() };
    if (!tab)
        return {};
    FinallyUtil finally { [tab]() { if (tab) mnt_free_table(tab); } };
    if (mnt_table_parse_mtab(tab, nullptr) != 0) {
        qCWarning(logDFMBase) << "Invalid mnt_table_parse_mtab call";
        return {};
    }

    auto query = lookForMpt ? mnt_table_find_source : mnt_table_find_target;
    auto get = lookForMpt ? mnt_fs_get_target : mnt_fs_get_source;
    std::string stdPath { in.toStdString() };
    auto fs = query(tab, stdPath.c_str(), MNT_ITER_BACKWARD);
    if (fs)
        return { get(fs) };

    qCWarning(logDFMBase) << "Invalid libmnt_fs*";
    return {};
}

QUrl DeviceUtils::getSambaFileUriFromNative(const QUrl &url)
{
    if (!url.isValid())
        return QUrl();

    if (!DeviceUtils::isSamba(url))
        return url;

    QUrl smbUrl;
    smbUrl.setScheme(Global::Scheme::kSmb);

    QString host, share;
    QString fullPath = url.path();
    if (!fullPath.endsWith("/"))
        fullPath.append("/");
    bool parseReuslt = DeviceUtils::parseSmbInfo(fullPath, host, share);
    if (!parseReuslt)
        return url;

    //  /run/user/1000/gvfs/smb-share...../helloworld.txt
    //  /root/.gvfs/smb-share...../helloworld.txt
    //  /media/user/smbmounts/smb-share...../helloworld.txt
    //  ======>  helloworld.txt
    static const QRegularExpression prefix(R"(^/run/user/.*/gvfs/[^/]*/|^/root/.gvfs/[^/]*/|^/media/.*/smbmounts/[^/]*/)");
    QString fileName = fullPath.remove(prefix);
    fileName.chop(1);   // remove last '/'.

    smbUrl.setHost(host);
    fileName.isEmpty() ? smbUrl.setPath("/" + share) : smbUrl.setPath("/" + share + "/" + fileName);
    return smbUrl;
}

QString DeviceUtils::errMessage(dfmmount::DeviceError err)
{
    return DFMMOUNT::Utils::errorMessage(err);
}

/*!
 * \brief DeviceUtils::convertSuitableDisplayName
 * \param devInfo which is obtained by DeviceManager/DeviceProxyManger
 * \return a suitable device name,
 * if device's idLabel is empty, get the display name by size (if size is not 0) or Empty XXX disc (if it is empty disc)
 * and this function should never returns an empty string, if that happened, please check your input.
 */
QString DeviceUtils::convertSuitableDisplayName(const QVariantMap &devInfo)
{
    QString uuid = devInfo.value(kUUID).toString();
    auto clearDevInfo = devInfo.value(BlockAdditionalProperty::kClearBlockProperty).toMap();
    if (!clearDevInfo.isEmpty())
        uuid = clearDevInfo.value(kUUID, uuid).toString();

    QString alias = nameOfAlias(uuid);
    if (!alias.isEmpty())
        return alias;

    QVariantMap clearInfo = devInfo.value(BlockAdditionalProperty::kClearBlockProperty).toMap();
    QString mpt = clearInfo.value(kMountPoint, devInfo.value(kMountPoint).toString()).toString();
    QString idLabel = clearInfo.value(kIdLabel, devInfo.value(kIdLabel).toString()).toString();
    // NOTE(xust): removable/hintSystem is not always correct in some certain hardwares.
    if (mpt == "/" || idLabel.startsWith("_dde_")) {
        return nameOfSystemDisk(devInfo);
    } else if (devInfo.value(kIsEncrypted).toBool()) {
        return nameOfEncrypted(devInfo);
    } else if (devInfo.value(kOpticalDrive).toBool()) {
        return nameOfOptical(devInfo);
    } else {
        const QString &&label = devInfo.value(kIdLabel).toString();
        quint64 size = devInfo.value(kSizeTotal).toULongLong();
        return nameOfDefault(label, size);
    }
}

QString DeviceUtils::convertSuitableDisplayName(const QVariantHash &devInfo)
{
    QVariantMap map;
    for (auto iter = devInfo.cbegin(); iter != devInfo.cend(); ++iter)
        map.insert(iter.key(), iter.value());
    return convertSuitableDisplayName(map);
}

QString DeviceUtils::formatOpticalMediaType(const QString &media)
{
    static std::initializer_list<std::pair<QString, QString>> opticalmediakeys {
        { "optical", "Optical" },
        { "optical_cd", "CD-ROM" },
        { "optical_cd_r", "CD-R" },
        { "optical_cd_rw", "CD-RW" },
        { "optical_dvd", "DVD-ROM" },
        { "optical_dvd_r", "DVD-R" },
        { "optical_dvd_rw", "DVD-RW" },
        { "optical_dvd_ram", "DVD-RAM" },
        { "optical_dvd_plus_r", "DVD+R" },
        { "optical_dvd_plus_rw", "DVD+RW" },
        { "optical_dvd_plus_r_dl", "DVD+R/DL" },
        { "optical_dvd_plus_rw_dl", "DVD+RW/DL" },
        { "optical_bd", "BD-ROM" },
        { "optical_bd_r", "BD-R" },
        { "optical_bd_re", "BD-RE" },
        { "optical_hddvd", "HD DVD-ROM" },
        { "optical_hddvd_r", "HD DVD-R" },
        { "optical_hddvd_rw", "HD DVD-RW" },
        { "optical_mo", "MO" }
    };
    static QMap<QString, QString> opticalmediamap(opticalmediakeys);

    return opticalmediamap.value(media);
}

bool DeviceUtils::isAutoMountEnable()
{
    return Application::genericAttribute(Application::GenericAttribute::kAutoMount).toBool();
}

bool DeviceUtils::isAutoMountAndOpenEnable()
{
    return Application::genericAttribute(Application::GenericAttribute::kAutoMountAndOpen).toBool();
}

bool DeviceUtils::isWorkingOpticalDiscDev(const QString &dev)
{
    static constexpr char kBurnStateGroup[] { "BurnState" };
    static constexpr char kWoringKey[] { "Working" };

    if (dev.isEmpty())
        return false;

    if (Application::dataPersistence()->keys(kBurnStateGroup).contains(dev)) {
        const QMap<QString, QVariant> &info = Application::dataPersistence()->value(kBurnStateGroup, dev).toMap();
        return info.value(kWoringKey).toBool();
    }
    return false;
}

bool DeviceUtils::isWorkingOpticalDiscId(const QString &id)
{
    static constexpr char kBurnStateGroup[] { "BurnState" };
    static constexpr char kWoringKey[] { "Working" };
    static constexpr char kID[] { "id" };

    if (id.isEmpty())
        return false;

    auto &&keys { Application::dataPersistence()->keys(kBurnStateGroup) };
    for (const QString &dev : keys) {
        const QMap<QString, QVariant> &info = Application::dataPersistence()->value(kBurnStateGroup, dev).toMap();
        QString &&devID { info.value(kID).toString() };
        if (devID == id)
            return info.value(kWoringKey).toBool();
    }

    return false;
}

bool DeviceUtils::isBlankOpticalDisc(const QString &id)
{
    // for dvd+rw/dvd-rw/bd-re disc, erase operation only overwrite some blocks which used to present filesystem,
    // so the blank field is still false even if it can be write datas from the beginning,
    auto &&map = DevProxyMng->queryBlockInfo(id);
    bool isBlank { map[kOpticalBlank].toBool() };
    auto mediaType { static_cast<MediaType>(map[kOpticalMediaType].toUInt()) };
    if (mediaType == MediaType::kDVD_PLUS_RW || mediaType == MediaType::kDVD_RW || mediaType == MediaType::kBD_RE)
        isBlank |= map[kSizeTotal].toULongLong() == map[kSizeFree].toULongLong();

    return isBlank;
}

bool DeviceUtils::isPWOpticalDiscDev(const QString &dev)
{
    // PW = packet writing
    using namespace GlobalServerDefines;
    if (!dev.startsWith("/dev/sr"))
        return false;
    const QString &id { DeviceUtils::getBlockDeviceId(dev) };
    const auto &map { DevProxyMng->queryBlockInfo(id) };

    auto fs { qvariant_cast<QString>(map[DeviceProperty::kFileSystem]) };
    if (fs != "udf")
        return false;
    auto fsVersion { qvariant_cast<QString>(map[DeviceProperty::kFsVersion]) };
    if (fsVersion != "2.01")
        return false;
    auto mediaType { DeviceUtils::formatOpticalMediaType(map.value(DeviceProperty::kMedia).toString()) };
    if (mediaType != "DVD+RW" && mediaType != "DVD-RW")
        return false;
    return true;
}

bool DeviceUtils::isPWUserspaceOpticalDiscDev(const QString &dev)
{
    if (!dev.startsWith("/dev/sr"))
        return false;
    using namespace GlobalServerDefines;
    const QString &id { DeviceUtils::getBlockDeviceId(dev) };
    const auto &map { DevProxyMng->queryBlockInfo(id) };
    auto mediaType { DeviceUtils::formatOpticalMediaType(map.value(DeviceProperty::kMedia).toString()) };
    if (mediaType != "DVD-RW")
        return false;
    return isPWOpticalDiscDev(dev);
}

bool DeviceUtils::isSamba(const QUrl &url)
{
    if (url.scheme() == Global::Scheme::kSmb)
        return true;
    static const QString smbMatch { "(^/run/user/\\d+/gvfs/smb|^/root/\\.gvfs/smb|^/media/[\\s\\S]*/smbmounts)" };   // TODO(xust) /media/$USER/smbmounts might be changed in the future.}
    return hasMatch(url.path(), smbMatch);
}

bool DeviceUtils::isFtp(const QUrl &url)
{
    static const QString smbMatch { "(^/run/user/\\d+/gvfs/s?ftp|^/root/\\.gvfs/s?ftp)" };
    return hasMatch(url.path(), smbMatch);
}

bool DeviceUtils::isSftp(const QUrl &url)
{
    static const QString smbMatch { "(^/run/user/\\d+/gvfs/sftp|^/root/\\.gvfs/sftp)" };
    return hasMatch(url.path(), smbMatch);
}

bool DeviceUtils::isMtpFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { R"(^/run/user/\d+/gvfs/mtp:host|^/root/.gvfs/mtp:host)" };
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}

bool DeviceUtils::supportDfmioCopyDevice(const QUrl &url)
{
    if (!url.isValid())
        return false;

    return !isMtpFile(url);
}

bool DeviceUtils::supportSetPermissionsDevice(const QUrl &url)
{
    if (!url.isValid())
        return false;

    return !isMtpFile(url);
}

bool DeviceUtils::isExternalBlock(const QUrl &url)
{
    return DeviceProxyManager::instance()->isFileOfExternalBlockMounts(url.path());
}

QUrl DeviceUtils::parseNetSourceUrl(const QUrl &target)
{
    if (!isSamba(target) && !isFtp(target))
        return {};

    QString host, port;
    NetworkUtils::instance()->parseIp(target.path(), host, port);
    if (host.isEmpty())
        return {};

    QString protocol, share;
    if (isSamba(target)) {
        protocol = "smb";
        static const QRegularExpression regxSmb(R"(,share=([^,/]*))");
        auto match = regxSmb.match(target.path());
        if (match.hasMatch())
            share = match.captured(1);
        else
            return {};
    } else {
        protocol = isSftp(target) ? "sftp" : "ftp";
    }

    static const QRegularExpression prefix(R"(^/run/user/.*/gvfs/[^/]*|^/media/.*/smbmounts/[^/]*)");
    QString dirPath = target.path();
    dirPath.remove(prefix);
    dirPath.prepend(share);
    if (!dirPath.startsWith("/"))
        dirPath.prepend("/");

    QUrl src;
    src.setScheme(protocol);
    src.setHost(host);
    src.setPath(dirPath);

    return src;
}

bool DeviceUtils::parseSmbInfo(const QString &smbPath, QString &host, QString &share, QString *port)
{
    static const QRegularExpression regx(R"(([:,]port=(?<port>\d*))?[,:]server=(?<host>[^/:,]+)(,share=(?<share>[^/:,]+))?)");
    auto match = regx.match(smbPath);
    if (!match.hasMatch())
        return false;

    host = match.captured("host");
    share = match.captured("share");
    if (port)
        *port = match.captured("port");
    return true;
}

QMap<QString, QString> DeviceUtils::fstabBindInfo()
{
    // TODO(perf) this costs times when first painting. most of the time is spent on function 'stat'
    static QMutex mutex;
    static QMap<QString, QString> table;
    struct stat statInfo;
    int result = stat("/etc/fstab", &statInfo);

    QMutexLocker locker(&mutex);
    if (0 == result) {
        static quint32 lastModify = 0;
        if (lastModify != statInfo.st_mtime) {
            lastModify = static_cast<quint32>(statInfo.st_mtime);
            table.clear();
            struct fstab *fs;

            setfsent();
            while ((fs = getfsent()) != nullptr) {
                QString mntops(fs->fs_mntops);
                if (mntops.contains("bind"))
                    table.insert(fs->fs_spec, fs->fs_file);
            }
            endfsent();
        }
    }

    return table;
}

QString DeviceUtils::nameOfSystemDisk(const QVariantMap &datas)
{
    QVariantMap clearInfo = datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap();

    QString mountPoint = clearInfo.value(kMountPoint, datas.value(kMountPoint)).toString();
    QString label = clearInfo.value(kIdLabel, datas.value(kIdLabel)).toString();
    qlonglong size = datas.value(kSizeTotal).toLongLong();

    // get system disk name if there is no alias
    if (mountPoint == "/")
        return QObject::tr("System Disk");
    if (!mountPoint.startsWith("/media/") && !mountPoint.isEmpty()) {
        if (label.startsWith("_dde_data"))
            return QObject::tr("Data Disk");
        if (label.startsWith("_dde_"))
            return datas.value(kIdLabel).toString().mid(5);
    }
    return nameOfDefault(label, size);
}

QString DeviceUtils::nameOfOptical(const QVariantMap &datas)
{
    QString label = datas.value(kIdLabel).toString();
    if (!label.isEmpty())
        return label;

    static const std::initializer_list<std::pair<QString, QString>> opticalMedias {
        { "optical", "Optical" },
        { "optical_cd", "CD-ROM" },
        { "optical_cd_r", "CD-R" },
        { "optical_cd_rw", "CD-RW" },
        { "optical_dvd", "DVD-ROM" },
        { "optical_dvd_r", "DVD-R" },
        { "optical_dvd_rw", "DVD-RW" },
        { "optical_dvd_ram", "DVD-RAM" },
        { "optical_dvd_plus_r", "DVD+R" },
        { "optical_dvd_plus_rw", "DVD+RW" },
        { "optical_dvd_plus_r_dl", "DVD+R/DL" },
        { "optical_dvd_plus_rw_dl", "DVD+RW/DL" },
        { "optical_bd", "BD-ROM" },
        { "optical_bd_r", "BD-R" },
        { "optical_bd_re", "BD-RE" },
        { "optical_hddvd", "HD DVD-ROM" },
        { "optical_hddvd_r", "HD DVD-R" },
        { "optical_hddvd_rw", "HD DVD-RW" },
        { "optical_mo", "MO" }
    };
    static const QMap<QString, QString> discMapper(opticalMedias);
    static const QVector<std::pair<QString, QString>> discVector(opticalMedias);

    auto totalSize { datas.value(kSizeTotal).toULongLong() };

    if (datas.value(kOptical).toBool()) {   // medium loaded
        if (datas.value(kOpticalBlank).toBool()) {   // show empty disc name
            QString mediaType = datas.value(kMedia).toString();
            return QObject::tr("Blank %1 Disc").arg(discMapper.value(mediaType, QObject::tr("Unknown")));
        } else {
            // totalSize changed after disc mounted
            auto udiks2Size { datas.value(kUDisks2Size).toULongLong() };
            return nameOfDefault(label, udiks2Size > 0 ? udiks2Size : totalSize);
        }
    } else {   // show drive name, medium is not loaded
        auto medias = datas.value(kMediaCompatibility).toStringList();
        for (auto iter = discVector.crbegin(); iter != discVector.crend(); ++iter) {
            if (medias.contains(iter->first))
                return QObject::tr("%1 Drive").arg(iter->second);
        }
    }

    return nameOfDefault(label, totalSize);
}

QString DeviceUtils::nameOfEncrypted(const QVariantMap &datas)
{
    if (datas.value(kCleartextDevice).toString().length() > 1
        && !datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap().isEmpty()) {
        auto clearDevData = datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap();
        QString clearDevLabel = clearDevData.value(kIdLabel).toString();
        qlonglong clearDevSize = clearDevData.value(kSizeTotal).toLongLong();
        return nameOfDefault(clearDevLabel, clearDevSize);
    } else {
        QString bacDevName = datas.value(kIdLabel, "").toString();
        if (bacDevName.isEmpty())
            bacDevName = nameOfSize(datas.value(kSizeTotal).toLongLong());
        return QObject::tr("%1 Encrypted")
                .arg(bacDevName);
    }
}

QString DeviceUtils::nameOfDefault(const QString &label, const quint64 &size)
{
    if (label.isEmpty())
        return QObject::tr("%1 Volume").arg(nameOfSize(size));
    return label;
}

/*!
 * \brief DeviceUtils::nameOfSize
 * \param size
 * \return
 * infact this function is basically the same as formatSize in FileUtils, but I don't want import any other
 * dfm-base files except device*, so I make a simple copy here.
 */
QString DeviceUtils::nameOfSize(const quint64 &size)
{
    quint64 num = size;
    if (num < 0) {
        qCWarning(logDFMBase) << "Negative number passed to formatSize():" << num;
        num = 0;
    }

    QStringList list;
    qreal fileSize(num);

    list << "B"
         << "KB"
         << "MB"
         << "GB"
         << "TB";   // should we use KiB since we use 1024 here?

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral("B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    return QString("%1 %2").arg(QString::number(fileSize, 'f', 1)).arg(unit);
}

QString DeviceUtils::nameOfAlias(const QString &uuid)
{
    const auto &lst = Application::genericSetting()->value(BlockAdditionalProperty::kAliasGroupName, BlockAdditionalProperty::kAliasItemName).toList();
    for (const QVariant &v : lst) {
        const QVariantMap &map = v.toMap();
        if (map.value(BlockAdditionalProperty::kAliasItemUUID).toString() == uuid) {
            return map.value(BlockAdditionalProperty::kAliasItemAlias).toString();
        }
    }
    return "";
}

bool DeviceUtils::checkDiskEncrypted()
{
    static bool isEncrypted = false;
    static std::once_flag flag;

    std::call_once(flag, [&] {
        QSettings settings("/etc/deepin/deepin-user-experience", QSettings::IniFormat);
        isEncrypted = settings.value("ExperiencePlan/FullDiskEncrypt", false).toBool();
    });

    return isEncrypted;
}

QStringList DeviceUtils::encryptedDisks()
{
    static QStringList deviceList;
    static std::once_flag flag;

    std::call_once(flag, [&] {
        // The `deepin-installer.conf` file is either in the `/etc` directory
        // or in the `/etc/deepin-installer` directory
        DFMIO::DFile file("/etc/deepin-installer.conf");
        QString conf = file.exists() ? "/etc/deepin-installer.conf" : "/etc/deepin-installer/deepin-installer.conf";

        QSettings settings(conf, QSettings::IniFormat);
        const QString &value = settings.value("DI_CRYPT_INFO", "").toString();
        if (!value.isEmpty()) {
            QStringList groupList = value.split(';');
            for (const auto &group : groupList) {
                QStringList device = group.split(':');
                if (!device.isEmpty())
                    deviceList << device.first();
            }
        }
    });

    return deviceList;
}

bool DeviceUtils::isSubpathOfDlnfs(const QString &path)
{
    return findDlnfsPath(path, [](const QString &target, const QString &compare) {
        return target.startsWith(compare);
    });
}

bool DeviceUtils::isMountPointOfDlnfs(const QString &path)
{
    return findDlnfsPath(path, [](const QString &target, const QString &compare) {
        return target == compare;
    });
}

bool DeviceUtils::isLowSpeedDevice(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString lowSpeedMountpoint { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/|^/media/[\\s\\S]*/smbmounts)" };
    // TODO(xust) /media/$USER/smbmounts might be changed in the future.
    QRegularExpression re { lowSpeedMountpoint };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}

/*!
 * \brief DeviceUtils::getLongestMountRootPath: get the mount root of a file `filePath`
 * return `/home/` for `/home/helloworld.txt`, eg.
 * \param filePath
 * \return
 */
QString DeviceUtils::getLongestMountRootPath(const QString &filePath)
{
    libmnt_table *tab { mnt_new_table() };
    libmnt_iter *iter { mnt_new_iter(MNT_ITER_BACKWARD) };

    FinallyUtil release([&] {
        if (tab) mnt_free_table(tab);
        if (iter) mnt_free_iter(iter);
    });

    int ret = mnt_table_parse_mtab(tab, nullptr);
    if (ret != 0)
        return "/";

    QStringList mpts;
    libmnt_fs *fs = nullptr;
    while (mnt_table_next_fs(tab, iter, &fs) == 0) {
        if (!fs)
            continue;

        const QString &target = mnt_fs_get_target(fs);
        mpts.append(target == "/" ? "/" : target + "/");
    }
    std::sort(mpts.begin(), mpts.end(), [](const QString &a, const QString &b) { return a.length() > b.length(); });

    QString path = filePath;
    if (!path.endsWith("/"))
        path.append("/");

    auto found = std::find_if(mpts.cbegin(), mpts.cend(), [path](const QString &mpt) { return path.startsWith(mpt); });
    return found != mpts.cend() ? *found : "/";
}
QString DeviceUtils::fileSystemType(const QUrl &url)
{
    return DFMIO::DFMUtils::fsTypeFromUrl(url);
}

qint64 DeviceUtils::deviceBytesFree(const QUrl &url)
{
    if (url.scheme() != Global::Scheme::kFile)
        return DFMIO::DFMUtils::deviceBytesFree(url);
    auto devicePath = bindPathTransform(url.path(), true);
    auto map = DevProxyMng->queryDeviceInfoByPath(devicePath, true);
    if (map.contains(kSizeFree) && map.value(kSizeFree, 0).toLongLong() > 0)
        return map.value(kSizeFree, 0).toLongLong();
    if (map.contains(kSizeTotal) && map.contains(kSizeUsed))
        return map.value(kSizeTotal, 0).toLongLong() - map.value(kSizeUsed, 0).toLongLong();

    return DFMIO::DFMUtils::deviceBytesFree(url);
}

bool DeviceUtils::isUnmountSamba(const QUrl &url)
{
    if (!isSamba(url))
        return false;

    return !DevProxyMng->isFileOfProtocolMounts(url.path());
}

QString DeviceUtils::bindPathTransform(const QString &path, bool toDevice)
{
    if (!path.startsWith("/") || path == "/")
        return path;

    const QMap<QString, QString> &table = DeviceUtils::fstabBindInfo();
    if (table.isEmpty())
        return path;

    QString bindPath(path);
    if (toDevice) {
        for (const auto &mntPoint : table.values()) {
            if (path.startsWith(mntPoint)) {
                bindPath.replace(mntPoint, table.key(mntPoint));
                break;
            }
        }
    } else {
        for (const auto &device : table.keys()) {
            if (path.startsWith(device)) {
                bindPath.replace(device, table[device]);
                break;
            }
        }
    }

    return bindPath;
}

bool DeviceUtils::isSystemDisk(const QVariantHash &devInfo)
{
    if (!devInfo.contains(GlobalServerDefines::DeviceProperty::kHintSystem))
        return false;

    bool isSystem = devInfo.value(GlobalServerDefines::DeviceProperty::kHintSystem).toBool()
            || devInfo.value(GlobalServerDefines::DeviceProperty::kConnectionBus).toString() != "usb";
    if (devInfo.value(GlobalServerDefines::DeviceProperty::kOpticalDrive).toBool())
        isSystem = false;
    // treat the siblings of root(/) device as System devices.
    isSystem |= isSiblingOfRoot(devInfo);
    return isSystem;
}

bool DeviceUtils::isSystemDisk(const QVariantMap &devInfo)
{
    QVariantHash hash;
    QMapIterator<QString, QVariant> iter(devInfo);
    while (iter.hasNext()) {
        iter.next();
        hash.insert(iter.key(), iter.value());
    }
    return isSystemDisk(hash);
}

bool DeviceUtils::isSiblingOfRoot(const QVariantHash &devInfo)
{
    static QString rootDrive;
    static std::once_flag flg;
    std::call_once(flg, [] {
        const QString &rootDev = DeviceUtils::getMountInfo("/", false);
        const QString &rootDevId = DeviceUtils::getBlockDeviceId(rootDev);
        const auto &data = DevProxyMng->queryBlockInfo(rootDevId);
        rootDrive = data.value(GlobalServerDefines::DeviceProperty::kDrive).toString();
        qCInfo(logDFMBase) << "got root drive:" << rootDrive << rootDev;
    });

    return rootDrive == devInfo.value(GlobalServerDefines::DeviceProperty::kDrive).toString();
}

bool DeviceUtils::isSiblingOfRoot(const QVariantMap &devInfo)
{
    QVariantHash hash;
    QMapIterator<QString, QVariant> iter(devInfo);
    while (iter.hasNext()) {
        iter.next();
        hash.insert(iter.key(), iter.value());
    }
    return isSiblingOfRoot(hash);
}

bool DeviceUtils::findDlnfsPath(const QString &target, Compare func)
{
    Q_ASSERT(func);
    libmnt_table *tab { mnt_new_table() };
    libmnt_iter *iter = mnt_new_iter(MNT_ITER_BACKWARD);

    FinallyUtil finally([=] {
        if (tab) mnt_free_table(tab);
        if (iter) mnt_free_iter(iter);
    });
    Q_UNUSED(finally);

    auto unifyPath = [](const QString &path) {
        return path.endsWith("/") ? path : path + "/";
    };

    int ret = mnt_table_parse_mtab(tab, nullptr);
    if (ret != 0) {
        qCWarning(logDFMBase) << "device: cannot parse mtab" << ret;
        return false;
    }

    libmnt_fs *fs = nullptr;
    while (mnt_table_next_fs(tab, iter, &fs) == 0) {
        if (!fs)
            continue;
        if (strcmp("dlnfs", mnt_fs_get_source(fs)) == 0) {
            QString mpt = unifyPath(mnt_fs_get_target(fs));
            if (func(unifyPath(target), mpt))
                return true;
        }
    }

    return false;
}

bool DeviceUtils::hasMatch(const QString &txt, const QString &rex)
{
    QRegularExpression re(rex);
    QRegularExpressionMatch match = re.match(txt);
    return match.hasMatch();
}
