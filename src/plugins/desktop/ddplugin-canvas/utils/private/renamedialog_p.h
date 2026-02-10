// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <QRegularExpressionValidator>

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
    void setRenameButtonStatus(const bool &enabled);

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

    QRegularExpressionValidator *validator = nullptr;
    RenameDialog::ModifyMode mode = RenameDialog::kReplace;

private:
    RenameDialog *q;
};

}

#endif   // RENAMEDIALOGPRIVATE_H
