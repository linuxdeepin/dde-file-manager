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

#ifndef SHAREINFO_H
#define SHAREINFO_H

#include <QString>
#include <QFile>
#include <QDebug>

class ShareInfo
{
public:
    ShareInfo(QString shareName = "",
              QString path = "",
              QString comment = "",
              bool isWritable = false,
              bool isGuestOk = false);
    ~ShareInfo();

    QString shareName() const;
    void setShareName(const QString &shareName);

    QString path() const;
    void setPath(const QString &path);

    QString comment() const;
    void setComment(const QString &comment);

    QString usershare_acl() const;
    void setUsershare_acl(const QString &usershare_acl);

    QString guest_ok() const;
    void setGuest_ok(const QString &guest_ok);

    bool isWritable() const;
    void setIsWritable(bool isWritable);

    bool isGuestOk() const;
    void setIsGuestOk(bool isGuestOk);

    bool isValid();

private:
    QString m_shareName;
    QString m_path;
    QString m_comment;
    QString m_usershare_acl;
    QString m_guest_ok;

    bool m_isWritable = false;
    bool m_isGuestOk = false;
};

typedef QList<ShareInfo> ShareInfoList;

QDebug operator<<(QDebug dbg, const ShareInfo &obj);

#endif // SHAREINFO_H
