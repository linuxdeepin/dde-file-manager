/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef COLLECTIONTITLEBAR_H
#define COLLECTIONTITLEBAR_H

#include "ddplugin_organizer_global.h"

#include <DBlurEffectWidget>

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionTitleBarPrivate;

class CollectionTitleBar : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit CollectionTitleBar(QWidget *parent = nullptr);
    ~CollectionTitleBar() override;

    void setRenamable(const bool renamable = true);
    bool renamable() const;
    void setTitleName(const QString &name);
    QString titleName() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QSharedPointer<CollectionTitleBarPrivate> d = nullptr;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONTITLEBAR_H
