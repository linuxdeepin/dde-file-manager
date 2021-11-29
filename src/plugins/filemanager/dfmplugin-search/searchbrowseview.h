/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHBROWSEVIEW_H
#define SEARCHBROWSEVIEW_H

#include "window/browseview.h"   // TODO(zhangs): hide
#include "searchfileviewmodel.h"

DSB_FM_USE_NAMESPACE

class SearchBrowseView : public BrowseView
{
public:
    explicit SearchBrowseView(QWidget *parent = nullptr);
    virtual void setRootUrl(const QUrl &url) override;
    virtual QUrl rootUrl() override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QUrl m_rootUrl;
};

#endif   // SEARCHBROWSEVIEW_H
