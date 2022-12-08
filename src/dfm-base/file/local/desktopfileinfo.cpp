/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "desktopfileinfo.h"
#include "dfm-base/utils/desktopfile.h"
#include "dfm-base/utils/properties.h"
#include "dfm-base/utils/fileutils.h"

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
    : LocalFileInfo(fileUrl), d(new DesktopFileInfoPrivate(fileUrl))
{
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

    const QString &iconName = this->nameInfo(NameInfo::kIconName);

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

        QDir::setCurrent(pathInfo(PathInfo::kAbsolutePath));

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

    // 临时代码
    d->icon = QIcon::fromTheme(iconName);   // todo(lxs) LocalFileInfo::fileIcon() 统一处理

    if (d->icon.isNull())
        return LocalFileInfo::fileIcon();

    return d->icon;
}

QString DesktopFileInfo::nameInfo(const NameInfo type) const
{
    switch (type) {
    case NameInfo::kFileNameOfRename:
        [[fallthrough]];
    case NameInfo::kBaseNameOfRename:
        return displayInfo(DisPlay::kFileDisplayName);
    case NameInfo::kSuffixOfRename:
        return QString();
    case NameInfo::kFileCopyName:
        return LocalFileInfo::nameInfo(NameInfo::kFileName);
    case NameInfo::kIconName:
        return desktopIconName();
    case NameInfo::kGenericIconName:
        return QStringLiteral("application-default-icon");
    default:
        return LocalFileInfo::nameInfo(type);
    }
}

QString DesktopFileInfo::displayInfo(const DisPlay type) const
{
    if (type == DisPlay::kFileDisplayName && !desktopName().isEmpty())
        return desktopName();

    return LocalFileInfo::displayInfo(type);
}

void DesktopFileInfo::refresh()
{
    LocalFileInfo::refresh();
    d->updateInfo(urlInfo(UrlInfo::kUrl));
}

Qt::DropActions DesktopFileInfo::supportedAttributes(const SupportType type) const
{
    if (type == SupportType::kDrag && (d->deepinID == "dde-trash" || d->deepinID == "dde-computer")) {
        return Qt::IgnoreAction;
    }

    return LocalFileInfo::supportedAttributes(type);
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

bool DesktopFileInfo::canAttributes(const CanInfo type) const
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

        return LocalFileInfo::canAttributes(type);
    default:
        return LocalFileInfo::canAttributes(type);
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
