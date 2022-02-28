/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "desktopfileactions.h"

#include "dfm-base/utils/properties.h"

#include <QSettings>
#include <QLocale>

DFMBASE_BEGIN_NAMESPACE

DesktopFileActions::DesktopFileActions(AbstractFileInfoPointer fInfo)
    : AbstractFileActions(fInfo)
{
    updateInfo(fInfo->url());
}

DesktopFileActions::~DesktopFileActions()
{
}

QVector<ActionType> DesktopFileActions::menuActionList(AbstractMenu::MenuType type) const
{
    if (deepinID == "dde-trash"
        || deepinID == "dde-home"
        || deepinID == "dde-computer"
        || (deepinID == "dde-file-manager" && exec.contains("-O"))) {
        QVector<ActionType> actions;
        actions << ActionType::kActOpen
                << ActionType::kActSeparator;

        if (deepinID == "dde-trash") {
            // TODO(lee): 暂未有清空回收站弹框功能，屏蔽掉，待功能齐全后开放
            actions /*<< ActionType::kActClearTrash*/
                    << ActionType::kActSeparator;
        }
        // TODO(lee) 创建链接后续功能还未完善，屏蔽，完善后开放
        //        if (type == AbtMenuType::kSingleFile)
        //            actions << ActionType::kActCreateSymlink;

        actions << ActionType::kActProperty;

        return actions;
    } else {
        return AbstractFileActions::getFileInfo()->menuActionList(type);
    }
}

QSet<ActionType> DesktopFileActions::disableMenuActionList() const
{
    // todo(Lee):
    //    if (d->deepinID == "dde-trash") {
    //        QSet<MenuAction> actions;
    //        if (FileUtils::filesCount(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath)) <= 0)
    //            actions << MenuAction::ClearTrash;
    //        return actions;
    //    }
    //        return DFileInfo::disableMenuActionList();
    return {};
}

QMap<QString, QVariant> DesktopFileActions::getDesktopFileInfo(const QUrl &fileUrl)
{
    // todo(Lee): 后续应该移植到其他专门解析desktop文件的类
    QMap<QString, QVariant> map;
    QSettings settings(fileUrl.path(), QSettings::IniFormat);

    settings.setIniCodec("utf-8");
    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(fileUrl.path(), "Desktop Entry");

    // 由于获取的系统语言简写与.desktop的语言简写存在不对应关系，经决定先采用获取的系统值匹配
    // 若没匹配到则采用系统值"_"左侧的字符串进行匹配，均为匹配到，才走原未匹配流程
    auto getValueFromSys = [&desktop, &settings](const QString &type, const QString &sysName) -> QString {
        const QString key = QString("%0[%1]").arg(type).arg(sysName);
        return desktop.value(key, settings.value(key)).toString();
    };

    auto getNameByType = [&desktop, &settings, &getValueFromSys](const QString &type) -> QString {
        QString tempSysName = QLocale::system().name();
        QString targetName = getValueFromSys(type, tempSysName);
        if (targetName.isEmpty()) {
            auto strSize = tempSysName.trimmed().split("_");
            if (!strSize.isEmpty()) {
                tempSysName = strSize.first();
                targetName = getValueFromSys(type, tempSysName);
            }

            if (targetName.isEmpty())
                targetName = desktop.value(type, settings.value(type)).toString();
        }

        return targetName;
    };

    map["Name"] = getNameByType("Name");
    map["GenericName"] = getNameByType("GenericName");

    map["Exec"] = desktop.value("Exec", settings.value("Exec"));
    map["Icon"] = desktop.value("Icon", settings.value("Icon"));
    map["Type"] = desktop.value("Type", settings.value("Type", "Application"));
    map["Categories"] = desktop.value("Categories", settings.value("Categories")).toString().remove(" ").split(";");
    map["MimeType"] = desktop.value("MimeType", settings.value("MimeType")).toString().remove(" ").split(";");
    map["DeepinID"] = desktop.value("X-Deepin-AppID", settings.value("X-Deepin-AppID")).toString();
    map["DeepinVendor"] = desktop.value("X-Deepin-Vendor", settings.value("X-Deepin-Vendor")).toString();

    return map;
}

void DesktopFileActions::updateInfo(const QUrl &fileUrl)
{
    const QMap<QString, QVariant> &map = DesktopFileActions::getDesktopFileInfo(fileUrl);

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
    if (categories.first().compare("") == 0) {
        categories.removeFirst();
    }
}

DFMBASE_END_NAMESPACE
