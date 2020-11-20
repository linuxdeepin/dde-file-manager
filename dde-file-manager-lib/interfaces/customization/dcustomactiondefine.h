/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zhangyu <zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu <zhangyub@uniontech.com>
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
    enum FileComboTypes {
        BlankSpace = 0,      //空白区域
        SingleFile = 1,      //单个文件
        SingleDir = 1 << 1,  //单个文件夹
        MultiFiles = 1 << 2, //多个文件
        MultiDirs = 1 << 3,  //多个文件夹
        FileAndDir = 1 << 4 //文件和文件夹，必须都包含
    };

    //支持菜单层级
    enum MenuHierarchy {
        One = 1,
        Two,
        Three
    };

    template<typename Element, typename AddFunc>
    void sortFunc(const QMap<int, QList<Element>> &locate, QList<Element> &orgin, AddFunc addfunc) {
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
                    for (int j = 0; j < added; ++j) {
                        if (orgin.isEmpty())
                            break;

                        addfunc({orgin.takeFirst()});
                        ++currentCount;
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
    static const int kCustomMaxNumOne = 30;
    static const int kCustomMaxNumTwo = 50;
    static const int kCustomMaxNumThree = 50;

    //基本信息，置于一级菜单之中
    static const QLatin1String kUosPrefix("Uos Entry");
    static const QLatin1String kFileVersion("Version");
    static const QLatin1String kComment("Comment");
    static const QLatin1String kCombo("X-DFM-MenuTypes");

    //菜单基本信息
    static const QLatin1String kActionGenericName("GenericName");
    static const QLatin1String kActionName("Name");
    static const QLatin1String kActionPos("PosNum");
    static const QLatin1String kActionSeparator("Separator");
    static const QLatin1String kActionCmd("Exec");
    static const QLatin1String kActionGroups("Actions");
    static const QLatin1String kActionPrefix("Uos Action");

    Q_DECLARE_FLAGS(ComboTypes, DCustomActionDefines::FileComboTypes);
    Q_DECLARE_OPERATORS_FOR_FLAGS(ComboTypes);

    struct FileBasicInfos {
        QString m_package;  //配置文件名
        QString m_version;  //版本
        QString m_comment;  //描述
        ComboTypes m_fileCombo;   //支持的选中项类型
    };
}

#endif // DCUSTOMACTIONDEFINE_H
