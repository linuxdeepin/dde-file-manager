// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopfileinfo.h"
#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>

#include <QDir>
#include <QSettings>
#include <QLocale>

using namespace dfmbase;

namespace dfmbase {
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

        icon = QIcon();
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
    //special handling for trash desktop file which has tash datas
    if (d->iconName == "user-trash") {
        if (!FileUtils::trashIsEmpty())
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
    if (Q_LIKELY(!d->icon.isNull())) {
        if (Q_LIKELY(!d->icon.availableSizes().isEmpty()))
            return d->icon;

        d->icon = QIcon();
    }

    const QString &iconName = this->nameOf(NameInfoType::kIconName);

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

    if (d->icon.isNull()) {
        d->icon = QIcon::fromTheme(iconName);

        if (d->icon.isNull())
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
        return QStringLiteral("application-default-icon");
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

    //桌面主目录不支持添加tag功能
    if (d->deepinID == "dde-file-manager" && d->exec.contains(" -O "))
        return false;

    return true;
}

bool DesktopFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanMoveOrCopy:
        //部分桌面文件不允许复制或剪切
        if (d->deepinID == "dde-trash" || d->deepinID == "dde-computer")
            return false;

        //exec执行字符串中“-O”参数表示打开主目录
        if (d->deepinID == "dde-file-manager" && d->exec.contains(" -O "))
            return false;

        return true;
    case FileCanType::kCanDrop:
        if (d->deepinID == "dde-computer")
            return false;

        return ProxyFileInfo::canAttributes(type);
    case FileCanType::kCanRename:
        if (!isAttributes(OptInfoType::kIsWritable))
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
