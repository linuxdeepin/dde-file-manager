/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include "dfmplugin_computer_global.h"

#include "dfm-base/interfaces/abstractbaseview.h"
#include "dfm-base/base/application/application.h"

#include <QUrl>
#include <QScopedPointer>

#include <DListView>

namespace dfmbase {
class EntryFileInfo;
}

DFMBASE_USE_NAMESPACE
namespace dfmplugin_computer {

class ComputerModel;
class ComputerViewPrivate;
class ComputerView : public Dtk::Widget::DListView, public DFMBASE_NAMESPACE::AbstractBaseView
{
    Q_OBJECT
    Q_DISABLE_COPY(ComputerView)

public:
    ComputerView(const QUrl &url, QWidget *parent = nullptr);
    virtual ~ComputerView() override;

public:
    // AbstractBaseView interface
    virtual QWidget *widget() const override;
    virtual QUrl rootUrl() const override;
    virtual ViewState viewState() const override;
    virtual bool setRootUrl(const QUrl &url) override;
    virtual QList<QAction *> toolBarActionList() const override;
    virtual void refresh() override;
    virtual QList<QUrl> selectedUrlList() const override;

    // QObject interface
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    void hideSystemPartitions(bool hide);
    void hideLoopPartitions(bool hide);

protected:
    // QWidget interface
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
    ComputerModel *computerModel() const;

private:
    void initView();
    void initConnect();

    typedef QSharedPointer<EntryFileInfo> DFMEntryFileInfoPointer;
    void connectShortcut(QKeySequence seq, std::function<void(DFMEntryFileInfoPointer)> slot);

private Q_SLOTS:
    void cdTo(const QModelIndex &index);
    void onMenuRequest(const QPoint &pos);
    void onRenameRequest(quint64 winId, const QUrl &url);

Q_SIGNALS:
    void enterPressed(const QModelIndex &index);

private:
    QScopedPointer<ComputerViewPrivate> dp;
};

}
#endif   // COMPUTERVIEW_H
