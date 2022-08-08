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
#include "computerviewcontainer.h"
#include "computerview.h"

#include <QHBoxLayout>

using namespace dfmplugin_computer;

/*!
 * \class ComputerViewContainer
 * this class is designed for providing a 10px spacing
 * at the left of the ComputerView.
 */

ComputerViewContainer::ComputerViewContainer(const QUrl &url, QWidget *parent)
    : QWidget(parent)
{
    view = new ComputerView(url, parent);
    auto lay = new QHBoxLayout(this);
    lay->addWidget(view);
    lay->setContentsMargins(10, 0, 0, 0);
}

QWidget *ComputerViewContainer::widget() const
{
    return const_cast<ComputerViewContainer *>(this);
}

QUrl ComputerViewContainer::rootUrl() const
{
    return view->rootUrl();
}

dfmbase::AbstractBaseView::ViewState dfmplugin_computer::ComputerViewContainer::viewState() const
{
    return view->viewState();
}

bool ComputerViewContainer::setRootUrl(const QUrl &url)
{
    return view->setRootUrl(url);
}

QList<QUrl> ComputerViewContainer::selectedUrlList() const
{
    return view->selectedUrlList();
}
