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
#ifndef COMPUTERVIEWCONTAINER_H
#define COMPUTERVIEWCONTAINER_H

#include "dfmplugin_computer_global.h"
#include "dfm-base/interfaces/abstractbaseview.h"

#include <QWidget>

namespace dfmplugin_computer {
class ComputerView;
class ComputerViewContainer : public QWidget, public DFMBASE_NAMESPACE::AbstractBaseView
{
public:
    explicit ComputerViewContainer(const QUrl &url, QWidget *parent = nullptr);

    virtual QWidget *widget() const override;
    virtual QUrl rootUrl() const override;
    virtual ViewState viewState() const override;
    virtual bool setRootUrl(const QUrl &url) override;
    virtual QList<QUrl> selectedUrlList() const override;

private:
    ComputerView *view { nullptr };
};
}

#endif   // COMPUTERVIEWCONTAINER_H
