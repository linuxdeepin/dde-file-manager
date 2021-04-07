/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

#include "dcustomactionparser.h"

#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QFileSystemWatcher>

using namespace DCustomActionDefines;

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
            section = line.mid(1, line.length()-2);
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
    return m_customFormat;
}

RegisterCustomFormat::RegisterCustomFormat()
{
    //注册读写规则
    m_customFormat = QSettings::registerFormat("conf", &RegisterCustomFormat::readConf, &RegisterCustomFormat::writeConf);
}

DCustomActionParser::DCustomActionParser(QObject *parent)
    : QObject(parent)
{
    //获取注册的自定义方式
    m_customFormat = RegisterCustomFormat::instance().customFormat();
    m_fileWatcher = new QFileSystemWatcher;
    //监听目录
    m_fileWatcher->addPath(kCustomMenuPath);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &DCustomActionParser::delayRefresh);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &DCustomActionParser::delayRefresh);

    initHash();
    loadDir(kCustomMenuPath);
    //暂时不考虑效率，todo后续优化考虑开线程处理此loadDir
}

DCustomActionParser::~DCustomActionParser()
{
    if (m_fileWatcher) {
        m_fileWatcher->deleteLater();
        m_fileWatcher = nullptr;
    }
}

/*!
    根据给定的文件夹路径\a dirPath 遍历解析该文件夹下的.conf文件,
    返回值 bool* 为是否成功遍历文件夹。
*/
bool DCustomActionParser::loadDir(const QString &dirPath)
{
    if (dirPath.isEmpty())
        return false;
    m_topActionCount = 0;
    QDir dir(dirPath);
    if (!dir.exists())
        return false;
    m_actionEntry.clear();

    m_fileWatcher->removePaths(m_fileWatcher->files());

    //以时间先后遍历
    for (const QFileInfo &actionFileInfo : dir.entryInfoList({"*.conf"}, QDir::Files, QDir::Time)) {
        //监听每个conf文件的修改
        m_fileWatcher->addPath(actionFileInfo.absoluteFilePath());

        //解析文件字段
        QSettings actionSetting(actionFileInfo.filePath(), m_customFormat);
        actionSetting.setIniCodec("UTF-8");
        parseFile(actionSetting);
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
    foreach (const DCustomActionEntry &entry, m_actionEntry) {
        //NotShowIn
        if (isActionShouldShow(entry.m_notShowIn, onDesktop))
            ret << entry;//一级菜单不在桌面/文管显示则跳过该项
    }

    return ret;
}

/*!
    根据传入的\a actionSetting 解析菜单项，返回返回值为解析成功与否，关键字段缺失会被断定未无效文件，归于失败
*/
bool DCustomActionParser::parseFile(QSettings &actionSetting)
{
    //基本信息，版本，选中类型,且选中类型无明确说明则认为是无效的配置文件
    FileBasicInfos basicInfos;
    bool prefixExists = actionSetting.childGroups().contains(kMenuPrefix);
    if (!prefixExists) { //关键入口信息没有，认为是无效的配置文件
        return false;
    }
    if (!actionFileInfos(basicInfos, actionSetting))
        return false;//关键信息无效则

    auto actions = getValue(actionSetting, kMenuPrefix, kActionGroups).toString().simplified();
    if (actions.isEmpty())
        return false; //无一级菜单,无效文件

    auto actStr = getValue(actionSetting, kMenuPrefix, kActionGroups);
    auto actList = actStr.toString().simplified().split(":", QString::SkipEmptyParts);

    for(auto &once : actList) {
        if (m_topActionCount == kCustomMaxNumOne) //一级数量限制
            break;
        QList<DCustomActionData> childrenActions;//这个实际上一级时没用
        bool needSort;//一级用不到
        QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);
        m_hierarchyNum = 1;
        bool isVisible = parseFile(childrenActions, actionSetting, targetGroup, basicInfos, needSort, true);
        //bug-59348 解决解析失败 count++ 导致不能显示50个有效文件(一级菜单)
        if (isVisible) {
            m_topActionCount++;
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
    m_hierarchyNum++;
    if (4 < m_hierarchyNum) //超过三级不解
        return false;

    DCustomActionData actData;
    //暂时用localname 和name,方式有些不确定，oem和之前的自定义右键是localName，打开方式又好像是genaricName
    //后续确认优化
    //目前菜单项名的国际化暂支持"语言_地区/国家"或“语言”简写，即支持“zh_CN”或“zh”的方式。若未找到对应国际化信息，则采用兜底信息
    QString name;
    auto getNameByType = [this, &name, &actionSetting, group](const QString& type) {

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
  
    actData.m_name = name;
    actionNameDynamicArg(actData);

    //story#4481,产品变更暂不考虑icon
#if 0
    //icon
    actData.m_icon = getValue(actionSetting, group, kActionIcon).toString();
#endif
    //pos
    actData.m_position =  getValue(actionSetting, group, kActionPos).toInt();
    if (0 == actData.m_position && isSort) //未定义pos行为当前层级以上级指定顺序
        isSort = false;

    //separator
    QString separator = getValue(actionSetting, group, kActionSeparator).toString().simplified();
    actData.m_separator = m_separtor.value(separator, None);

    //actions 父子action级联与动作

    //actions 父级级联与动作
    QString actions = getValue(actionSetting, group, kActionGroups).toString().simplified();
    if (actions.isEmpty()) {
        //无级联检查是否有动作
        QString command = getValue(actionSetting, group, kActionCmd).toString().simplified();
        if (command.isEmpty())
            return false; //无动作无子级
        actData.m_command = command;
        execDynamicArg(actData);
    }
    else {
        //add 子菜单项，父级有子菜单，则忽略动作，即便子菜单无一有效，后续也不再添加动作
        QList<DCustomActionData> tpChildrenActions;
        auto actStr = getValue(actionSetting, group, kActionGroups);
        auto actList = actStr.toString().simplified().split(":", QString::SkipEmptyParts);

        int actCount = 0;
        bool needSort = true;
        for(auto &once : actList) {
            QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);
            //解决二三级存在的count问题
            bool isVisible = parseFile(tpChildrenActions, actionSetting, targetGroup, basicInfos, needSort, false);
            m_hierarchyNum--;
            if(isVisible) {
                actCount++;
            }
            if (2 == m_hierarchyNum && actCount == kCustomMaxNumTwo) //二级数量限制
                break;
            if (3 == m_hierarchyNum && actCount == kCustomMaxNumThree) //三级数量限制
                break;
        }
        if (0 == tpChildrenActions.size())
            return false; //作为无动作无子级，不再为其添加已有动作
//        actData.m_childrenActions = tpChildrenActions;
        if (needSort)//全量二三级才排序,否则按照写入actions顺序
            std::stable_sort(tpChildrenActions.begin()
                        , tpChildrenActions.end()
                        , [](const DCustomActionData& a
                        , const DCustomActionData& b){
                return a.m_position < b.m_position;
            });
        actData.m_childrenActions = tpChildrenActions;
    }

    if (isTop) {
        DCustomActionEntry tpEntry;

        //支持类型combo
        auto comboStr = getValue(actionSetting, group, kConfCombo).toString().simplified();
        if (comboStr.isEmpty()) {
            return false;//无支持选中类型默认该一级无效
        }
        else {
            QStringList comboList = comboStr.split(":", QString::SkipEmptyParts);
            ComboTypes target;
            for (auto temp : comboList) {
                auto tp = temp.simplified();
                if (m_combos.contains(tp))
                    target = target | m_combos.value(temp);
            }
            tpEntry.m_fileCombo = target;
        }

        //MimeType
        QString mimeTypeStr = getValue(actionSetting, group, kConfMimeType).toString().simplified();
        if (!mimeTypeStr.isEmpty())
            tpEntry.m_mimeTypes = mimeTypeStr.split(":");

        //X-DFM-ExcludeMimeTypes
        QString excludeMimeTypesStr = getValue(actionSetting, group, kConfExcludeMimeTypes).toString().simplified();
        if (!excludeMimeTypesStr.isEmpty())
            tpEntry.m_excludeMimeTypes= excludeMimeTypesStr.split(":");

        //X-DFM-SupportSchemes
        QString supportSchemesStr = getValue(actionSetting, group, kConfSupportSchemes).toString().simplified();
        if (!supportSchemesStr.isEmpty())
            tpEntry.m_supportSchemes = supportSchemesStr.split(":");

        //X-DFM-NotShowIn
        QString supportNotShowInStr = getValue(actionSetting, group, kConfNotShowIn).toString().simplified();
        if (!supportNotShowInStr.isEmpty())
            tpEntry.m_notShowIn = supportNotShowInStr.split(":");

        //X-DFM-SupportSuffix
        QString supportSuffixStr = getValue(actionSetting, group, kConfSupportSuffix).toString().simplified();
        if (!supportSuffixStr.isEmpty())
            tpEntry.m_supportSuffix = supportSuffixStr.split(":");

        //comboPos
        if (!comboPosForTopAction(actionSetting, group, actData))
            return false;//有一级菜单项支持的类型，但全无效，自动作为无效废弃项

        tpEntry.m_package = basicInfos.m_package;
        tpEntry.m_version = basicInfos.m_version;
        tpEntry.m_comment = basicInfos.m_comment;
        tpEntry.m_data = actData;
        m_actionEntry.append(tpEntry);
    }
    else {
        childrenActions.append(actData);
    }
    return true;
}

/*!
    初始化一些判断内容
*/
void DCustomActionParser::initHash()
{
    m_combos.insert("SingleFile", ComboType::SingleFile);
    m_combos.insert("SingleDir", ComboType::SingleDir);
    m_combos.insert("MultiFiles", ComboType::MultiFiles);
    m_combos.insert("MultiDirs", ComboType::MultiDirs);
    m_combos.insert("FileAndDir", ComboType::FileAndDir);
    m_combos.insert("BlankSpace", ComboType::BlankSpace);

    m_separtor.insert("None", Separator::None);
    m_separtor.insert("Top", Separator::Top);
    m_separtor.insert("Both", Separator::Both);
    m_separtor.insert("Bottom", Separator::Bottom);

    //name参数类型仅支持：DirName BaseName FileName
    m_actionNameArg.insert(kStrActionArg[DirName], ActionArg::DirName);       //%d
    m_actionNameArg.insert(kStrActionArg[BaseName], ActionArg::BaseName);     //%b
    m_actionNameArg.insert(kStrActionArg[FileName], ActionArg::FileName);     //"%a",

    //cmd参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    m_actionExecArg.insert(kStrActionArg[DirPath], ActionArg::DirPath);       //"%p"
    m_actionExecArg.insert(kStrActionArg[FilePath], ActionArg::FilePath);     //"%f"
    m_actionExecArg.insert(kStrActionArg[FilePaths], ActionArg::FilePaths);   //"%F"
    m_actionExecArg.insert(kStrActionArg[UrlPath], ActionArg::UrlPath);       //"%u"
    m_actionExecArg.insert(kStrActionArg[UrlPaths], ActionArg::UrlPaths);     //"%U"
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
    //基本信息
    //文件名
    basicInfo.m_package = actionSetting.fileName();

    //签名
    basicInfo.m_sign = getValue(actionSetting, kMenuPrefix, kConfSign).toString().simplified();

    //版本
    basicInfo.m_version = getValue(actionSetting, kMenuPrefix, kConfFileVersion).toString().simplified();
    if (basicInfo.m_version.isEmpty())
        return false;

    //描述
    basicInfo.m_comment = getValue(actionSetting, kMenuPrefix, kConfComment).toString().simplified();
    return true;
}

/*!
    菜单项名字参数动态获取
*/
void DCustomActionParser::actionNameDynamicArg(DCustomActionData &act)
{
    //name参数类型仅支持：DirName BaseName FileName
    int firstValidIndex = act.m_name.indexOf("%");
    auto cnt = act.m_name.length() - 1;
    if (0 == cnt || 0 > firstValidIndex) {
        act.m_nameArg = NoneArg;
        return;
    }

    while (cnt > firstValidIndex) {
        auto tgStr = act.m_name.mid(firstValidIndex, 2);
        auto tempValue = m_actionNameArg.value(tgStr, NoneArg);
        if (NoneArg != tempValue) {
            act.m_nameArg = tempValue;
            break;
        }
        firstValidIndex = act.m_name.indexOf("%", firstValidIndex + 1);
        if (-1 == firstValidIndex)
            break;
    }
}

/*!
    菜单项执行参数动态获取
*/
void DCustomActionParser::execDynamicArg(DCustomActionData &act)
{
    //cmd参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    int firstValidIndex = act.m_command.indexOf("%");
    auto cnt = act.m_command.length() - 1;
    if (0 == cnt || 0 > firstValidIndex) {
        act.m_cmdArg = NoneArg;
        return;
    }

    while (cnt > firstValidIndex) {
        auto tgStr = act.m_command.mid(firstValidIndex, 2);
        auto tempValue = m_actionExecArg.value(tgStr, NoneArg);
        if (NoneArg != tempValue) {
            act.m_cmdArg = tempValue;
            break;
        }
        firstValidIndex = act.m_name.indexOf("%", firstValidIndex + 1);
        if (-1 == firstValidIndex)
            break;
    }
}

/*!
    菜单项对应位置
*/
bool DCustomActionParser::comboPosForTopAction(QSettings &actionSetting, const QString &group, DCustomActionData &act)
{
    //能到这一步说明这个文件的有效性已经验证了
    auto comboStr = getValue(actionSetting, group, kConfCombo).toString().simplified();
    QStringList comboList = comboStr.split(":", QString::SkipEmptyParts);

    QString cPos;
    bool hasCombo = false;
    for (auto temp : comboList) {
        cPos = QString("%1-%2").arg(kActionPos, temp.simplified());
        auto ret = getValue(actionSetting, group, cPos);    //取出对应选中类型的pos
        if (m_combos.contains(temp)) {
            int pos = act.m_position;
            if (ret.isValid())
                pos = ret.toInt();
            act.m_comboPos.insert(m_combos.value(temp), pos);
            hasCombo = true;
        }
    }
    return hasCombo;
}

void DCustomActionParser::delayRefresh()
{
    if (m_refreshTimer) {
        m_refreshTimer->start(300);
        qDebug() << "restart refresh timer" << this;
        return;
    }

    qDebug() << "create refresh timer" << this;
    m_refreshTimer = new QTimer;
    connect(m_refreshTimer,&QTimer::timeout,this,[this](){
        m_actionEntry.clear();

        qInfo() << "loading custom menus" << this;
        loadDir(kCustomMenuPath);

        m_refreshTimer->stop();
        m_refreshTimer->deleteLater();
        m_refreshTimer = nullptr;
    });
    m_refreshTimer->start(300);
}

bool DCustomActionParser::isActionShouldShow(const QStringList &notShowInList, bool onDesktop)
{
    // X-DFM-NotShowIn not exist
    if (notShowInList.isEmpty())
        return true;    //未明确指明仅显示在桌面或者文管窗口默认都显示
    if (notShowInList.contains("*"))
        return false;   //都不显示： 配置了"X-DFM-NotShowIn=*"或者"X-DFM-NotShowIn=desktop:filemanager"

    // is menu triggered on desktop
    return (onDesktop && !notShowInList.contains("Desktop", Qt::CaseInsensitive)) ||
           (!onDesktop && !notShowInList.contains("Filemanager", Qt::CaseInsensitive));
}
