/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef BOXSELECTER_H
#define BOXSELECTER_H

#include "dfm_desktop_service_global.h"

#include <QRect>
#include <QPoint>
#include <QObject>

DSB_D_BEGIN_NAMESPACE

class BoxSelecter : public QObject
{
    Q_OBJECT
public:
    static BoxSelecter *instance();
    void beginSelect(const QPoint &globalPos, bool autoSelect);
    void endSelect();
    void setBegin(const QPoint &globalPos);
    void setEnd(const QPoint &globalPos);
    QRect validRect(QWidget *) const;
    QRect globalRect() const;
    QRect clipRect(QRect rect, const QRect &geometry) const;
    bool isBeginFrom(QWidget *w);
public:
    inline bool isAcvite() const {
        return active;
    }
protected:
    void setAcvite(bool ac);
    explicit BoxSelecter(QObject *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event);
signals:
    void changed();
public slots:
private:
    bool automatic = false;
    bool active = false;
    QPoint begin;
    QPoint end;
};

#define BoxSelIns DSB_D_NAMESPACE::BoxSelecter::instance()

DSB_D_END_NAMESPACE

#endif // BOXSELECTER_H
