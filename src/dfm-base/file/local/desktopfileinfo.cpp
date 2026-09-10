// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopfileinfo.h"
#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDir>
#include <QSettings>
#include <QLocale>
#include <QApplication>
#include <QCoreApplication>
#include <QThread>
#include <QAtomicInteger>
#include <DGuiApplicationHelper>
#include <DPlatformTheme>

DGUI_USE_NAMESPACE
using namespace dfmbase;

namespace dfmbase {

// 主题版本号：主题类型/调色板/图标主题变化时自增，用于失效桌面项缓存的图标状态（#8）
static QAtomicInteger<int> s_iconThemeGeneration { 0 };
static void invalidateDesktopIconCache()
{
    s_iconThemeGeneration.storeRelease(s_iconThemeGeneration.loadAcquire() + 1);
}
static int iconThemeGeneration()
{
    // C++11 魔法静态：连接只建立一次，线程安全
    static const bool s_connected = []() {
        QObject::connect(DGuiApplicationHelper::instance(),
                         &DGuiApplicationHelper::themeTypeChanged,
                         &invalidateDesktopIconCache);
        QObject::connect(qApp, &QApplication::paletteChanged,
                         &invalidateDesktopIconCache);
        if (auto theme = DGuiApplicationHelper::instance()->systemTheme()) {
            QObject::connect(theme,
                             &DPlatformTheme::iconThemeNameChanged,
                             &invalidateDesktopIconCache);
        }
        return true;
    }();
    Q_UNUSED(s_connected)
    return s_iconThemeGeneration.loadAcquire();
}

class DesktopFileInfoPrivate : public QSharedData
{
public:
    inline explicit DesktopFileInfoPrivate(const QUrl &url)
        : QSharedData()
    {
        updateInfo(url);
    }

    inline DesktopFileInfoPrivate(const DesktopFileInfoPrivate &copy)
        : QSharedData(copy)
    {
    }

    inline ~DesktopFileInfoPrivate()
    {
    }

    void updateInfo(const QUrl &fileUrl)
    {
        const QMap<QString, QVariant> &map = DesktopFileInfo::desktopFileInfo(fileUrl);

        name = map.value("Name").toString();
        genericName = map.value("GenericName").toString();
        exec = map.value("Exec").toString();
        iconName = map.value("Icon").toString();
        type = map.value("Type").toString();
        categories = map.value("Categories").toStringList();
        mimeType = map.value("MimeType").toStringList();
        deepinID = map.value("DeepinID").toString();
        deepinVendor = map.value("DeepinVendor").toString();
        // Fix categories
        if (!categories.isEmpty() && categories.first().compare("") == 0) {
            categories.removeFirst();
        }

        if (iconName == "user-trash") {
            if (!FileUtils::trashIsEmpty())
                iconName = "user-trash-full";
        }

        icon = QIcon();
        iconThemeGen.storeRelease(-1);   // 失效图标缓存，使 ensureIconCacheFresh() 重算 genericIconName / useProxyIcon
    }

    // 主题变化时失效缓存的图标状态（#8）：icon / useProxyIcon / genericIconName
    void ensureIconCacheFresh()
    {
        Q_ASSERT(QThread::currentThread() == qApp->thread());
        const int gen = iconThemeGeneration();
        if (iconThemeGen.loadAcquire() == gen)
            return;
        iconThemeGen.storeRelease(gen);
        icon = QIcon();
        useProxyIcon.storeRelease(false);
        genericIconName = (!genericName.isEmpty() && QIcon::hasThemeIcon(genericName))
                          ? genericName
                          : QStringLiteral("application-default-icon");
    }

public:
    QString name;
    QString genericName;
    QString exec;
    QIcon icon;
    QString iconName;
    QString type;
    QStringList categories;
    QStringList mimeType;
    QString deepinID;
    QString deepinVendor;
    QString genericIconName { QStringLiteral("application-default-icon") };
    QAtomicInteger<bool> useProxyIcon { false };
    QAtomicInteger<int> iconThemeGen { -1 };   // 上次校验图标缓存时所用的主题版本号
};
}

DesktopFileInfo::DesktopFileInfo(const QUrl &fileUrl)
    : DesktopFileInfo(fileUrl, InfoFactory::create<FileInfo>(fileUrl))
{
}

DesktopFileInfo::DesktopFileInfo(const QUrl &fileUrl, const FileInfoPointer &info)
    : ProxyFileInfo(fileUrl), d(new DesktopFileInfoPrivate(fileUrl))
{
    setProxy(info);
}

DesktopFileInfo::~DesktopFileInfo()
{
}

QString DesktopFileInfo::desktopName() const
{
    if (d->deepinVendor == QStringLiteral("deepin") && !(d->genericName.isEmpty())) {
        return d->genericName;
    }

    return d->name;
}

QString DesktopFileInfo::desktopExec() const
{
    return d->exec;
}

QString DesktopFileInfo::desktopIconName() const
{
    d->ensureIconCacheFresh();   // 主题切换后失效陈旧的 iconName 缓存（#8）

    // special handling for trash desktop file which has tash datas
    if (d->iconName == "user-trash") {
        const auto trashState = FileUtils::trashEmptyState();
        if (trashState == FileUtils::TrashEmptyState::kEmpty)
            return d->iconName;

        // Fix: desktop startup must not synchronously touch trash:///.
        // Default to the non-empty icon until trashcore finishes probing.
        return "user-trash-full";
    }

    return d->iconName;
}

QString DesktopFileInfo::desktopType() const
{
    return d->type;
}

QStringList DesktopFileInfo::desktopCategories() const
{
    return d->categories;
}

QIcon DesktopFileInfo::fileIcon()
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    d->ensureIconCacheFresh();   // 主题切换后失效陈旧的 icon / useProxyIcon 缓存（#8）

    if (Q_LIKELY(!d->icon.isNull())) {
        return d->icon;
    }

    if (d->useProxyIcon)
        return proxy->fileIcon();

    const QString iconName = nameOf(NameInfoType::kIconName);

    if (iconName.startsWith("data:image/")) {
        int firstSemicolon = iconName.indexOf(';', 11);

        if (firstSemicolon > 11) {
            // iconPath is a string representing an inline image.

            int base64strPos = iconName.indexOf("base64,", firstSemicolon);

            if (base64strPos > 0) {
                QPixmap pixmap;

                bool ok = pixmap.loadFromData(QByteArray::fromBase64(iconName.mid(base64strPos + 7).toLatin1()) /*, format.toLatin1().constData()*/);

                if (ok) {
                    d->icon = QIcon(pixmap);
                } else {
                    d->icon = QIcon::fromTheme("application-default-icon");
                }
            }
        }
    } else {
        const QString &currentDir = QDir::currentPath();

        QDir::setCurrent(pathOf(PathInfoType::kAbsolutePath));

        QFileInfo fileInfo(iconName.startsWith("~") ? (QDir::homePath() + iconName.mid(1)) : iconName);

        if (!fileInfo.exists())
            fileInfo.setFile(QUrl::fromUserInput(iconName).toLocalFile());

        if (fileInfo.exists()) {
            d->icon = QIcon(fileInfo.absoluteFilePath());
        }

        QDir::setCurrent(currentDir);

        if (!d->icon.isNull() && QPixmap(fileInfo.absoluteFilePath()).isNull())
            d->icon = QIcon();
    }

    if (d->icon.isNull() && !iconName.isEmpty()) {
        d->icon = QIcon::fromTheme(iconName);

        if (d->icon.isNull()) {
            d->useProxyIcon.storeRelease(true);
            return ProxyFileInfo::fileIcon();
        }
    }

    // 图标无论如何都不能为空白：iconName 为空（如 0 字节 .desktop 文件）
    // 或所有解析路径均失败时，最终回退到 ProxyFileInfo（进而解析为 unknown 图标）
    if (d->icon.isNull()) {
        d->useProxyIcon.storeRelease(true);
        return ProxyFileInfo::fileIcon();
    }

    return d->icon;
}

QString DesktopFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileNameOfRename:
        [[fallthrough]];
    case NameInfoType::kBaseNameOfRename:
        return displayOf(DisPlayInfoType::kFileDisplayName);
    case NameInfoType::kSuffixOfRename:
        return QString();
    case NameInfoType::kFileCopyName:
        return ProxyFileInfo::nameOf(NameInfoType::kFileName);
    case NameInfoType::kIconName:
        return desktopIconName();
    case NameInfoType::kGenericIconName:
        d->ensureIconCacheFresh();   // 主题切换后失效陈旧的 genericIconName 缓存（#8）
        return d->genericIconName;
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QString DesktopFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (type == DisPlayInfoType::kFileDisplayName && !desktopName().isEmpty())
        return desktopName();

    return ProxyFileInfo::displayOf(type);
}

void DesktopFileInfo::refresh()
{
    ProxyFileInfo::refresh();
    d->updateInfo(urlOf(UrlInfoType::kUrl));
}

Qt::DropActions DesktopFileInfo::supportedOfAttributes(const SupportType type) const
{
    if (type == SupportType::kDrag && (d->deepinID == "dde-trash" || d->deepinID == "dde-computer")) {
        return Qt::IgnoreAction;
    }

    return ProxyFileInfo::supportedOfAttributes(type);
}

void DesktopFileInfo::updateAttributes(const QList<FileInfo::FileInfoAttributeID> &types)
{
    ProxyFileInfo::updateAttributes(types);
    d->updateInfo(urlOf(UrlInfoType::kUrl));
}

bool DesktopFileInfo::canTag() const
{
    if (d->deepinID == "dde-trash" || d->deepinID == "dde-computer")
        return false;

    // 桌面主目录不支持添加tag功能
    if (d->deepinID == "dde-file-manager" && d->exec.contains(" -O "))
        return false;

    return true;
}

bool DesktopFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanMoveOrCopy:
        // 部分桌面文件不允许复制或剪切
        if (d->deepinID == "dde-trash" || d->deepinID == "dde-computer")
            return false;

        // exec执行字符串中“-O”参数表示打开主目录
        if (d->deepinID == "dde-file-manager" && d->exec.contains(" -O "))
            return false;

        return true;
    case FileCanType::kCanDrop:
        if (d->deepinID == "dde-computer")
            return false;

        return ProxyFileInfo::canAttributes(type);
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

QMap<QString, QVariant> DesktopFileInfo::desktopFileInfo(const QUrl &fileUrl)
{
    QMap<QString, QVariant> map;
    DesktopFile desktopFile(fileUrl.path());

    map["Name"] = desktopFile.desktopLocalName();
    map["GenericName"] = desktopFile.desktopDisplayName();

    map["Exec"] = desktopFile.desktopExec();
    map["Icon"] = desktopFile.desktopIcon();
    map["Type"] = desktopFile.desktopType();
    map["Categories"] = desktopFile.desktopCategories();
    map["MimeType"] = desktopFile.desktopMimeType();
    map["DeepinID"] = desktopFile.desktopDeepinId();
    map["DeepinVendor"] = desktopFile.desktopDeepinVendor();

    return map;
}

QSharedPointer<FileInfo> DesktopFileInfo::convert(QSharedPointer<FileInfo> fileInfo)
{
    if (fileInfo.isNull())
        return fileInfo;

    const QUrl &url = fileInfo->urlOf(UrlInfoType::kUrl);
    if (!url.isValid() || url.scheme().isEmpty())
        return fileInfo;

    // invoking suffix/mimeTypeName might cost huge time
    if (ProtocolUtils::isRemoteFile(url))
        return fileInfo;

    if (FileUtils::isDesktopFileSuffix(fileInfo->fileUrl()))
        return FileInfoPointer(new DFMBASE_NAMESPACE::DesktopFileInfo(url, fileInfo));

    return fileInfo;
}
