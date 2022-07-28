/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DETAILSPACEWIDGET_H
#define DETAILSPACEWIDGET_H

#include "dfmplugin_detailspace_global.h"
#include "dfm-base/interfaces/abstractframe.h"

#include <QUrl>

namespace dfmplugin_detailspace {

class DetailView;
class DetailSpaceWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(DetailSpaceWidget)

public:
    explicit DetailSpaceWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    void setCurrentUrl(const QUrl &url, int widgetFilter);
    QUrl currentUrl() const override;

    bool insterExpandControl(const int &index, QWidget *widget);

    void removeControls();

private:
    void initializeUi();
    void initConnect();

private:
    QUrl detailSpaceUrl;
    DetailView *detailView { nullptr };
};

}

#endif   // DETAILSPACEWIDGET_H
