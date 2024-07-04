// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RENAMEBAR_H
#define RENAMEBAR_H

#include "dfmplugin_workspace_global.h"

#include <QFrame>

namespace dfmplugin_workspace {

class RenameBarPrivate;
class RenameBar : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(RenameBar)

public:
    explicit RenameBar(QWidget *parent = nullptr);

    void reset() noexcept;   //###: do not delete item! just clear content

    void storeUrlList(const QList<QUrl> &list) noexcept;

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

private:
    void initConnect();
    QList<QUrl> getSelectFiles();

private:
    QSharedPointer<RenameBarPrivate> d { nullptr };
    Q_DECLARE_PRIVATE_D(d, RenameBar)
};

}
#endif   // RENAMEBAR_H
