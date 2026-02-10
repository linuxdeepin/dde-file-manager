// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RENAMEBAR_H
#define RENAMEBAR_H

#include "dfmplugin_workspace_global.h"

#include <QFrame>
#include <QScrollArea>

namespace dfmplugin_workspace {

class WorkspacePage;
class RenameBarPrivate;
class RenameBar : public QScrollArea
{
    Q_OBJECT
    Q_DISABLE_COPY(RenameBar)

public:
    explicit RenameBar(QWidget *parent = nullptr);

    void reset() noexcept;   //###: do not delete item! just clear content

    void storeUrlList(const QList<QUrl> &list) noexcept;

public Q_SLOTS:
    void setVisible(bool visible) override;

signals:
    void requestReplaceOperator();
    void clickCancelButton();
    void clickRenameButton();

    void visibleChanged(bool value);

private slots:
    void onVisibleChanged(bool value) noexcept;
    void onRenamePatternChanged(const int &index) noexcept;

    void onReplaceOperatorFileNameChanged(const QString &text) noexcept;
    void onReplaceOperatorDestNameChanged(const QString &textChanged) noexcept;
    void onAddOperatorAddedContentChanged(const QString &text) noexcept;
    void onAddTextPatternChanged(const int &index) noexcept;
    void onCustomOperatorFileNameChanged() noexcept;
    void onCustomOperatorSNNumberChanged();
    void eventDispatcher();
    void hideRenameBar();
    void onSelectUrlChanged(const QList<QUrl> &urls);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void initConnect();
    QList<QUrl> getSelectFiles();
    WorkspacePage *findPage();

private:
    QSharedPointer<RenameBarPrivate> d { nullptr };
    Q_DECLARE_PRIVATE_D(d, RenameBar)
};

}
#endif   // RENAMEBAR_H
