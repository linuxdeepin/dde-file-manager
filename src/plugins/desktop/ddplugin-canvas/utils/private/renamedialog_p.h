/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef RENAMEDIALOGPRIVATE_H
#define RENAMEDIALOGPRIVATE_H

#include "ddplugin_canvas_global.h"
#include "utils/renamedialog.h"

#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QRegExpValidator>

namespace ddplugin_canvas {

class RenameDialogPrivate : public QObject
{
    Q_OBJECT
public:
    explicit RenameDialogPrivate(RenameDialog *qq = nullptr);
    ~RenameDialogPrivate() = default;

    void initUi();
    void initParameters();
    void initLayout();
    void initConnect();

private slots:
    void updateStates();
private:
    void setRenameButtonStatus(const bool& enabled);

public:
    QStackedLayout *stackedLayout = nullptr;

    QLabel *titleLabel = nullptr;
    std::tuple<QLabel *, QComboBox *, QHBoxLayout *> modeSelection;

    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> replaceForFinding;
    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> replaceForReplacing;
    QPair<QVBoxLayout *, QFrame *> replaceLayout;

    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> addForAdding;
    std::tuple<QLabel *, QComboBox *, QHBoxLayout *> addForLocating;
    QPair<QVBoxLayout *, QFrame *> addLayout;

    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> customForName;
    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> customForNumber;
    QPair<QVBoxLayout *, QFrame *> customLayout;

    QVBoxLayout *mainLayout = nullptr;
    QFrame *mainFrame = nullptr;

    QRegExpValidator *validator = nullptr;
    RenameDialog::ModifyMode mode = RenameDialog::kReplace;

private:
    RenameDialog *q;
};

}

#endif // RENAMEDIALOGPRIVATE_H
