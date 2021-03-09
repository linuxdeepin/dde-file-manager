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

#include "dcustomactionbuilder.h"
#include "dfilemenu.h"

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
        //baseName
        if (info->isDir()) {
            m_fileBaseName = m_fileFullName;
            return;
        }
        //fix bug 65159,这里只针对一些常见的后缀处理，暂不针对一些非标准的特殊情况做处理,待后续产品有特殊要求再处理特殊情况
        //suffixForFileName对复式后缀会返回xx.*,比如test.7z.001返回的是7z.*
        //不过在一些非标准的复式后缀判定中，仍可能判定不准确：比如：test.part1.rar被识别成rar
        //隐藏文件：".tar"、".tar.gz"后缀识别成""和".gz"
        //可能无法识别到后缀：如test.run或者.tar
        QString suffix = mimeDatabase.suffixForFileName(m_fileFullName);
        if (suffix.isEmpty()) {
            m_fileBaseName = m_fileFullName;
            return;
        }
        //二次过滤后缀，方式识别到分卷带*的情况
        suffix = this->getCompleteSuffix(m_fileFullName, suffix);
        m_fileBaseName = m_fileFullName.left(m_fileFullName.length() - suffix.length() - 1);

        //解决 .xx 一类的隐藏文件
        if (m_fileBaseName.isEmpty())
            m_fileBaseName = m_fileFullName;
    }
}

/*!
    过滤识别结果带*的情况，返回当前文件名的实际全后缀(已经经过DMimeDatabase识别后且为复式后缀后使用)。
    检查 \a fileName 待划分后缀的全文件名， \a suf 经过DMimeDatabase识别后的复式后缀。
 */
QString DCustomActionBuilder::getCompleteSuffix(const QString &fileName, const QString &suf)
{
    QString tempStr;
    if (!suf.contains(".") || suf.isEmpty())
        return suf;
    auto sufLst = suf.split(".");
    if (0 < sufLst.size()) {
        tempStr = sufLst.first();
        int index = fileName.lastIndexOf(tempStr);
        if (index > 0) {
            return fileName.mid(index);;
        }
    }
    return suf;
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
QList<DCustomActionEntry> DCustomActionBuilder::matchFileCombo(const QList<DCustomActionEntry> &rootActions,
                                                               DCustomActionDefines::ComboTypes type)
{
    QList<DCustomActionEntry> ret;
    //无自定义菜单项
    if(0 == rootActions.size())
        return ret;

    for (auto it = rootActions.begin(); it != rootActions.end(); ++it) {
        if (it->fileCombo() & type)
            ret << *it;
    }
    return ret;
}

QList<DCustomActionEntry> DCustomActionBuilder::matchActions(const DUrlList &selects,
                                        QList<DCustomActionEntry> oriActions)
{
    //todo：细化功能颗粒度，一个函数尽量专职一件事
    /*
     *根据选中内容、配置项、选中项类型匹配合适的菜单项
     *是否action支持的协议
     *是否action支持的后缀
     *action不支持类型过滤（不加上父类型过滤，todo: 为何不支持项不考虑?）
     *action支持类型过滤(类型过滤要加上父类型一起过滤)
    */

    //具体配置过滤
    for (auto &singleUrl : selects) {
        //协议、后缀
        DAbstractFileInfoPointer fileInfo;
        fileInfo = DFileService::instance()->createFileInfo(nullptr, singleUrl);
        if (!fileInfo) {
            qWarning() << "create selected FileInfo failed: " << singleUrl.toString();
            continue;
        }

        /*
         * 选中文件类型过滤：
         * fileMimeTypes:包括所有父类型的全量类型集合
         * fileMimeTypesNoParent:不包含父类mimetype的集合
         * 目的是在一些应用对文件的识别支持上有差异：比如xlsx的 parentMimeTypes 是application/zip
         * 归档管理器打开则会被作为解压
        */

        QStringList fileMimeTypes;
        QStringList fileMimeTypesNoParent;
//        fileMimeTypes.append(fileInfo->mimeType().name());
//        fileMimeTypes.append(fileInfo->mimeType().aliases());
//        const QMimeType &mt = fileInfo->mimeType();
//        fileMimeTypesNoParent = fileMimeTypes;
//        appendParentMimeType(mt.parentMimeTypes(), fileMimeTypes);
//        fileMimeTypes.removeAll({});
//        fileMimeTypesNoParent.removeAll({});

        appendAllMimeTypes(fileInfo, fileMimeTypesNoParent, fileMimeTypes);
        for (auto it = oriActions.begin(); it != oriActions.end();) {
            DCustomActionEntry &tempAction = *it;
            //协议，后缀
            if (!isSchemeSupport(tempAction, singleUrl) || !isSuffixSupport(tempAction, singleUrl)) {
                it = oriActions.erase(it);   //不支持的action移除
                continue;
            }

            //不支持的mimetypes,使用不包含父类型的mimetype集合过滤
            if (isMimeTypeMatch(fileMimeTypesNoParent, tempAction.excludeMimeTypes())) {
                it = oriActions.erase(it);
                continue;
            }

            // MimeType在原有oem中，未指明或Mimetype=*都作为支持所有类型
            if (tempAction.mimeTypes().isEmpty()) {
                ++it;
                continue;
            }

            //支持的mimetype,使用包含父类型的mimetype集合过滤
            QStringList supportMimeTypes =  tempAction.mimeTypes();
            supportMimeTypes.removeAll({});
            auto match = isMimeTypeMatch(fileMimeTypes, supportMimeTypes);

//在自定义右键菜中有作用域限制，此类情况不显示自定义菜单，故可屏蔽，若后续有作用域的调整再考虑是否开放
#if 0
            //部分mtp挂载设备目录下文件属性不符合规范(普通目录mimetype被认为是octet-stream)，暂时做特殊处理-
            if (singleUrl.path().contains("/mtp:host") && supportMimeTypes.contains("application/octet-stream") && fileMimeTypes.contains("application/octet-stream"))
                match = false;
#endif
            if (!match) {
                it = oriActions.erase(it);
                continue;
            }
            ++it;
        }

    }

    return oriActions;
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

bool DCustomActionBuilder::isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes)
{
    foreach (const QString &fmt, fileMimeTypes) {
        if (fmt.contains(mt, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

bool DCustomActionBuilder::isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes)
{
    bool match = false;
    for (const QString &mt : supportMimeTypes) {
        if (fileMimeTypes.contains(mt, Qt::CaseInsensitive)) {
            match = true;
            break;
        }

        int starPos = mt.indexOf("*");
        if (starPos >= 0 && isMimeTypeSupport(mt.left(starPos), fileMimeTypes)) {
            match = true;
            break;
        }
    }
    return match;
}

bool DCustomActionBuilder::isSchemeSupport(const DCustomActionEntry &action, const DUrl &url)
{
    // X-DFM-SupportSchemes not exist
    auto supportList = action.surpportSchemes();
    if (supportList.contains("*") || supportList.isEmpty())
        return true;    //支持所有协议: 未特殊指明X-DFM-SupportSchemes或者"X-DFM-SupportSchemes=*"
    return supportList.contains(url.scheme(), Qt::CaseInsensitive);
}

bool DCustomActionBuilder::isSuffixSupport(const DCustomActionEntry &action, const DUrl &url)
{
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(nullptr, url);
    auto supportList =  action.supportStuffix();
    if (!fileInfo || fileInfo->isDir() || supportList.isEmpty() || supportList.contains("*")) {
        return true;//未特殊指明支持项或者包含*为支持所有
    }
    QFileInfo info(url.toLocalFile());

    //例如： 7z.001,7z.002, 7z.003 ... 7z.xxx
    QString cs = info.completeSuffix();
    if (supportList.contains(cs, Qt::CaseInsensitive)) {
        return true;
    }

    bool match = false;
    for (const QString &suffix : supportList) {
        auto tempSuffix = suffix;
        int endPos = tempSuffix.lastIndexOf("*"); // 例如：7z.*
        if (endPos >= 0 && cs.length() > endPos && tempSuffix.left(endPos) == cs.left(endPos)) {
            match = true;
            break;
        }
    }
    return match;
}

void DCustomActionBuilder::appendAllMimeTypes(const DAbstractFileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes)
{
    noParentmimeTypes.append(fileInfo->mimeType().name());
    noParentmimeTypes.append(fileInfo->mimeType().aliases());
    const QMimeType &mt = fileInfo->mimeType();
    allMimeTypes = noParentmimeTypes;
    appendParentMimeType(mt.parentMimeTypes(), allMimeTypes);
    noParentmimeTypes.removeAll({});
    allMimeTypes.removeAll({});
}

void DCustomActionBuilder::appendParentMimeType(const QStringList &parentmimeTypes, QStringList &mimeTypes)
{
    if (parentmimeTypes.size() == 0)
        return;

    for (const QString &mtName : parentmimeTypes) {
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForName(mtName);
        mimeTypes.append(mt.name());
        mimeTypes.append(mt.aliases());
        QStringList pmts = mt.parentMimeTypes();
        appendParentMimeType(pmts, mimeTypes);
    }
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
//story#4481,产品变更暂不考虑图标
#if 0
    //图标
    const QString &iconName = actionData.icon();
    if (!iconName.isEmpty()) {
        const QIcon &&icon = getIcon(iconName);
        if (!icon.isNull())
            action->setIcon(icon);
    }
#endif
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
