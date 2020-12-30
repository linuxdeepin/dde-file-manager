#include "dcustomactionbuilder.h"
#include "dfilemenu.h"
#include "interfaces/dfileservices.h"

#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QFontMetrics>

DCustomActionBuilder::DCustomActionBuilder(QObject *parent)
    : QObject(parent)
    , m_fm(QFontMetrics(QAction().font()))
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
    } else {
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
    auto info = DFileService::instance()->createFileInfo(nullptr, dir);
    if (info) {
        m_dirName = info->fileName();

        //解决根目录没有名称问题
        if (m_dirName.isEmpty() && dir.toLocalFile() == "/") {
            m_dirName = "/";
        }
    }
}

/*!
    设置焦点文件 \a file ，即右键点击的文件。当为空白区域时，无需设置。
    提取文件\a file 的不带扩展名的文件名称和文件完整名称，用于菜单传参
 */
void DCustomActionBuilder::setFocusFile(const DUrl &file)
{
    m_filePath = file;
    auto info = DFileService::instance()->createFileInfo(nullptr, file);
    if (info) {
        m_fileFullName = info->fileName();
        m_fileBaseName = info->baseName();

        //解决 .xx 一类的隐藏文件
        if (m_fileBaseName.isEmpty())
            m_fileBaseName = m_fileFullName;
    }
}

/*!
    检查 \a files 文件列表中的文件组合
 */
DCustomActionDefines::ComboType DCustomActionBuilder::checkFileCombo(const DUrlList &files)
{
    int fileCount = 0;
    int dirCount = 0;
    for (const DUrl &file : files) {
        if (file.isEmpty())
            continue;

        auto info = DFileService::instance()->createFileInfo(Q_NULLPTR, file);
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
    展开 \a cmd 中需要的参数。只处理找到的一个有效的 \a arg 参数，后面的不再替换。
    参数类型只支持：DirPath FilePath FilePaths UrlPath UrlPaths
    若参数 \a arg 为FilePaths和UrlPaths则只支持作为独立参数(“ %u ”),不支持组合（“\"--file %u\"” 或者“\"--file-%u\"” ）
 */
QPair<QString, QStringList> DCustomActionBuilder::makeCommand(const QString &cmd, DCustomActionDefines::ActionArg arg, const DUrl &dir, const DUrl &foucs, const DUrlList &files)
{
    QPair<QString, QStringList> ret;
    auto args = splitCommand(cmd);
    if (args.isEmpty()) {
        return ret;
    }

    //执行程序
    ret.first = args.takeFirst();
    //无参数
    if (args.isEmpty()) {
        return ret;
    }

    auto replace = [ = ](QStringList & args, const QString & before, const QString & after) {
        QStringList rets;
        while (!args.isEmpty()) {
            QString arg = args.takeFirst();
            //找到在参数中第一个有效的before匹配值，并替换为after。之后的不在处理
            int index = arg.indexOf(before);
            if (index >= 0) {
                rets << arg.replace(index, before.size(), after);
                rets << args;
                args.clear();
            } else {
                rets << arg;
            }
        }
        return rets;
    };

    auto replaceList = [ = ](QStringList & args, const QString & before, const QStringList & after) {
        QStringList rets;
        while (!args.isEmpty()) {
            QString arg = args.takeFirst();
            //仅支持独立参数，有其它组合的不处理
            if (arg == before) {
                //放入文件路径
                rets << after;
                //放入原参数
                rets << args;
                args.clear();
            } else {
                rets << arg;
            }
        }
        return rets;
    };

    //url转为文件路径
    auto urlListToLocalFile = [](const DUrlList & files) {
        QStringList rets;
        for (auto it = files.begin(); it != files.end(); ++it) {
            rets << it->toLocalFile();
        }
        return rets;
    };

    //url字符串
    auto urlListToString = [](const DUrlList & files) {
        QStringList rets;
        for (auto it = files.begin(); it != files.end(); ++it) {
            rets << it->toString();
        }
        return rets;
    };

    //传参
    switch (arg) {
    case DCustomActionDefines::DirPath:
        ret.second = replace(args, DCustomActionDefines::kStrActionArg[arg], dir.toLocalFile());
        break;
    case DCustomActionDefines::FilePath:
        ret.second = replace(args, DCustomActionDefines::kStrActionArg[arg], foucs.toLocalFile());
        break;
    case DCustomActionDefines::FilePaths:
        ret.second = replaceList(args, DCustomActionDefines::kStrActionArg[arg], urlListToLocalFile(files));
        break;
    case DCustomActionDefines::UrlPath:
        ret.second = replace(args, DCustomActionDefines::kStrActionArg[arg], foucs.toString());
        break;
    case DCustomActionDefines::UrlPaths:
        ret.second = replaceList(args, DCustomActionDefines::kStrActionArg[arg], urlListToString(files));
        break;
    default:
        ret.second = args;
        break;
    }
    return ret;
}

/*!
    将命令行 \a cmd 中的数据使用空格（" "）分割。
    引号内的空格不做分割。
 */
QStringList DCustomActionBuilder::splitCommand(const QString &cmd)
{
    QStringList args;
    bool inQuote = false;

    QString arg;
    for (int i = 0 ; i < cmd.count(); i++) {
        const bool isEnd = (cmd.size() == (i + 1));

        const QChar &ch = cmd.at(i);
        //引号
        const bool isQuote = (ch == QLatin1Char('\'') || ch == QLatin1Char('\"'));

        //遇到引号或者最后一个字符
        if (!isEnd && isQuote) {
            //进入引号内或退出引号
            inQuote = !inQuote;
        } else {
            //处于引号中或者非空格作为一个参数
            if ((!ch.isSpace() || inQuote) && !isQuote) {
                arg.append(ch);
            }

            //遇到空格且不再引号中解出一个单独参数
            if ((ch.isSpace() && !inQuote) || isEnd) {
                if (!arg.isEmpty()) {
                    args << arg;
                }
                arg.clear();
            }
        }
    }
    return args;
}

/*!
    创建菜单项，\a parentForSubmenu 用于指定菜单的父对象，用于自动释放
    通过获取 \a actionData 中的标题，图标等信息创建菜单项，并遍历创建子项和分割符号。
    返回值 QAction* 对象的生命周期由调用方控制。
*/
QAction *DCustomActionBuilder::createMenu(const DCustomActionData &actionData, QWidget *parentForSubmenu) const
{
    //fix-bug 59298
    //createAction 构造action 图标等, 把关于构造action参数放在createAction中
    QAction *action = createAciton(actionData);
    QMenu *menu = new DFileMenu(parentForSubmenu);
    menu->setToolTipsVisible(true);

    action->setMenu(menu);
    action->setProperty(DCustomActionDefines::kCustomActionFlag, true);

    //子项,子项的顺序由解析器保证
    QList<DCustomActionData> subActions = actionData.acitons();
    for (auto it = subActions.begin(); it != subActions.end(); ++it) {
        QAction *ba = buildAciton(*it, parentForSubmenu);
        if (!ba)
            continue;

        auto separator = it->separator();
        //上分割线
        if (separator & DCustomActionDefines::Top) {
            const QList<QAction *> &actionList = menu->actions();
            if (!actionList.isEmpty()) {
                auto lastAction = menu->actions().last();

                //不是分割线则插入
                if (!lastAction->isSeparator()) {
                    menu->addSeparator();
                }
            }
        }

        ba->setParent(menu);
        menu->addAction(ba);

        //下分割线
        if ((separator & DCustomActionDefines::Bottom) && ((it + 1) != subActions.end())) {
            menu->addSeparator();
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
    action->setProperty(DCustomActionDefines::kCustomActionCommandArgFlag, actionData.commandArg());

    //标题
    {
        const QString &&name = makeName(actionData.name(), actionData.nameArg());
        //TODO width是临时值，最终效果需设计定义
        const QString &&elidedName = m_fm.elidedText(name, Qt::ElideMiddle, 150);
        action->setText(elidedName);
        if (elidedName != name)
            action->setToolTip(name);
    }

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
    QFileInfo info(iconName.startsWith("~") ? (QDir::homePath() + iconName.mid(1)) : iconName);

    if (!info.exists())
        info.setFile(QUrl::fromUserInput(iconName).toLocalFile());

    if (info.exists()) {
        ret = QIcon(info.absoluteFilePath());
    }

    //从主题获取
    if (ret.isNull()) {
        ret = QIcon::fromTheme(iconName);
    }

    return ret;
}

/*!
    使用当前文件或文件夹信息替换 \a name 中的 \a arg 参数。
    只处理找到的一个有效的 \a arg 参数，后面的不再替换。
    参数类型仅支持：DirName BaseName FileName
 */
QString DCustomActionBuilder::makeName(const QString &name, DCustomActionDefines::ActionArg arg) const
{
    auto replace = [](QString input, const QString & before, const QString & after) {
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
        ret = replace(name, DCustomActionDefines::kStrActionArg[arg], m_dirName);
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
