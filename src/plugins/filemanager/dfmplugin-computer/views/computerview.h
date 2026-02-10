// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/base/application/application.h>

#include <QUrl>
#include <QScopedPointer>

#include <DListView>

namespace dfmbase {
class EntryFileInfo;
}

DFMBASE_USE_NAMESPACE
namespace dfmplugin_computer {

class ComputerStatusBar;
class ComputerModel;
class ComputerViewPrivate;
class ComputerView : public Dtk::Widget::DListView, public DFMBASE_NAMESPACE::AbstractBaseView
{
    Q_OBJECT
    Q_DISABLE_COPY(ComputerView)

public:
    explicit ComputerView(const QUrl &url, QWidget *parent = nullptr);
    virtual ~ComputerView() override;

public:
    // AbstractBaseView interface
    virtual QWidget *widget() const override;
    virtual QUrl rootUrl() const override;
    virtual ViewState viewState() const override;
    virtual bool setRootUrl(const QUrl &url) override;
    virtual QList<QUrl> selectedUrlList() const override;

    // QObject interface
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    void setStatusBarHandler(ComputerStatusBar *sb);
    void setRowHidden(int row, bool hide);

public Q_SLOTS:
    void handleComputerItemVisible();

protected:
    // QWidget interface
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
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
    void onUpdateItemAlias(const QUrl &url, const QString &alias, bool isProtocol);
    void handleDisksVisible();
    void handleUserDirVisible();
    void handle3rdEntriesVisible();
    void handleDiskSplitterVisible();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

Q_SIGNALS:
    void enterPressed(const QModelIndex &index);

private:
    QScopedPointer<ComputerViewPrivate> dp;
};

}
#endif   // COMPUTERVIEW_H
