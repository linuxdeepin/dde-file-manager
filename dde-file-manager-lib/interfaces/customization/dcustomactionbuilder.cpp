#include "dcustomactionbuilder.h"
#include "dfilemenu.h"
#include "interfaces/dfileservices.h"

#include <QDir>
#include <QUrl>
#include <QMenu>

DCustomActionBuilder::DCustomActionBuilder(QObject *parent) : QObject(parent)
{

}

/*!
    根据 \a actionData中配置信息自动识别创建菜单和动作对象并递归完成子项的创建。
    \a parentForSubmenu 指定子菜单的父对象，必须为有效值，子菜单随 \a parentForSubmenu 释放而释放，
    若为无效值则子菜单将无法释放，造成内存泄漏。
    返回值 QAction* 对象的生命周期由调用方控制。
*/
QAction *DCustomActionBuilder::buildAciton(const DCustomActionData &actionData, QWidget *parentForSubmenu) const
{
    QAction *ret = nullptr;
    if (actionData.isAction()) {
        ret = createAciton(actionData);
    }
    else {
        ret = createMenu(actionData, parentForSubmenu);
    }

    return ret;
}

/*!
    设置菜单的文件夹 \a dir 。提取文件夹名称，用于菜单传参
 */
void DCustomActionBuilder::setActiveDir(const DUrl &dir)
{
    m_dirPath = dir;
    auto fileInfo = DFileService::instance()->createFileInfo(nullptr, dir);
    if (fileInfo)
        m_dirName = fileInfo->fileName();
}

/*!
    设置焦点文件 \a file ，即右键点击的文件。当为空白区域时，无需设置。
    提取文件\a file 的不带扩展名的文件名称和文件完整名称，用于菜单传参
 */
void DCustomActionBuilder::setFocusFile(const DUrl &file)
{
    m_filePath = file;
    auto fileInfo = DFileService::instance()->createFileInfo(nullptr, file);
    if (fileInfo) {
        m_fileFullName = fileInfo->fileName();
        m_fileBaseName = fileInfo->baseName();
    }
}

/*!
    检查 \a files 文件列表中的文件组合
 */
DCustomActionDefines::ComboTypes DCustomActionBuilder::checkFileCombo(const DUrlList &files)
{
    int fileCount = 0;
    int dirCount = 0;
    for (const DUrl &url : files) {
        if (url.isEmpty())
            continue;

        auto info = DFileService::instance()->createFileInfo(Q_NULLPTR, url);
        if (!info)
            continue;

        //目前只判断是否为文件夹
        info->isDir() ? ++dirCount : ++fileCount;

        //文件夹和文件同时存在
        if (dirCount > 0 && fileCount > 0)
            return DCustomActionDefines::FileAndDir;
    }

    //文件
    if (fileCount > 0)
        return fileCount > 1 ? DCustomActionDefines::MultiFiles : DCustomActionDefines::SingleFile;

    //文件夹
    if (dirCount > 0)
        return dirCount > 1 ? DCustomActionDefines::MultiDirs : DCustomActionDefines::SingleDir;

    return DCustomActionDefines::BlankSpace;
}

/*!
    筛选 \a rootActions 中支持 \a type 文件组合的菜单项
 */
QList<DCustomActionEntry> DCustomActionBuilder::matchFileCombo(const QList<DCustomActionEntry> &rootActions, DCustomActionDefines::ComboTypes type)
{
    QList<DCustomActionEntry> ret;
    for (auto it = rootActions.begin(); it != rootActions.end(); ++it) {
        if (it->fileCombo() & type)
            ret << *it;
    }

    return ret;
}

/*!
    创建菜单项，\a parentForSubmenu 用于指定菜单的父对象，用于自动释放
    通过获取 \a actionData 中的标题，图标等信息创建菜单项，并遍历创建子项和分割符号。
    返回值 QAction* 对象的生命周期由调用方控制。
*/
QAction *DCustomActionBuilder::createMenu(const DCustomActionData &actionData, QWidget *parentForSubmenu) const
{
    QAction *action = new QAction;
    QMenu *menu = new DFileMenu(parentForSubmenu);
    action->setMenu(menu);
    action->setProperty(DCustomActionDefines::kCustomActionFlag, true);

    //标题
    action->setText(makeName(actionData.name(), actionData.nameArg()));

    //图标
    const QString &iconName = actionData.icon();
    if (!iconName.isEmpty()) {
        const QIcon &&icon = getIcon(iconName);
        if (!icon.isNull())
            action->setIcon(icon);
    }

    //子项,子项的顺序由解析器保证
    QList<DCustomActionData> subActions = actionData.acitons();
    for (auto it = subActions.begin(); it != subActions.end(); ++it) {
        QAction *action = buildAciton(*it, parentForSubmenu);
        if (!action)
            continue;

        auto separator = it->separator();
        //上分割线
        if (separator & DCustomActionDefines::Top) {
            const QList<QAction*> &actionList = menu->actions();
            if (!actionList.isEmpty()) {
                auto lastAction = menu->actions().last();

                //不是分割线则插入
                if (!lastAction->isSeparator()) {
                    menu->addSeparator()->setParent(menu);
                }
            }
        }

        action->setParent(menu);
        menu->addAction(action);

        //下分割线
        if ((separator & DCustomActionDefines::Bottom) && ((it + 1) != subActions.end())) {
            menu->addSeparator()->setParent(menu);
        }
    }

    return action;
}

/*!
    使用 \a actionData 创建动作对象。
    将动作需要执行的命令写入属性 \key DCustomActionDefines::kCustomActionCommand 中。
    返回值 QAction* 对象的生命周期由调用方控制。
*/
QAction *DCustomActionBuilder::createAciton(const DCustomActionData &actionData) const
{
    QAction *action = new QAction;
    action->setProperty(DCustomActionDefines::kCustomActionFlag, true);

    //执行动作
    action->setProperty(DCustomActionDefines::kCustomActionCommand, actionData.command());

    //标题
    action->setText(makeName(actionData.name(), actionData.nameArg()));

    //图标
    const QString &iconName = actionData.icon();
    if (!iconName.isEmpty()) {
        const QIcon &&icon = getIcon(iconName);
        if (!icon.isNull())
            action->setIcon(icon);
    }

    return action;
}

/*!
    使用 \a iconName 获取图标，可以是主题类型的图标名，也可以是图标的绝对路径。
    若找不到图标，则返回空对象
*/
QIcon DCustomActionBuilder::getIcon(const QString &iconName) const
{
    QIcon ret;

    //通过路径获取图标
    QFileInfo fileInfo(iconName.startsWith("~") ? (QDir::homePath() + iconName.mid(1)) : iconName);

    if (!fileInfo.exists())
        fileInfo.setFile(QUrl::fromUserInput(iconName).toLocalFile());

    if (fileInfo.exists()) {
        ret = QIcon(fileInfo.absoluteFilePath());
    }

    //从主题获取
    if (ret.isNull()) {
        ret = QIcon::fromTheme(iconName);
    }

    return ret;
}

/*!
    使用当前文件或文件夹信息替换 \a name 中的 \a arg 参数。
 */
QString DCustomActionBuilder::makeName(const QString &name, DCustomActionDefines::ActionArg arg) const
{
    auto replace = [](QString input, const QString &before,const QString &after){
        QString ret = input;
        int index = input.indexOf(before);
        if (index >= 0) {
            ret = input.replace(index, before.size(), after);
        }
        return ret;
    };

    QString ret;
    switch (arg) {
    case DCustomActionDefines::DirName:
        ret = replace(name, DCustomActionDefines::kStrActionArg[arg],m_dirName);
        break;
    case DCustomActionDefines::BaseName:
        ret = replace(name, DCustomActionDefines::kStrActionArg[arg], m_fileBaseName);
        break;
    case DCustomActionDefines::FileName:
        ret = replace(name, DCustomActionDefines::kStrActionArg[arg], m_fileFullName);
        break;
    default:
        ret = name;
        break;
    }
    return ret;
}
