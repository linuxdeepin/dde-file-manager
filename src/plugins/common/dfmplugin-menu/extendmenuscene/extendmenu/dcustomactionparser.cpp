// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcustomactionparser.h"

#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <QThread>

using namespace dfmplugin_menu;
using namespace DCustomActionDefines;
DFMBASE_USE_NAMESPACE

/*!
 * \brief 自定义配置文件读规则
 * \param device 读取io
 * \param settingsMap 保存读取结果
 * \return
 */
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

/*!
 * \brief 自定义配置文件写规则
 * \param device
 * \param settingsMap
 * \return
 */
bool RegisterCustomFormat::writeConf(QIODevice &device, const QSettings::SettingsMap &settingsMap)
{
    Q_UNUSED(device)
    Q_UNUSED(settingsMap)
    return true;
}

/*!
 * \brief 注册自定义qsettings读取规则
 */
RegisterCustomFormat &RegisterCustomFormat::instance()
{
    static RegisterCustomFormat instance;
    return instance;
}

QSettings::Format RegisterCustomFormat::customFormat()
{
    return registeredcustomFormat;
}

RegisterCustomFormat::RegisterCustomFormat()
{
    // 注册读写规则
    registeredcustomFormat = QSettings::registerFormat("conf", &RegisterCustomFormat::readConf, &RegisterCustomFormat::writeConf);
}

DCustomActionParser::DCustomActionParser(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    // 获取注册的自定义方式
    customFormat = RegisterCustomFormat::instance().customFormat();

    initWatcher();
    initHash();
}

DCustomActionParser::~DCustomActionParser()
{
    std::for_each(watcherGroup.begin(), watcherGroup.end(), [](AbstractFileWatcherPointer pointer) {
        if (pointer)
            pointer->stopWatcher();
    });
}

/*!
    根据给定的文件夹路径\a dirPath 遍历解析该文件夹下的.conf文件,
    返回值 bool* 为是否成功遍历文件夹。
*/
bool DCustomActionParser::loadDir(const QStringList &dirPaths)
{
    if (dirPaths.isEmpty())
        return false;

    actionEntry.clear();
    QStringList cfgList;
    topActionCount = 0;
    for (auto dirPath : dirPaths) {

        QDir dir(dirPath);
        if (!dir.exists())
            continue;

        // 以时间先后遍历
        for (const QFileInfo &actionFileInfo : dir.entryInfoList({ "*.conf" }, QDir::Files, QDir::Time)) {
            const auto &cfgName = actionFileInfo.fileName();
            if (cfgList.contains(cfgName))
                continue;

            cfgList << cfgName;
            // 解析文件字段
            QSettings actionSetting(actionFileInfo.filePath(), customFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            actionSetting.setIniCodec("UTF-8");
#endif
            parseFile(actionSetting);
        }
    }
    return true;
}

/*!
    返回值QList<DCustomActionEntry>，返回加载解析的菜单项.
    \a onDesktop 匹配是否不再桌面/文管显示
*/
QList<DCustomActionEntry> DCustomActionParser::getActionFiles(bool onDesktop)
{
    QList<DCustomActionEntry> ret;
    foreach (const DCustomActionEntry &entry, actionEntry) {
        // NotShowIn
        if (isActionShouldShow(entry.actionNotShowIn, onDesktop))
            ret << entry;   // 一级菜单不在桌面/文管显示则跳过该项
    }

    return ret;
}

/*!
    根据传入的\a actionSetting 解析菜单项，返回返回值为解析成功与否，关键字段缺失会被断定未无效文件，归于失败
*/
bool DCustomActionParser::parseFile(QSettings &actionSetting)
{
    // 基本信息，版本，选中类型,且选中类型无明确说明则认为是无效的配置文件
    FileBasicInfos basicInfos;
    bool prefixExists = actionSetting.childGroups().contains(kMenuPrefix);
    if (!prefixExists) {   // 关键入口信息没有，认为是无效的配置文件
        return false;
    }
    if (!actionFileInfos(basicInfos, actionSetting))
        return false;   // 关键信息无效则

    auto actions = getValue(actionSetting, kMenuPrefix, kActionGroups).toString().simplified();
    if (actions.isEmpty())
        return false;   // 无一级菜单,无效文件

    auto actStr = getValue(actionSetting, kMenuPrefix, kActionGroups);
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    auto actList = actStr.toString().simplified().split(":", QString::SkipEmptyParts);
#else
    auto actList = actStr.toString().simplified().split(":", Qt::SkipEmptyParts);
#endif

    for (auto &once : actList) {
        if (topActionCount == kCustomMaxNumOne)   // 一级数量限制
            break;
        QList<DCustomActionData> childrenActions;   // 这个实际上一级时没用
        bool needSort;   // 一级用不到
        QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);
        hierarchyNum = 1;
        bool isVisible = parseFile(childrenActions, actionSetting, targetGroup, basicInfos, needSort, true);
        // bug-59348 解决解析失败 count++ 导致不能显示50个有效文件(一级菜单)
        if (isVisible) {
            topActionCount++;
        }
    }
    return true;
}

/*!
    该方法用于递归解析对应组下的菜单信息，\a childrenActions 获取菜单项，\a actionSetting 为解析对象，\a group 为待解析分组，
\a basicInfos 是为一级菜单项准备的基本信息，\a isTop 表示当前解析的是否是一级菜单
*/
bool DCustomActionParser::parseFile(QList<DCustomActionData> &childrenActions, QSettings &actionSetting, const QString &group, const FileBasicInfos &basicInfos, bool &isSort, bool isTop)
{
    hierarchyNum++;
    if (4 < hierarchyNum)   // 超过三级不解
        return false;

    DCustomActionData actData;
    // 暂时用localname 和name,方式有些不确定，oem和之前的自定义右键是localName，打开方式又好像是genaricName
    // 后续确认优化
    // 目前菜单项名的国际化暂支持"语言_地区/国家"或“语言”简写，即支持“zh_CN”或“zh”的方式。若未找到对应国际化信息，则采用兜底信息
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
        fmInfo() << "systemName: " << QLocale::system().name();
        return false;
    }

    actData.actionName = name;
    actionNameDynamicArg(actData);

    // story#4481,产品变更暂不考虑icon
#if 0
    //icon
    actData.m_icon = getValue(actionSetting, group, kActionIcon).toString();
#endif
    // pos
    actData.actionPosition = getValue(actionSetting, group, kActionPos).toInt();
    if (0 == actData.actionPosition)
        actData.actionPosition = getValue(actionSetting, group, kActionPosAlias).toInt();

    if (0 == actData.actionPosition && isSort)   // 未定义pos行为当前层级以上级指定顺序
        isSort = false;

    // separator
    QString separator = getValue(actionSetting, group, kActionSeparator).toString().simplified();
    if (separator.isEmpty())
        separator = getValue(actionSetting, group, kActionSeparatorAlias).toString().simplified();
    actData.actionSeparator = separtor.value(separator, kNone);

    QString parentMenuPath = getValue(actionSetting, group, kConfParentMenuPath).toString();
    actData.actionParentPath = parentMenuPath;

    // actions 父子action级联与动作

    // actions 父级级联与动作
    QString actions = getValue(actionSetting, group, kActionGroups).toString().simplified();
    if (actions.isEmpty()) {
        // 无级联检查是否有动作
        QString command = getValue(actionSetting, group, kActionCmd).toString().simplified();
        if (command.isEmpty())
            return false;   // 无动作无子级
        actData.actionCommand = command;
        execDynamicArg(actData);
    } else {
        // add 子菜单项，父级有子菜单，则忽略动作，即便子菜单无一有效，后续也不再添加动作
        QList<DCustomActionData> tpChildrenActions;
        auto actStr = getValue(actionSetting, group, kActionGroups);
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
        auto actList = actStr.toString().simplified().split(":", QString::SkipEmptyParts);
#else
        auto actList = actStr.toString().simplified().split(":", Qt::SkipEmptyParts);
#endif
        int actCount = 0;
        bool needSort = true;
        for (auto &once : actList) {
            QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);
            // 解决二三级存在的count问题
            bool isVisible = parseFile(tpChildrenActions, actionSetting, targetGroup, basicInfos, needSort, false);
            hierarchyNum--;
            if (isVisible) {
                actCount++;
            }
            if (2 == hierarchyNum && actCount == kCustomMaxNumTwo)   // 二级数量限制
                break;
            if (3 == hierarchyNum && actCount == kCustomMaxNumThree)   // 三级数量限制
                break;
        }
        if (0 == tpChildrenActions.size())
            return false;   // 作为无动作无子级，不再为其添加已有动作
        //        actData.m_childrenActions = tpChildrenActions;
        if (needSort)   // 全量二三级才排序,否则按照写入actions顺序
            std::stable_sort(tpChildrenActions.begin(), tpChildrenActions.end(), [](const DCustomActionData &a, const DCustomActionData &b) {
                return a.actionPosition < b.actionPosition;
            });
        actData.childrenActions = tpChildrenActions;
    }

    if (isTop) {
        DCustomActionEntry tpEntry;

        // 支持类型combo
        auto comboStr = getValue(actionSetting, group, kConfCombo).toString().simplified();
        if (comboStr.isEmpty())
            comboStr = getValue(actionSetting, group, kConfComboAlias).toString().simplified();

        if (comboStr.isEmpty()) {
            return false;   // 无支持选中类型默认该一级无效
        } else {
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
            QStringList &&comboList = comboStr.split(":", QString::SkipEmptyParts);
#else
            QStringList &&comboList = comboStr.split(":", Qt::SkipEmptyParts);
#endif
            ComboTypes target;
            for (auto temp : comboList) {
                auto tp = temp.simplified();
                if (combos.contains(tp))
                    target = target | combos.value(temp);
            }
            tpEntry.actionFileCombo = target;
        }

        // MimeType
        QString mimeTypeStr = getValue(actionSetting, group, kConfMimeType).toString().simplified();
        if (!mimeTypeStr.isEmpty())
            tpEntry.actionMimeTypes = mimeTypeStr.split(":");

        // X-DFM-ExcludeMimeTypes
        QString excludeMimeTypesStr = getValue(actionSetting, group, kConfExcludeMimeTypes).toString().simplified();
        if (excludeMimeTypesStr.isEmpty())
            excludeMimeTypesStr = getValue(actionSetting, group, kConfExcludeMimeTypesAlias).toString().simplified();
        if (!excludeMimeTypesStr.isEmpty())
            tpEntry.actionExcludeMimeTypes = excludeMimeTypesStr.split(":");

        // X-DFM-SupportSchemes
        QString supportSchemesStr = getValue(actionSetting, group, kConfSupportSchemes).toString().simplified();
        if (supportSchemesStr.isEmpty())
            supportSchemesStr = getValue(actionSetting, group, kConfSupportSchemesAlias).toString().simplified();
        if (!supportSchemesStr.isEmpty())
            tpEntry.actionSupportSchemes = supportSchemesStr.split(":");

        // X-DFM-NotShowIn
        QString supportNotShowInStr = getValue(actionSetting, group, kConfNotShowIn).toString().simplified();
        if (supportNotShowInStr.isEmpty())
            supportNotShowInStr = getValue(actionSetting, group, kConfNotShowInAlias).toString().simplified();
        if (!supportNotShowInStr.isEmpty())
            tpEntry.actionNotShowIn = supportNotShowInStr.split(":");

        // X-DFM-SupportSuffix
        QString supportSuffixStr = getValue(actionSetting, group, kConfSupportSuffix).toString().simplified();
        if (supportSuffixStr.isEmpty())
            supportSuffixStr = getValue(actionSetting, group, kConfSupportSuffixAlias).toString().simplified();
        if (!supportSuffixStr.isEmpty())
            tpEntry.actionSupportSuffix = supportSuffixStr.split(":");

        // comboPos
        if (!comboPosForTopAction(actionSetting, group, actData))
            return false;   // 有一级菜单项支持的类型，但全无效，自动作为无效废弃项

        tpEntry.packageName = basicInfos.package;
        tpEntry.packageVersion = basicInfos.version;
        tpEntry.packageComment = basicInfos.comment;
        tpEntry.actionData = actData;
        actionEntry.append(tpEntry);
    } else {
        childrenActions.append(actData);
    }
    return true;
}

void DCustomActionParser::initWatcher()
{
    static const QStringList &kPaths { { "/usr/etc/deepin/context-menus" },
                                       { "/etc/deepin/context-menus" },
                                       { QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                                         + QStringLiteral("/deepin/dde-file-manager/context-menus") } };

    // Add directories from XDG_DATA_DIRS environment variable
    const QByteArray xdgDataDirs = qgetenv("XDG_DATA_DIRS");
    if (!xdgDataDirs.isEmpty()) {
        const QStringList dataDirs = QString::fromLocal8Bit(xdgDataDirs).split(':');
        for (const QString &dir : dataDirs) {
            QString path = dir + "/applications/context-menus";
            if (!kPaths.contains(path) && QDir(path).exists())
                menuPaths.append(path);
        }
    }

    std::for_each(kPaths.begin(), kPaths.end(), [this](const QString &path) {
        if (QDir(path).exists())
            menuPaths.append(path);
    });

    std::for_each(menuPaths.begin(), menuPaths.end(), [this](const QString &path) {
        AbstractFileWatcherPointer watcher { WatcherFactory::create<AbstractFileWatcher>(QUrl::fromLocalFile(path)) };
        watcherGroup.append(watcher);
        if (watcher) {
            connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                    this, &DCustomActionParser::delayRefresh);
            watcher->startWatcher();
        }
    });
}

/*!
    初始化一些判断内容
*/
void DCustomActionParser::initHash()
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

    // name参数类型仅支持：DirName BaseName FileName
    actionNameArg.insert(kStrActionArg[kDirName], ActionArg::kDirName);   //%d
    actionNameArg.insert(kStrActionArg[kBaseName], ActionArg::kBaseName);   //%b
    actionNameArg.insert(kStrActionArg[kFileName], ActionArg::kFileName);   //"%a",

    // cmd参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    actionExecArg.insert(kStrActionArg[kDirPath], ActionArg::kDirPath);   //"%p"
    actionExecArg.insert(kStrActionArg[kFilePath], ActionArg::kFilePath);   //"%f"
    actionExecArg.insert(kStrActionArg[kFilePaths], ActionArg::kFilePaths);   //"%F"
    actionExecArg.insert(kStrActionArg[kUrlPath], ActionArg::kUrlPath);   //"%u"
    actionExecArg.insert(kStrActionArg[kUrlPaths], ActionArg::kUrlPaths);   //"%U"
}

/*!
    获取配置文件对应组下的对应字段信息，\a actionSetting 是解析对象，\a group是待解析的组， \a key是待解析字段
*/
QVariant DCustomActionParser::getValue(QSettings &actionSetting, const QString &group, const QString &key)
{
    actionSetting.beginGroup(group);
    auto temp = actionSetting.value(key, QVariant());
    actionSetting.endGroup();
    return temp;
}

/*!
    获取配置文件的一些基本信息，\a actionSetting获取解析的基本信息，\a actionSetting 是解析对象，
    返回值表示该文件有效与否，基本的关键信息没有会被判定成无效的配置文件
*/
bool DCustomActionParser::actionFileInfos(FileBasicInfos &basicInfo, QSettings &actionSetting)
{
    // 基本信息
    // 文件名
    basicInfo.package = actionSetting.fileName();

    // 签名
    basicInfo.sign = getValue(actionSetting, kMenuPrefix, kConfSign).toString().simplified();

    // 版本
    basicInfo.version = getValue(actionSetting, kMenuPrefix, kConfFileVersion).toString().simplified();
    if (basicInfo.version.isEmpty())
        return false;

    // 描述
    basicInfo.comment = getValue(actionSetting, kMenuPrefix, kConfComment).toString().simplified();
    return true;
}

/*!
    菜单项名字参数动态获取
*/
void DCustomActionParser::actionNameDynamicArg(DCustomActionData &act)
{
    // name参数类型仅支持：DirName BaseName FileName
    int firstValidIndex = act.actionName.indexOf("%");
    auto cnt = act.actionName.length() - 1;
    if (0 == cnt || 0 > firstValidIndex) {
        act.actionNameArg = kNoneArg;
        return;
    }

    while (cnt > firstValidIndex) {
        auto tgStr = act.actionName.mid(firstValidIndex, 2);
        auto tempValue = actionNameArg.value(tgStr, kNoneArg);
        if (kNoneArg != tempValue) {
            act.actionNameArg = tempValue;
            break;
        }
        firstValidIndex = act.actionName.indexOf("%", firstValidIndex + 1);
        if (-1 == firstValidIndex)
            break;
    }
}

/*!
    菜单项执行参数动态获取
*/
void DCustomActionParser::execDynamicArg(DCustomActionData &act)
{
    // cmd参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    int firstValidIndex = act.actionCommand.indexOf("%");
    auto cnt = act.actionCommand.length() - 1;
    if (0 == cnt || 0 > firstValidIndex) {
        act.actionCmdArg = kNoneArg;
        return;
    }

    while (cnt > firstValidIndex) {
        auto tgStr = act.actionCommand.mid(firstValidIndex, 2);
        auto tempValue = actionExecArg.value(tgStr, kNoneArg);
        if (kNoneArg != tempValue) {
            act.actionCmdArg = tempValue;
            break;
        }
        firstValidIndex = act.actionCommand.indexOf("%", firstValidIndex + 1);
        if (-1 == firstValidIndex)
            break;
    }
}

/*!
    菜单项对应位置
*/
bool DCustomActionParser::comboPosForTopAction(QSettings &actionSetting, const QString &group, DCustomActionData &act)
{
    // 能到这一步说明这个文件的有效性已经验证了
    auto comboStr = getValue(actionSetting, group, kConfCombo).toString().simplified();
    if (comboStr.isEmpty())
        comboStr = getValue(actionSetting, group, kConfComboAlias).toString().simplified();
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
    QStringList &&comboList = comboStr.split(":", QString::SkipEmptyParts);
#else
    QStringList &&comboList = comboStr.split(":", Qt::SkipEmptyParts);
#endif

    QString cPos;
    bool hasCombo = false;
    for (auto temp : comboList) {
        cPos = QString("%1-%2").arg(kActionPos, temp.simplified());
        auto ret = getValue(actionSetting, group, cPos);   // 取出对应选中类型的pos
        if (!ret.isValid()) {
            cPos = QString("%1-%2").arg(kActionPosAlias, temp.simplified());
            ret = getValue(actionSetting, group, cPos);
        }
        if (combos.contains(temp)) {
            int pos = act.actionPosition;
            if (ret.isValid())
                pos = ret.toInt();
            act.comboPos.insert(combos.value(temp), pos);
            hasCombo = true;
        }
    }
    return hasCombo;
}

void DCustomActionParser::delayRefresh()
{
    if (refreshTimer) {
        refreshTimer->start(300);
        fmDebug() << "restart refresh timer" << this;
        return;
    }

    fmDebug() << "create refresh timer" << this;
    refreshTimer = new QTimer;
    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        refreshTimer->stop();
        refreshTimer->deleteLater();
        refreshTimer = nullptr;

        fmInfo() << "loading custom menus" << this;
        refresh();
    });
    refreshTimer->start(300);
}

bool DCustomActionParser::isActionShouldShow(const QStringList &notShowInList, bool onDesktop)
{
    // X-DFM-NotShowIn not exist
    if (notShowInList.isEmpty())
        return true;   // 未明确指明仅显示在桌面或者文管窗口默认都显示
    if (notShowInList.contains("*"))
        return false;   // 都不显示： 配置了"X-DFM-NotShowIn=*"或者"X-DFM-NotShowIn=desktop:filemanager"

    // is menu triggered on desktop
    return (onDesktop && !notShowInList.contains("Desktop", Qt::CaseInsensitive)) || (!onDesktop && !notShowInList.contains("Filemanager", Qt::CaseInsensitive));
}
