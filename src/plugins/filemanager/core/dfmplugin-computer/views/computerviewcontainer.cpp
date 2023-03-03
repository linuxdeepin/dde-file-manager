// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
