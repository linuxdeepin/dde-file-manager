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

#include "llsdeepinlabellibrary.h"

#include <QLibrary>
#include <QDebug>

LlsDeepinLabelLibrary::LlsDeepinLabelLibrary()
{
    loadLibrary(LibraryName());
}

void LlsDeepinLabelLibrary::loadLibrary(const QString &fileName)
{
    QLibrary lib(fileName);
    bool isLoaded = lib.load();
    if (isLoaded){
        m_lls_simplechecklabel = (func_lls_simplechecklabel) lib.resolve("lls_simplechecklabel");
        m_lls_fullchecklabel = (func_lls_fullchecklabel) lib.resolve("lls_fullchecklabel");
        m_lls_checkprivilege = (func_lls_checkprivilege) lib.resolve("lls_checkprivilege");
        m_lls_geterrordesc = (func_lls_geterrordesc) lib.resolve("lls_geterrordesc");
        qDebug() << fileName << m_lls_simplechecklabel << m_lls_fullchecklabel << m_lls_checkprivilege << m_lls_geterrordesc << lib.errorString();
        if (m_lls_simplechecklabel && m_lls_fullchecklabel && m_lls_checkprivilege && m_lls_geterrordesc){
            m_isCompletion = true;
        }
    }else{
       qDebug() << "load library " << LibraryName() << "failed: " << lib.errorString();
    }
}

bool LlsDeepinLabelLibrary::isCompletion() const
{
    return m_isCompletion;
}

LlsDeepinLabelLibrary::func_lls_geterrordesc LlsDeepinLabelLibrary::lls_geterrordesc() const
{
    return m_lls_geterrordesc;
}

void LlsDeepinLabelLibrary::setLls_geterrordesc(const func_lls_geterrordesc &lls_geterrordesc)
{
    m_lls_geterrordesc = lls_geterrordesc;
}

LlsDeepinLabelLibrary::func_lls_checkprivilege LlsDeepinLabelLibrary::lls_checkprivilege() const
{
    return m_lls_checkprivilege;
}

void LlsDeepinLabelLibrary::setLls_checkprivilege(const func_lls_checkprivilege &lls_checkprivilege)
{
    m_lls_checkprivilege = lls_checkprivilege;
}

LlsDeepinLabelLibrary::func_lls_fullchecklabel LlsDeepinLabelLibrary::lls_fullchecklabel() const
{
    return m_lls_fullchecklabel;
}

void LlsDeepinLabelLibrary::setLls_fullchecklabel(const func_lls_fullchecklabel &lls_fullchecklabel)
{
    m_lls_fullchecklabel = lls_fullchecklabel;
}

LlsDeepinLabelLibrary::func_lls_simplechecklabel LlsDeepinLabelLibrary::lls_simplechecklabel() const
{
    return m_lls_simplechecklabel;
}

void LlsDeepinLabelLibrary::setLls_simplechecklabel(const func_lls_simplechecklabel &lls_simplechecklabel)
{
    m_lls_simplechecklabel = lls_simplechecklabel;
}
