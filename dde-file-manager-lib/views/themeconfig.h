/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef THEMECONFIG_H
#define THEMECONFIG_H

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class ThemeConfig
{
public:
    enum State {
        Normal = 0,
        Hover = 0x1,
        Pressed = 0x2,
        Checked = 0x10,
        Dislable = 0x20,
        Focus = 0x40,
        Inactive = 0x80
    };

    Q_DECLARE_FLAGS(States, State)

    static ThemeConfig *instace();

    bool hasValue(const QString &scope, const QString &key, States state = Normal) const;

    void setValue(const QString &scope, const QString &key, States state, const QVariant &value);

    QVariant value(const QString &scope, const QString &key, States state = Normal) const;
    QColor color(const QString &scope, const QString &key, States state = Normal) const;
    QString string(const QString &scope, const QString &key, States state = Normal) const;
    int integer(const QString &scope, const QString &key, States state = Normal) const;
    QPixmap pixmap(const QString &scope, const QString &key, States state = Normal, qreal scaleRatio = 0) const;

    void update(const QString &theme);

protected:
    ThemeConfig();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ThemeConfig::States)

DFM_END_NAMESPACE

#endif // THEMECONFIG_H
