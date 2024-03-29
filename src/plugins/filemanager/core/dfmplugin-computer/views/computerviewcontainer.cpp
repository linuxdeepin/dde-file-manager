// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computerviewcontainer.h"
#include "computerview.h"
#include "computerstatusbar.h"

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
    auto mainLay = new QVBoxLayout(this);
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, 0, 0, 0);

    view = new ComputerView(url, parent);
    auto lay = new QHBoxLayout();
    lay->addWidget(view);
    lay->setContentsMargins(11, 0, 0, 0);

    mainLay->addLayout(lay, 1);
    auto sb = new ComputerStatusBar(this);
    mainLay->addWidget(sb);
    view->setStatusBarHandler(sb);
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
