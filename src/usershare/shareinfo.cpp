/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "shareinfo.h"

ShareInfo::ShareInfo(QString shareName, QString path, QString comment, bool isWritable, bool isGuestOk)
{
    setShareName(shareName);
    setPath(path);
    setComment(comment);
    setIsWritable(isWritable);
    setIsGuestOk(isGuestOk);
}

ShareInfo::~ShareInfo()
{

}

QString ShareInfo::shareName() const
{
    return m_shareName;
}

void ShareInfo::setShareName(const QString &shareName)
{
    m_shareName = shareName;
}

QString ShareInfo::path() const
{
    return m_path;
}

void ShareInfo::setPath(const QString &path)
{
    m_path = path;
}

QString ShareInfo::comment() const
{
    return m_comment;
}

void ShareInfo::setComment(const QString &comment)
{
    m_comment = comment;
    if (m_comment.isEmpty()){
        m_comment = "\"\"";
    }
}

QString ShareInfo::usershare_acl() const
{
    return m_usershare_acl;
}

void ShareInfo::setUsershare_acl(const QString &usershare_acl)
{
    m_usershare_acl = usershare_acl;
    if (m_usershare_acl.isEmpty()){
        m_usershare_acl = "\"\"";
    }

    if(m_usershare_acl.contains("Everyone:R")){
        m_isWritable = false;
    }else{
        m_isWritable = true;
    }
}

QString ShareInfo::guest_ok() const
{
    return m_guest_ok;
}

void ShareInfo::setGuest_ok(const QString &guest_ok)
{
    m_guest_ok = guest_ok;
    if (m_guest_ok.isEmpty()){
        m_guest_ok = "n";
    }
    if(guest_ok == "n"){
        m_isGuestOk = false;
    }else{
        m_isGuestOk = true;
    }
}

bool ShareInfo::isWritable() const
{
    return m_isWritable;
}

void ShareInfo::setIsWritable(bool isWritable)
{
    m_isWritable = isWritable;
    if (m_isWritable){
        m_usershare_acl = "Everyone:f";
    }else{
        m_usershare_acl = "Everyone:r";
    }
}

bool ShareInfo::isGuestOk() const
{
    return m_isGuestOk;
}

void ShareInfo::setIsGuestOk(bool isGuestOk)
{
    m_isGuestOk = isGuestOk;

    if (m_isGuestOk){
        m_guest_ok = "guest_ok=y";
    }else{
        m_guest_ok = "guest_ok=n";
    }
}

bool ShareInfo::isValid()
{
    if (!m_shareName.isEmpty() && QFile(m_path).exists()){
        return true;
    }
    return false;
}



QDebug operator<<(QDebug dbg, const ShareInfo &obj)
{
    dbg.nospace() << "{";
    dbg.nospace() << "shareName:" << obj.shareName() << ",";
    dbg.nospace() << "path:" << obj.path() << ",";
    dbg.nospace() << "comment:" << obj.comment() << ",";
    dbg.nospace() << "usershare_acl:" << obj.usershare_acl() << ",";
    dbg.nospace() << "guest_ok:" << obj.guest_ok() << ",";
    dbg.nospace() << "}";
    return dbg;
}
