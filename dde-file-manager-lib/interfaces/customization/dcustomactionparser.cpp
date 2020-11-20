#include "dcustomactionparser.h"

#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QTextCodec>


using namespace DCustomActionDefines;

DCustomActionParser::DCustomActionParser(QObject *parent) : QObject(parent)
{
    initHash();
}



/*!
    根据给定的文件夹路径\a dirPath 遍历解析该文件夹下的.conf文件,
    返回值 bool* 为是否成功遍历文件夹。
*/
bool DCustomActionParser::loadDir(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
        return false;
    m_actionEntry.clear();
    //以时间先后遍历
    for (const QFileInfo &actionFileInfo : dir.entryInfoList({"*.conf"}, QDir::Files, QDir::Time)) {

        //解析文件字段
        QSettings actionSetting(actionFileInfo.filePath(), QSettings::IniFormat);
        actionSetting.setIniCodec("UTF-8");
        parseFile(actionSetting);
    }
    return true;
}

/*!
    返回值QList<DCustomActionEntry>，返回加载解析的菜单项
*/
QList<DCustomActionEntry> DCustomActionParser::getActionFiles()
{
    return m_actionEntry;
}

/*!
    根据传入的\a actionSetting 解析菜单项，返回返回值为解析成功与否，关键字段缺失会被断定未无效文件，归于失败
*/
bool DCustomActionParser::parseFile(QSettings &actionSetting)
{
    //基本信息，版本，选中类型,且选中类型无明确说明则认为是无效的配置文件
    FileBasicInfos basicInfos;
    bool prefixExists = actionSetting.childGroups().contains(kUosPrefix);
    if (!prefixExists) { //关键入口信息没有，认为是无效的配置文件
        return false;
    }
    if (!actionFileInfos(basicInfos, actionSetting))
        return false;//关键信息无效则

    auto actions = getValue(actionSetting, kUosPrefix, kActionGroups).toString().trimmed();
    if (actions.isEmpty())
        return false; //无一级菜单,无效文件

    auto actStr = getValue(actionSetting, kUosPrefix, kActionGroups);
    auto actList = actStr.toString().trimmed().split(":", QString::SkipEmptyParts);
    hierarchyNum = 1;
    int actCount = 0;
    for(auto &once : actList) {
        QList<DCustomActionData> childrenActions;//这个实际上一级时没用
        QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);

        actCount++;
        parseFile(childrenActions, actionSetting, targetGroup, basicInfos, true);
        if (actCount == kCustomMaxNumOne) //一级数量限制
            break;
    }
    return true;
}

/*!
    该方法用于递归解析对应组下的菜单信息，\a childrenActions 获取菜单项，\a actionSetting 为解析对象，\a group 为待解析分组，
\a basicInfos 是为一级菜单项准备的基本信息，\a isTop 表示当前解析的是否是一级菜单
*/
void DCustomActionParser::parseFile(QList<DCustomActionData> &childrenActions, QSettings &actionSetting, const QString &group, const FileBasicInfos &basicInfos, bool isTop)
{
    hierarchyNum++;
    if (4 < hierarchyNum) //超过三级不解（待与产品沟通，是否需要）
        return;

    DCustomActionData actData;
    //暂时用localname 和name,方式有些不确定，oem和之前的自定义右键是localName，打开方式又好像是genaricName
    //后续确认优化
    QString localName = QString("Name[%1]").arg(QLocale::system().name());
    QString name = getValue(actionSetting, group, localName).toString().trimmed();
    if (name.isEmpty()) {
        name =  getValue(actionSetting, group, kActionName).toString();
        if (name.isEmpty())
            return; //无name无action
    }
    actData.m_name = name;
    qDebug() << name;

    //pos
    actData.m_position =  getValue(actionSetting, group, kActionPos).toInt();

    //separator
    QString separator = getValue(actionSetting, group, kActionPos).toString().trimmed();
    actData.m_separator = m_separtor.value(separator, None);

    //actions 父子action级联与动作

    //actions 父级级联与动作
    QString actions = getValue(actionSetting, group, kActionGroups).toString().trimmed();
    if (actions.isEmpty()) {
        //无级联检查是否有动作
        QString command = getValue(actionSetting, group, kActionCmd).toString().trimmed();
        if (command.isEmpty())
            return; //无动作无子级
        actData.m_command = command;
    }
    else {
        //add 子菜单项，此时父级应当无动作
        QList<DCustomActionData> tpChildrenActions;
        auto actStr = getValue(actionSetting, group, kActionGroups);
        auto actList = actStr.toString().trimmed().split(":", QString::SkipEmptyParts);

        int actCount = 0;
        for(auto &once : actList) {
            actCount++;
            QString targetGroup = QString("%1 %2").arg(kActionPrefix).arg(once);
            parseFile(tpChildrenActions, actionSetting, targetGroup, basicInfos);
            hierarchyNum--;
            if (2 == hierarchyNum && actCount == kCustomMaxNumTwo) //二级数量限制
                break;
            if (3 == hierarchyNum && actCount == kCustomMaxNumThree) //二级数量限制
                break;
        }
        if (0 == tpChildrenActions.size())
            return; //无动作无子级
        actData.m_childrenActions = tpChildrenActions;
    }

    if (isTop) {
        DCustomActionEntry tpEntry;
        tpEntry.m_package = basicInfos.m_package;
        tpEntry.m_version = basicInfos.m_version;
        tpEntry.m_comment = basicInfos.m_comment;
        tpEntry.m_fileCombo = basicInfos.m_fileCombo;
        tpEntry.m_data = actData;

        m_actionEntry.append(tpEntry);
    }
    else {
        childrenActions.append(actData);
    }
}

/*!
    初始化一些判断内容
*/
void DCustomActionParser::initHash()
{
    m_combos.insert("SingleFile", DCustomActionDefines::FileComboTypes::SingleFile);
    m_combos.insert("SingleDir", DCustomActionDefines::FileComboTypes::SingleDir);
    m_combos.insert("MultiFiles", DCustomActionDefines::FileComboTypes::MultiFiles);
    m_combos.insert("MultiDirs", DCustomActionDefines::FileComboTypes::MultiDirs);
    m_combos.insert("FileAndDir", DCustomActionDefines::FileComboTypes::FileAndDir);
    m_combos.insert("BlankSpace", DCustomActionDefines::FileComboTypes::BlankSpace);

    m_separtor.insert("None", DCustomActionDefines::Separator::None);
    m_separtor.insert("Top", DCustomActionDefines::Separator::Top);
    m_separtor.insert("Both", DCustomActionDefines::Separator::Both);
    m_separtor.insert("Bottom", DCustomActionDefines::Separator::Bottom);
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

    //版本
    basicInfo.m_version = getValue(actionSetting, kUosPrefix, kFileVersion).toString().trimmed();
    if (basicInfo.m_version.isEmpty())
        return false;

    //描述
    basicInfo.m_comment = getValue(actionSetting, kUosPrefix, kComment).toString().trimmed();

    //支持类型
    auto comboStr = getValue(actionSetting, kUosPrefix, kCombo).toString().trimmed();
    if (comboStr.isEmpty()) {
        return false;//无支持选中类型默认该文件无效
    }
    else {
        QStringList comboList = comboStr.split(":", QString::SkipEmptyParts);
        ComboTypes target;
        for (auto temp : comboList) {
            auto tp = temp.trimmed();
            if (m_combos.contains(tp))
                target = target | m_combos.value(temp);
        }
        basicInfo.m_fileCombo = target;
    }
    return true;

    //支持的文件类型(mimeTypes)，目前无需求暂不判断

}
