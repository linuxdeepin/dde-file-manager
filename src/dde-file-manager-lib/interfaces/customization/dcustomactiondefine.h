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

#ifndef DCUSTOMACTIONDEFINE_H
#define DCUSTOMACTIONDEFINE_H

#include <QString>
#include <QMap>

namespace DCustomActionDefines
{
    //分割线
    enum Separator {
        None = 0,
        Top = 1,
        Bottom = 1 << 1,
        Both = Top | Bottom
    };

    //支持的文件组合方式
    enum ComboType {
        BlankSpace = 1,      //空白区域
        SingleFile = 1 << 1,      //单个文件
        SingleDir = 1 << 2,  //单个文件夹
        MultiFiles = 1 << 3, //多个文件
        MultiDirs = 1 << 4,  //多个文件夹
        FileAndDir = 1 << 5, //文件和文件夹，必须都包含
        AllFile = SingleFile | MultiFiles,
        AllDir = SingleDir | MultiDirs,
        AllCombo = BlankSpace | AllFile | AllDir | FileAndDir
    };
    Q_DECLARE_FLAGS(ComboTypes, DCustomActionDefines::ComboType);
    Q_DECLARE_OPERATORS_FOR_FLAGS(ComboTypes);

    //支持菜单层级
    enum MenuHierarchy {
        One = 1,
        Two,
        Three
    };

    //动态参数
    enum ActionArg {
        NoneArg = -1,
        DirName = 0, //%d 当前文件夹名称
        DirPath, //%p 当前文件夹路径
        BaseName, //%b 文件名，不含路径和扩展名
        FileName, //%a 文件全名，包含扩展名，不含路径
        FilePath,//%f 文件路径
        FilePaths, //%F 多个文件路径
        UrlPath, //%u 使用url表示的文件路径
        UrlPaths, //%U 多个url
    };

    static const QStringList kStrActionArg = {"%d","%p","%b","%a","%f","%F","%u","%U"};

    template<typename Element, typename AddFunc,typename IsCount>
    void sortFunc(const QMap<int, QList<Element>> &locate, QList<Element> &orgin, AddFunc addfunc,IsCount countFunc) {
        int maxPos = locate.isEmpty() ? 0 : locate.lastKey();
        int currentCount = 0;
        auto nextIter = locate.begin();
        for (int i = 1; i <= maxPos; ++i) {
            auto current = locate.find(i);
            if (current == locate.end()) {
                if (orgin.isEmpty()) {
                    //已经取完，剩下的全部按顺序放入
                    for (; nextIter != locate.end(); ++nextIter) {
                        currentCount += nextIter->size();
                        addfunc(*nextIter);
                    }
                    break;
                } else {
                    //用原有数据占位
                    int added = i - currentCount;
                    for (int j = 0; j < added;) {
                        if (orgin.isEmpty())
                            break;
                        auto el = orgin.takeFirst();
                        addfunc({el});

                        //是否记数
                        if (countFunc(el)) {
                            ++j;
                            ++currentCount;
                        }
                    }
                }
            } else {
                currentCount += current->size();
                addfunc(*current);
                nextIter = current + 1;
            }
        }

        //最后剩下的
        addfunc(orgin);
        orgin.clear();
    }

    static const char* const kCustomActionFlag = "Custom_Action_Flag";
    static const char* const kCustomActionCommand = "Custom_Action_Command";
    static const char* const kCustomActionCommandArgFlag = "Custom_Action_Command_Arg_Flag";
    static const char* const kCustomActionDataDir = "Custom_Action_Data_Dir";
    static const char* const kCustomActionDataFoucsFile = "Custom_Action_Foucs_File";
    static const char* const kCustomActionDataSelectedFiles = "Custom_Action_Seletcted_Files";
    static const int kCustomMaxNumOne = 50;
    static const int kCustomMaxNumTwo = 100;
    static const int kCustomMaxNumThree = 100;

    //基本信息，置于一级菜单之中
    static const QLatin1String kMenuPrefix("Menu Entry");
    static const QLatin1String kConfFileVersion("Version");
    static const QLatin1String kConfComment("Comment");
    static const QLatin1String kConfCombo("X-DFM-MenuTypes");
    static const QLatin1String kConfMimeType("MimeType");
    static const QLatin1String kConfExcludeMimeTypes("X-DFM-ExcludeMimeTypes");
    static const QLatin1String kConfSupportSchemes("X-DFM-SupportSchemes");
    static const QLatin1String kConfNotShowIn("X-DFM-NotShowIn");     // "Desktop", "Filemanager"
    static const QLatin1String kConfSupportSuffix("X-DFM-SupportSuffix"); // for deepin-compress *.7z.001,*.7z.002,*.7z.003...
    static const QLatin1String kConfSign("Sign");

    //菜单基本信息
    static const QLatin1String kActionGenericName("GenericName");
    static const QLatin1String kActionName("Name");
    static const QLatin1String kActionIcon("Icon");
    static const QLatin1String kActionPos("PosNum");
    static const QLatin1String kActionSeparator("Separator");
    static const QLatin1String kActionCmd("Exec");
    static const QLatin1String kActionGroups("Actions");
    static const QLatin1String kActionPrefix("Menu Action");

    //指定路径
    static const QLatin1String kCustomMenuPath("/usr/share/applications/context-menus");

    struct FileBasicInfos {
        QString m_package;  //配置文件名
        QString m_version;  //版本
        QString m_comment;  //描述
        ComboTypes m_fileCombo;   //支持的选中项类型
        QString m_sign;     //签名
    };
}

#endif // DCUSTOMACTIONDEFINE_H
