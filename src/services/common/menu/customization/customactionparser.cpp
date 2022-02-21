/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "customactionparser.h"

#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QFileSystemWatcher>

using namespace CustomActionDefines;

bool RegisterCustomFormat::readConf(QIODevice &device, QSettings::SettingsMap &settingsMap)
{
    QString section;
    QTextStream stream(&device);
    bool prefixExists = false;
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        // 跳过备注
        if (line.startsWith(QLatin1Char('#')))
            continue;
        // 分组
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            section = line.mid(1, line.length() - 2);
            if (section == kMenuPrefix)
                prefixExists = true;
            continue;
        }

        QString key = line.section(QLatin1Char('='), 0, 0).trimmed();
        QString value = line.section(QLatin1Char('='), 1).trimmed();

        if (key.isEmpty())
            continue;
        settingsMap[section + QLatin1Char('/') + key] = QVariant(value);
    }
    return prefixExists;
}

bool RegisterCustomFormat::writeConf(QIODevice &device, const QSettings::SettingsMap &settingsMap)
{
    Q_UNUSED(device)
    Q_UNUSED(settingsMap)
    return true;
}

RegisterCustomFormat &RegisterCustomFormat::instance()
{
    static RegisterCustomFormat instance;
    return instance;
}

QSettings::Format RegisterCustomFormat::customConfFormat()
{
    return customFormat;
}

RegisterCustomFormat::RegisterCustomFormat()
{
    //注册读写规则
    customFormat = QSettings::registerFormat("conf", &RegisterCustomFormat::readConf, &RegisterCustomFormat::writeConf);
}

CustomActionParser::CustomActionParser(QObject *parent)
    : QObject(parent)
{
    //获取注册的自定义方式
    customFormat = RegisterCustomFormat::instance().customConfFormat();
    fileWatcher = new QFileSystemWatcher;
    //监听目录
    fileWatcher->addPath(kCustomMenuPath);
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &CustomActionParser::delayRefresh);
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &CustomActionParser::delayRefresh);

    initHash();
    loadDir(kCustomMenuPath);
    //暂时不考虑效率，todo后续优化考虑开线程处理此loadDir
}

CustomActionParser::~CustomActionParser()
{
    if (fileWatcher) {
        fileWatcher->deleteLater();
        fileWatcher = nullptr;
    }
}

bool CustomActionParser::loadDir(const QString &dirPath)
{
    if (dirPath.isEmpty())
        return false;
    topActionCount = 0;
    QDir dir(dirPath);
    if (!dir.exists())
        return false;
    actionEntry.clear();

    fileWatcher->removePaths(fileWatcher->files());

    //以时间先后遍历
    for (const QFileInfo &actionFileInfo : dir.entryInfoList({ "*.conf" }, QDir::Files, QDir::Time)) {
        //监听每个conf文件的修改
        fileWatcher->addPath(actionFileInfo.absoluteFilePath());

        //解析文件字段
        QSettings actionSetting(actionFileInfo.filePath(), customFormat);
        actionSetting.setIniCodec("UTF-8");
        parseFile(actionSetting);
    }
    return true;
}

QList<CustomActionEntry> CustomActionParser::getActionFiles(bool onDesktop)
{
    QList<CustomActionEntry> ret;
    foreach (const CustomActionEntry &entry, actionEntry) {
        //NotShowIn
        if (isActionShouldShow(entry.actNotShowIn, onDesktop))
            ret << entry;   //一级菜单不在桌面/文管显示则跳过该项
    }

    return ret;
}

bool CustomActionParser::parseFile(QSettings &actionSetting)
{
    //基本信息，版本，选中类型,且选中类型无明确说明则认为是无效的配置文件
    FileBasicInfos basicInfos;
    bool prefixExists = actionSetting.childGroups().contains(kMenuPrefix);
    if (!prefixExists) {   //关键入口信息没有，认为是无效的配置文件
        return false;
    }
    if (!actionFileInfos(basicInfos, actionSetting))
        return false;   //关键信息无效则

    auto actions = getValue(actionSetting, kMenuPrefix, kActionGroups).toString().simplified();
    if (actions.isEmpty())
        return false;   //无一级菜单,无效文件

    auto actStr = getValue(actionSetting, kMenuPrefix, kActionGroups);
    auto actList = actStr.toString().simplified().split(":", QString::SkipEmptyParts);

    for (auto &once : actList) {
        if (topActionCount == kCustomMaxNumOne)   //一级数量限制
            break;
        QList<CustomActionData> childrenActions;   //这个实际上一级时没用
        bool needSort;   //一级用不到
        QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);
        hierarchyNum = 1;
        bool isVisible = parseFile(childrenActions, actionSetting, targetGroup, basicInfos, needSort, true);
        // 解决解析失败 count++ 导致不能显示50个有效文件(一级菜单)
        if (isVisible) {
            topActionCount++;
        }
    }
    return true;
}

bool CustomActionParser::parseFile(QList<CustomActionData> &childrenActions, QSettings &actionSetting, const QString &group, const FileBasicInfos &basicInfos, bool &isSort, bool isTop)
{
    hierarchyNum++;
    if (4 < hierarchyNum)   //超过三级不解
        return false;

    CustomActionData actData;
    //暂时用localname 和name,方式有些不确定，oem和之前的自定义右键是localName，打开方式又好像是genaricName
    //后续确认优化
    //目前菜单项名的国际化暂支持"语言_地区/国家"或“语言”简写，即支持“zh_CN”或“zh”的方式。若未找到对应国际化信息，则采用兜底信息
    QString name;
    auto getNameByType = [this, &name, &actionSetting, group](const QString &type) {
        QString systemName = QLocale::system().name().simplified();
        QString localName = QString("%1[%2]").arg(type).arg(systemName);
        name = getValue(actionSetting, group, localName).toString().simplified();
        QStringList localeList = systemName.simplified().split("_");

        if (name.isEmpty() && localeList.size() > 0) {
            localName = QString("%1[%2]").arg(type).arg(localeList.first());
            name = getValue(actionSetting, group, localName).toString().simplified();
        }

        if (name.isEmpty()) {
            name = getValue(actionSetting, group, type).toString().simplified();
        }
    };

    getNameByType(kActionName);
    if (name.isEmpty()) {
        qInfo() << "systemName: " << QLocale::system().name();
        return false;
    }

    actData.actName = name;
    actionNameDynamicArg(actData);

    // 产品变更暂不考虑icon
#if 0
    //icon
    actData.m_icon = getValue(actionSetting, group, kActionIcon).toString();
#endif
    //pos
    actData.actPosition = getValue(actionSetting, group, kActionPos).toInt();
    if (0 == actData.actPosition && isSort)   //未定义pos行为当前层级以上级指定顺序
        isSort = false;

    //separator
    QString separator = getValue(actionSetting, group, kActionSeparator).toString().simplified();
    actData.actSeparator = separtor.value(separator, kNone);

    //actions 父子action级联与动作

    //actions 父级级联与动作
    QString actions = getValue(actionSetting, group, kActionGroups).toString().simplified();
    if (actions.isEmpty()) {
        //无级联检查是否有动作
        QString command = getValue(actionSetting, group, kActionCmd).toString().simplified();
        if (command.isEmpty())
            return false;   //无动作无子级
        actData.actCommand = command;
        execDynamicArg(actData);
    } else {
        //add 子菜单项，父级有子菜单，则忽略动作，即便子菜单无一有效，后续也不再添加动作
        QList<CustomActionData> tpChildrenActions;
        auto actStr = getValue(actionSetting, group, kActionGroups);
        auto actList = actStr.toString().simplified().split(":", QString::SkipEmptyParts);

        int actCount = 0;
        bool needSort = true;
        for (auto &once : actList) {
            QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);
            //解决二三级存在的count问题
            bool isVisible = parseFile(tpChildrenActions, actionSetting, targetGroup, basicInfos, needSort, false);
            hierarchyNum--;
            if (isVisible) {
                actCount++;
            }
            if (2 == hierarchyNum && actCount == kCustomMaxNumTwo)   //二级数量限制
                break;
            if (3 == hierarchyNum && actCount == kCustomMaxNumThree)   //三级数量限制
                break;
        }
        if (0 == tpChildrenActions.size())
            return false;   //作为无动作无子级，不再为其添加已有动作
        //        actData.m_childrenActions = tpChildrenActions;
        if (needSort)   //全量二三级才排序,否则按照写入actions顺序
            std::stable_sort(tpChildrenActions.begin(), tpChildrenActions.end(), [](const CustomActionData &a, const CustomActionData &b) {
                return a.actPosition < b.actPosition;
            });
        actData.actChildrenActions = tpChildrenActions;
    }

    if (isTop) {
        CustomActionEntry tpEntry;

        //支持类型combo
        auto comboStr = getValue(actionSetting, group, kConfCombo).toString().simplified();
        if (comboStr.isEmpty()) {
            return false;   //无支持选中类型默认该一级无效
        } else {
            QStringList comboList = comboStr.split(":", QString::SkipEmptyParts);
            ComboTypes target;
            for (auto temp : comboList) {
                auto tp = temp.simplified();
                if (combos.contains(tp))
                    target = target | combos.value(temp);
            }
            tpEntry.actFileCombo = target;
        }

        //MimeType
        QString mimeTypeStr = getValue(actionSetting, group, kConfMimeType).toString().simplified();
        if (!mimeTypeStr.isEmpty())
            tpEntry.actMimeTypes = mimeTypeStr.split(":");

        //X-DFM-ExcludeMimeTypes
        QString excludeMimeTypesStr = getValue(actionSetting, group, kConfExcludeMimeTypes).toString().simplified();
        if (!excludeMimeTypesStr.isEmpty())
            tpEntry.actExcludeMimeTypes = excludeMimeTypesStr.split(":");

        //X-DFM-SupportSchemes
        QString supportSchemesStr = getValue(actionSetting, group, kConfSupportSchemes).toString().simplified();
        if (!supportSchemesStr.isEmpty())
            tpEntry.actSupportSchemes = supportSchemesStr.split(":");

        //X-DFM-NotShowIn
        QString supportNotShowInStr = getValue(actionSetting, group, kConfNotShowIn).toString().simplified();
        if (!supportNotShowInStr.isEmpty())
            tpEntry.actNotShowIn = supportNotShowInStr.split(":");

        //X-DFM-SupportSuffix
        QString supportSuffixStr = getValue(actionSetting, group, kConfSupportSuffix).toString().simplified();
        if (!supportSuffixStr.isEmpty())
            tpEntry.actSupportSuffix = supportSuffixStr.split(":");

        //comboPos
        if (!comboPosForTopAction(actionSetting, group, actData))
            return false;   //有一级菜单项支持的类型，但全无效，自动作为无效废弃项

        tpEntry.filePackage = basicInfos.package;
        tpEntry.fileVersion = basicInfos.version;
        tpEntry.fileComment = basicInfos.comment;
        tpEntry.actData = actData;
        actionEntry.append(tpEntry);
    } else {
        childrenActions.append(actData);
    }
    return true;
}

void CustomActionParser::initHash()
{
    combos.insert("SingleFile", ComboType::kSingleFile);
    combos.insert("SingleDir", ComboType::kSingleDir);
    combos.insert("MultiFiles", ComboType::kMultiFiles);
    combos.insert("MultiDirs", ComboType::kMultiDirs);
    combos.insert("FileAndDir", ComboType::kFileAndDir);
    combos.insert("BlankSpace", ComboType::kBlankSpace);

    separtor.insert("None", Separator::kNone);
    separtor.insert("Top", Separator::kTop);
    separtor.insert("Both", Separator::kBoth);
    separtor.insert("Bottom", Separator::kBottom);

    //name参数类型仅支持：DirName BaseName FileName
    actionNameArg.insert(kStrActionArg[kDirName], ActionArg::kDirName);   //%d
    actionNameArg.insert(kStrActionArg[kBaseName], ActionArg::kBaseName);   //%b
    actionNameArg.insert(kStrActionArg[kFileName], ActionArg::kFileName);   //"%a",

    //cmd参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    actionExecArg.insert(kStrActionArg[kDirPath], ActionArg::kDirPath);   //"%p"
    actionExecArg.insert(kStrActionArg[kFilePath], ActionArg::kFilePath);   //"%f"
    actionExecArg.insert(kStrActionArg[kFilePaths], ActionArg::kFilePaths);   //"%F"
    actionExecArg.insert(kStrActionArg[kUrlPath], ActionArg::kUrlPath);   //"%u"
    actionExecArg.insert(kStrActionArg[kUrlPaths], ActionArg::kUrlPaths);   //"%U"
}

QVariant CustomActionParser::getValue(QSettings &actionSetting, const QString &group, const QString &key)
{
    actionSetting.beginGroup(group);
    auto temp = actionSetting.value(key, QVariant());
    actionSetting.endGroup();
    return temp;
}

bool CustomActionParser::actionFileInfos(FileBasicInfos &basicInfo, QSettings &actionSetting)
{
    //基本信息
    //文件名
    basicInfo.package = actionSetting.fileName();

    //签名
    basicInfo.sign = getValue(actionSetting, kMenuPrefix, kConfSign).toString().simplified();

    //版本
    basicInfo.version = getValue(actionSetting, kMenuPrefix, kConfFileVersion).toString().simplified();
    if (basicInfo.version.isEmpty())
        return false;

    //描述
    basicInfo.comment = getValue(actionSetting, kMenuPrefix, kConfComment).toString().simplified();
    return true;
}

void CustomActionParser::actionNameDynamicArg(CustomActionData &act)
{
    //name参数类型仅支持：DirName BaseName FileName
    int firstValidIndex = act.actName.indexOf("%");
    auto cnt = act.actName.length() - 1;
    if (0 == cnt || 0 > firstValidIndex) {
        act.actNameArg = kNoneArg;
        return;
    }

    while (cnt > firstValidIndex) {
        auto tgStr = act.actName.mid(firstValidIndex, 2);
        auto tempValue = actionNameArg.value(tgStr, kNoneArg);
        if (kNoneArg != tempValue) {
            act.actNameArg = tempValue;
            break;
        }
        firstValidIndex = act.actName.indexOf("%", firstValidIndex + 1);
        if (-1 == firstValidIndex)
            break;
    }
}

void CustomActionParser::execDynamicArg(CustomActionData &act)
{
    //cmd参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    int firstValidIndex = act.actCommand.indexOf("%");
    auto cnt = act.actCommand.length() - 1;
    if (0 == cnt || 0 > firstValidIndex) {
        act.actCmdArg = kNoneArg;
        return;
    }

    while (cnt > firstValidIndex) {
        auto tgStr = act.actCommand.mid(firstValidIndex, 2);
        auto tempValue = actionExecArg.value(tgStr, kNoneArg);
        if (kNoneArg != tempValue) {
            act.actCmdArg = tempValue;
            break;
        }
        firstValidIndex = act.actName.indexOf("%", firstValidIndex + 1);
        if (-1 == firstValidIndex)
            break;
    }
}

bool CustomActionParser::comboPosForTopAction(QSettings &actionSetting, const QString &group, CustomActionData &act)
{
    //能到这一步说明这个文件的有效性已经验证了
    auto comboStr = getValue(actionSetting, group, kConfCombo).toString().simplified();
    QStringList comboList = comboStr.split(":", QString::SkipEmptyParts);

    QString cPos;
    bool hasCombo = false;
    for (auto temp : comboList) {
        cPos = QString("%1-%2").arg(kActionPos, temp.simplified());
        auto ret = getValue(actionSetting, group, cPos);   //取出对应选中类型的pos
        if (combos.contains(temp)) {
            int pos = act.actPosition;
            if (ret.isValid())
                pos = ret.toInt();
            act.actComboPos.insert(combos.value(temp), pos);
            hasCombo = true;
        }
    }
    return hasCombo;
}

void CustomActionParser::delayRefresh()
{
    if (refreshTimer) {
        refreshTimer->start(300);
        qDebug() << "restart refresh timer" << this;
        return;
    }

    qDebug() << "create refresh timer" << this;
    refreshTimer = new QTimer;
    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        actionEntry.clear();

        qInfo() << "loading custom menus" << this;
        loadDir(kCustomMenuPath);

        refreshTimer->stop();
        refreshTimer->deleteLater();
        refreshTimer = nullptr;
    });
    refreshTimer->start(300);
}

bool CustomActionParser::isActionShouldShow(const QStringList &notShowInList, bool onDesktop)
{
    // X-DFM-NotShowIn not exist
    if (notShowInList.isEmpty())
        return true;   //未明确指明仅显示在桌面或者文管窗口默认都显示
    if (notShowInList.contains("*"))
        return false;   //都不显示： 配置了"X-DFM-NotShowIn=*"或者"X-DFM-NotShowIn=desktop:filemanager"

    // is menu triggered on desktop
    return (onDesktop && !notShowInList.contains("Desktop", Qt::CaseInsensitive)) || (!onDesktop && !notShowInList.contains("Filemanager", Qt::CaseInsensitive));
}
