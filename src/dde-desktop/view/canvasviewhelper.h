/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

#pragma once

#include <dfmevent.h>
#include <dfileviewhelper.h>

class DFMEvent;
class CanvasGridView;
class CanvasViewHelper : public DFileViewHelper
{
    Q_OBJECT
public:
    explicit CanvasViewHelper(CanvasGridView *parent);

    CanvasGridView *parent() const;

    virtual quint64 windowId() const override;
    virtual const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const override;
    virtual DFMStyledItemDelegate *itemDelegate() const override;
    virtual DFileSystemModel *model() const override;
    virtual const DUrlList selectedUrls() const override;
    virtual void select(const QList<DUrl> &list) override;
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    virtual int selectedIndexsCount() const override;
    virtual bool isSelected(const QModelIndex &index) const override;
    virtual void viewFlicker() override;
    bool isPaintFile() const;

public slots:
    void edit(const DFMEvent &event);
    void onRequestSelectFiles(const QList<DUrl> &urls);
    void handleSelectEvent(const DFMUrlListBaseEvent &event);
private:
    bool m_paintFile = true;
};
