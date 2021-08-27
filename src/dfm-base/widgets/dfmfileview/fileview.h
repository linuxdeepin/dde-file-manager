/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMFILEVIEW_H
#define DFMFILEVIEW_H

#include <DListView>

DWIDGET_USE_NAMESPACE

class DFMFileViewModel;
class DFMFileViewPrivate;
class DFMFileView : public DListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMFileView)
    QSharedPointer<DFMFileViewPrivate> d_ptr;

public:
    explicit DFMFileView(QWidget *parent = nullptr);

    virtual void setViewMode(QListView::ViewMode mode);

    virtual void setRootUrl(const QUrl &url);

    virtual QUrl rootUrl();

protected:
    void resizeEvent(QResizeEvent *event) override;

Q_SIGNALS:
    void urlChanged(const QUrl &old, const QUrl &now);
};

#endif // DFMFILEVIEW_H
