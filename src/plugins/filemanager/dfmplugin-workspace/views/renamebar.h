/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#ifndef RENAMEBAR_H
#define RENAMEBAR_H

#include "dfmplugin_workspace_global.h"

#include <QFrame>

DPWORKSPACE_BEGIN_NAMESPACE

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

DPWORKSPACE_END_NAMESPACE
#endif   // RENAMEBAR_H
