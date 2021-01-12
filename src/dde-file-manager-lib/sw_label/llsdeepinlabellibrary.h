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

#ifndef LLSDEEPINLABELLIBRARY_H
#define LLSDEEPINLABELLIBRARY_H

#include <QString>
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

class LlsDeepinLabelLibrary
{
public:

    static LlsDeepinLabelLibrary* instance(){
          static LlsDeepinLabelLibrary instance;
          return &instance;
    }

    inline static QString LibraryName(){
        return "/usr/lib/sw_64-linux-gnu/dde-file-manager/libllsdeeplabel.so";
    }

    void loadLibrary(const QString& fileName);

    typedef int (*func_lls_simplechecklabel)(char *filename);
    typedef int (*func_lls_fullchecklabel)(char *filename);
    typedef int (*func_lls_checkprivilege)(char *srcfilename, char *dstfilename, int iFilePrivilege56);
    typedef int (*func_lls_geterrordesc)(int ierrorcode, string &serrordesc, int &ierrorlevel);

    bool isCompletion() const;

    func_lls_simplechecklabel lls_simplechecklabel() const;
    void setLls_simplechecklabel(const func_lls_simplechecklabel &lls_simplechecklabel);

    func_lls_fullchecklabel lls_fullchecklabel() const;
    void setLls_fullchecklabel(const func_lls_fullchecklabel &lls_fullchecklabel);

    func_lls_checkprivilege lls_checkprivilege() const;
    void setLls_checkprivilege(const func_lls_checkprivilege &lls_checkprivilege);

    func_lls_geterrordesc lls_geterrordesc() const;
    void setLls_geterrordesc(const func_lls_geterrordesc &lls_geterrordesc);

private:
    LlsDeepinLabelLibrary();
    LlsDeepinLabelLibrary(const LlsDeepinLabelLibrary &);
    LlsDeepinLabelLibrary & operator = (const LlsDeepinLabelLibrary &);

    bool m_isCompletion = false;
    func_lls_simplechecklabel m_lls_simplechecklabel = NULL;
    func_lls_fullchecklabel m_lls_fullchecklabel = NULL;
    func_lls_checkprivilege m_lls_checkprivilege = NULL;
    func_lls_geterrordesc m_lls_geterrordesc = NULL;
};

#endif // LLSDEEPINLABELLIBRARY_H
