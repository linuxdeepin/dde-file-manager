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

    static const char* const kCustomActionFlag = "Custom_Action_Flag";
    static const char* const kCustomActionCommand = "Custom_Action_Command";
    static const int kCustomMaxNumOne = 30;
    static const int kCustomMaxNumTwo = 50;
    static const int kCustomMaxNumThree = 50;
}
#endif // DCUSTOMACTIONDEFINE_H
