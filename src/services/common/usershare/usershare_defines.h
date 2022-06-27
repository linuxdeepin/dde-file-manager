/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef NETANDSHARE_DEFINES_H
#define NETANDSHARE_DEFINES_H

#include "dfm_common_service_global.h"

#include <QString>

#include <functional>

namespace dfm_service_common {

namespace EventType {
extern const int kRemoveShare;
}

class ShareInfo
{
public:
    explicit ShareInfo(const QString &name = {}, const QString &path = {}, const QString &comment = {}, bool writable = false, bool anonymous = false);

    bool isValid() const;

    inline QString getShareName() const { return shareName; }
    void setShareName(const QString &value);

    inline QString getPath() const { return path; }
    void setPath(const QString &value);

    inline QString getComment() const { return comment; }
    void setComment(const QString &value);

    inline QString getUserShareAcl() const { return userShareAcl; }
    void setUserShareAcl(const QString &value);

    inline QString getGuestEnable() const { return guestEnable; }
    void setGuestEnable(const QString &value);

    inline bool getWritable() const { return writable; }
    void setWritable(bool value);

    inline bool getAnonymous() const { return anonymous; }
    void setAnonymous(bool value);

private:
    QString shareName;
    QString path;
    QString comment;
    QString userShareAcl;
    QString guestEnable;

    bool writable = false;
    bool anonymous = false;
};

QDebug operator<<(QDebug dbg, const ShareInfo &obj);

using StartSambaFinished = std::function<void(bool, const QString &)>;

}

typedef QList<DSC_NAMESPACE::ShareInfo> ShareInfoList;

#endif   // NETANDSHARE_DEFINES_H
