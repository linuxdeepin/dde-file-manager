/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#pragma once
#include <string>
using namespace std;

//文件操作行为/操作权限
enum FilePrivilege56
{
	E_FILE_PRI_READ=1,		//1、可读
	E_FILE_PRI_WRITE=2,		//2、可写
	E_FILE_PRI_PRINT=3,		//3、可打印
        E_FILE_PRI_COPY=4,		//4、可复制
	E_FILE_PRI_DVD=5,		//5、可刻光盘
	E_FILE_PRI_STORE=6,		//6、可存储到移动介质
        E_FILE_PRI_RENAME=7,	//7、可重命名
	E_FILE_PRI_DELETE=8,		//8、可删除
	E_FILE_PRI_MOVE=9,		//9、可移动
        E_FILE_PRI_COPY_CONTENT=10,  //10、文件内容复制粘贴

};

//错误级别
enum ErrorLevel
{
        ERR_INFO=1,                         //1、提示信息
        ERR_WARNING=2,          	//2、警告
        ERR_ERROR=3,                     //3、错误
};


//-----------------------------------------------
//功能描述：简单校验是否为标签文件（不准确，但效率高）
//参数：filename=文件全路径名称
//返回：0=标签文件；其它=非标签文件
//说明：
//-----------------------------------------------
int lls_simplechecklabel(char *filename);

//-----------------------------------------------
//功能描述：完整检验是否为标签文件（准确，但效率低）
//参数：filename=文件全路径名称
//返回：0=标签文件；其它=非标签文件
//说明：
//-----------------------------------------------
int lls_fullchecklabel(char *filename);

//-----------------------------------------------
//功能描述：判断文件是否允许执行该操作（是否具有操作权限）
//参数：srcfilename=文件全路径名称;
//          dstfilename=目标文件全路径名称;
//          iFilePrivilege56=文件操作权限/行为
//返回：0=有权限；其它=错误码
//说明：
//-----------------------------------------------
int lls_checkprivilege(char *srcfilename, char *dstfilename, int iFilePrivilege56);

//-----------------------------------------------
//功能描述：根据错误码，获取“错误级别”和“错误描述”
//参数：ierrorcode=错误码；
//          serrordesc=错误描述；
//          iErrorLevel=错误级别
//返回：0=成功；其它=失败
//说明：
//-----------------------------------------------
int lls_geterrordesc(int ierrorcode, string &serrordesc, int &ierrorlevel);
