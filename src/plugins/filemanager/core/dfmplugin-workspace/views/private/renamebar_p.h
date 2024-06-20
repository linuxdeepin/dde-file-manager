// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RENAMEBAR_P_H
#define RENAMEBAR_P_H

#include "dfmplugin_workspace_global.h"

#include <QPair>
#include <QUrl>
#include <DSuggestButton>
#include <tuple>

#include <QRegularExpressionValidator>

QT_BEGIN_NAMESPACE
class QLabel;
class QHBoxLayout;
class QComboBox;
class QStackedWidget;
class QLineEdit;
class QFrame;
class QRegExpValidator;
class QPushButton;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

namespace dfmplugin_workspace {
class RenameBar;

class RenameBarPrivate : public QObject
{
    template<typename... Types>
    using DTuple = std::tuple<Types...>;

    template<std::size_t size>
    using DArray = std::array<bool, size>;

public:
    enum class RenamePattern : uint8_t {
        kReplace = 0,
        kAdd,
        kCustom
    };

    enum class AddTextFlags : uint8_t {
        kBefore = 0,
        kAfter = 1
    };

public:
    explicit RenameBarPrivate(RenameBar *const qPtr);
    ~RenameBarPrivate() = default;

    void initUI();
    void setUIParameters();
    void layoutItems() noexcept;
    void setRenameBtnStatus(const bool &value) noexcept;

    QString filteringText(const QString &text);
    void updateLineEditText(QLineEdit *lineEdit, const QString &defaultValue = "");

    RenameBar *q_ptr { nullptr };

    QHBoxLayout *mainLayout { nullptr };
    QComboBox *comboBox { nullptr };
    QStackedWidget *stackWidget { nullptr };
    DArray<3> renameButtonStates { { false } };   //###: this is a array for recording the state of rename button in current pattern.
    RenamePattern currentPattern { RenamePattern::kReplace };   //###: this number record current pattern.
    QList<QUrl> urlList {};   //###: this list stores the url of file which is waiting to be modified!
    AddTextFlags flag { AddTextFlags::kBefore };   //###: the loacation of text should be added.

    DTuple<QLabel *, QLineEdit *, QLabel *, QLineEdit *> replaceOperatorItems {};
    QPair<QHBoxLayout *, QFrame *> frameForLayoutReplaceArea {};

    DTuple<QLabel *, QLineEdit *, QLabel *, QComboBox *> addOperatorItems {};
    QPair<QHBoxLayout *, QFrame *> frameForLayoutAddArea {};

    DTuple<QLabel *, QLineEdit *, QLabel *, QLineEdit *, QLabel *> customOPeratorItems {};
    QPair<QHBoxLayout *, QFrame *> frameForLayoutCustomArea {};
    QRegularExpressionValidator *validator { nullptr };

    DTuple<QPushButton *, QPushButton *, QHBoxLayout *, QFrame *> buttonsArea {};

    DSuggestButton *renameBtn { nullptr };

public slots:
    void onRenamePatternChanged(const int &index) noexcept;
};

}

#endif   // RENAMEBAR_P_H
