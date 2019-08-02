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

#ifndef QMOUNT_H
#define QMOUNT_H

#include <QString>
#include <QMap>
#include <QDebug>

class QMount
{
public:
    QMount();

    QString name() const;
    QString uuid() const;

    QString default_location() const;
    void setDefault_location(const QString &default_location);

    QStringList icons() const;

    QStringList symbolic_icons() const;
    void setSymbolic_icons(const QStringList &symbolic_icons);

    bool can_unmount() const;
    void setCan_unmount(bool can_unmount);

    bool can_eject() const;
    void setCan_eject(bool can_eject);

    bool is_shadowed() const;
    void setIs_shadowed(bool is_shadowed);

    void setName(const QString &name);

    void setIcons(const QStringList &icons);

    QString mounted_root_uri() const;
    void setMounted_root_uri(const QString &mounted_root_uri);

private:
    QString m_name;
    QString m_mounted_root_uri;
    QString m_uuid;
    QString m_default_location;
    QStringList m_icons;
    QStringList m_symbolic_icons;
    bool m_can_unmount = false;
    bool m_can_eject = false;
    bool m_is_shadowed = false;
};

QDebug operator<<(QDebug dbg, const QMount& mount);

#endif // QMOUNT_H
