// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
