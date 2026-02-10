// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCUSTOMACTIONDEFINE_H
#define DCUSTOMACTIONDEFINE_H

#include "dfmplugin_menu_global.h"

#include <QString>
#include <QMap>

namespace dfmplugin_menu {

namespace DCustomActionDefines {
// 分割线
enum Separator {
    kNone = 0,
    kTop = 1,
    kBottom = 1 << 1,
    kBoth = kTop | kBottom
};

// 支持的文件组合方式
enum ComboType {
    kBlankSpace = 1,   //空白区域
    kSingleFile = 1 << 1,   //单个文件
    kSingleDir = 1 << 2,   //单个文件夹
    kMultiFiles = 1 << 3,   //多个文件
    kMultiDirs = 1 << 4,   //多个文件夹
    kFileAndDir = 1 << 5,   //文件和文件夹，必须都包含。焦点优化后废弃，当做kMultiDirs|kMultiFiles
    kAllFile = kSingleFile | kMultiFiles,
    kAllDir = kSingleDir | kMultiDirs,
    kAllCombo = kBlankSpace | kAllFile | kAllDir | kFileAndDir
};
Q_DECLARE_FLAGS(ComboTypes, DCustomActionDefines::ComboType);
Q_DECLARE_OPERATORS_FOR_FLAGS(ComboTypes);

// 支持菜单层级
enum MenuHierarchy {
    kOne = 1,
    kTwo,
    kThree
};

// 动态参数
enum ActionArg {
    kNoneArg = -1,
    kDirName = 0,   //%d 当前文件夹名称
    kDirPath,   //%p 当前文件夹路径
    kBaseName,   //%b 文件名，不含路径和扩展名
    kFileName,   //%a 文件全名，包含扩展名，不含路径
    kFilePath,   //%f 文件路径
    kFilePaths,   //%F 多个文件路径
    kUrlPath,   //%u 使用url表示的文件路径
    kUrlPaths,   //%U 多个url
};

static const char *const kStrActionArg[] { "%d", "%p", "%b", "%a", "%f", "%F", "%u", "%U" };

template<typename Element, typename AddFunc, typename IsCount>
void sortFunc(const QMap<int, QList<Element>> &locate, QList<Element> &orgin, AddFunc addfunc, IsCount countFunc)
{
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
                    addfunc({ el });

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

static const char *const kCustomActionFlag = "Custom_Action_Flag";
static const char *const kCustomActionCommand = "Custom_Action_Command";
static const char *const kCustomActionCommandArgFlag = "Custom_Action_Command_Arg_Flag";
static const int kCustomMaxNumOne = 50;
static const int kCustomMaxNumTwo = 100;
static const int kCustomMaxNumThree = 100;

//基本信息，置于一级菜单之中
static const char *const kMenuPrefix("Menu Entry");
static const char *const kConfFileVersion("Version");
static const char *const kConfComment("Comment");
static const char *const kConfCombo("X-DDE-FileManager-MenuTypes");
static const char *const kConfComboAlias("X-DFM-MenuTypes");
static const char *const kConfMimeType("MimeType");
static const char *const kConfExcludeMimeTypes("X-DDE-FileManager-ExcludeMimeTypes");
static const char *const kConfExcludeMimeTypesAlias("X-DFM-ExcludeMimeTypes");
static const char *const kConfSupportSchemes("X-DDE-FileManager-SupportSchemes");
static const char *const kConfSupportSchemesAlias("X-DFM-SupportSchemes");
static const char *const kConfNotShowIn("X-DDE-FileManager-NotShowIn");   // "Desktop", "Filemanager"
static const char *const kConfNotShowInAlias("X-DFM-NotShowIn");
static const char *const kConfSupportSuffix("X-DDE-FileManager-SupportSuffix");   // for deepin-compress *.7z.001,*.7z.002,*.7z.003...
static const char *const kConfSupportSuffixAlias("X-DFM-SupportSuffix");
static const char *const kConfParentMenuPath("X-DFM-ParentMenuPath");
static const char *const kConfSign("Sign");

//菜单基本信息
static const char *const kActionGenericName("GenericName");
static const char *const kActionName("Name");
static const char *const kActionIcon("Icon");
static const char *const kActionPos("X-DDE-FileManager-PosNum");
static const char *const kActionPosAlias("PosNum");
static const char *const kActionSeparator("X-DDE-FileManager-Separator");
static const char *const kActionSeparatorAlias("Separator");
static const char *const kActionCmd("Exec");
static const char *const kActionGroups("Actions");
static const char *const kActionPrefix("Menu Action");

struct FileBasicInfos
{
    QString package;   //配置文件名
    QString version;   //版本
    QString comment;   //描述
    ComboTypes fileCombo;   //支持的选中项类型
    QString sign;   //签名
};
}

}

#endif   // DCUSTOMACTIONDEFINE_H
