// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "renamebar.h"
#include "private/renamebar_p.h"
#include "utils/fileoperatorhelper.h"
#include <dfm-base/interfaces/abstractjobhandler.h>
#include "workspacewidget.h"
#include "views/fileview.h"

#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

RenameBar::RenameBar(QWidget *parent)
    : QFrame(parent), d(new RenameBarPrivate(this))
{
    setMinimumHeight(44);
    setMinimumWidth(900);
    initConnect();
}

void RenameBar::reset() noexcept
{
    ///replace
    QLineEdit *lineEdit { std::get<1>(d->replaceOperatorItems) };
    lineEdit->clear();
    lineEdit = std::get<3>(d->replaceOperatorItems);
    lineEdit->clear();

    ///add
    lineEdit = std::get<1>(d->addOperatorItems);
    lineEdit->clear();

    ///custom
    lineEdit = std::get<1>(d->customOPeratorItems);
    lineEdit->clear();
    lineEdit = std::get<3>(d->customOPeratorItems);
    lineEdit->setText(QString { "1" });

    d->flag = RenameBarPrivate::AddTextFlags::kBefore;
    d->currentPattern = RenameBarPrivate::RenamePattern::kReplace;
    d->renameButtonStates = std::array<bool, 3> { false, false, false };

    d->comboBox->setCurrentIndex(0);
    d->stackWidget->setCurrentIndex(0);
    std::get<3>(d->addOperatorItems)->setCurrentIndex(0);
}

void RenameBar::storeUrlList(const QList<QUrl> &list) noexcept
{
    Q_D(RenameBar);

    d->urlList = list;
}

void RenameBar::onVisibleChanged(bool value) noexcept
{
    Q_D(RenameBar);

    using RenamePattern = RenameBarPrivate::RenamePattern;
    if (value) {
        switch (d->currentPattern) {
        case RenamePattern::kReplace: {
            QLineEdit *lineEdit { std::get<1>(d->replaceOperatorItems) };
            lineEdit->setFocus();
            break;
        }
        case RenamePattern::kAdd: {
            QLineEdit *lineEdit { std::get<1>(d->addOperatorItems) };
            lineEdit->setFocus();
            break;
        }
        case RenamePattern::kCustom: {
            QLineEdit *lineEdit { std::get<1>(d->customOPeratorItems) };
            lineEdit->setFocus();
            break;
        }
        }
    } else {
        //还原焦点
        if (parentWidget()) {
            parentWidget()->setFocus();
        }
    }
}

void RenameBar::onRenamePatternChanged(const int &index) noexcept
{
    Q_D(RenameBar);

    d->currentPattern = static_cast<RenameBarPrivate::RenamePattern>(index);

    bool state { d->renameButtonStates[static_cast<std::size_t>(index)] };   //###: we get the value of state of button in current mode.
    d->stackWidget->setCurrentIndex(index);
    std::get<1>(d->buttonsArea)->setEnabled(state);

    ///###: here, call a slot, this function will set focus of QLineEdit in current mode.
    this->onVisibleChanged(true);
}

void RenameBar::onReplaceOperatorFileNameChanged(const QString &text) noexcept
{
    Q_D(RenameBar);

    QLineEdit *lineEdit { std::get<1>(d->replaceOperatorItems) };
    d->updateLineEditText(lineEdit);

    if (text.isEmpty()) {
        d->renameButtonStates[0] = false;   //###: record the states of rename button

        d->setRenameBtnStatus(false);
        return;
    }

    d->renameButtonStates[0] = true;   //###: record the states of rename button

    d->setRenameBtnStatus(true);
}

void RenameBar::onReplaceOperatorDestNameChanged(const QString &textChanged) noexcept
{
    Q_UNUSED(textChanged)
    Q_D(RenameBar);

    QLineEdit *lineEdit { std::get<3>(d->replaceOperatorItems) };
    d->updateLineEditText(lineEdit);
}

void RenameBar::onAddOperatorAddedContentChanged(const QString &text) noexcept
{
    Q_D(RenameBar);

    QLineEdit *lineEdit { std::get<1>(d->addOperatorItems) };
    d->updateLineEditText(lineEdit);

    if (text.isEmpty()) {
        d->renameButtonStates[1] = false;

        d->setRenameBtnStatus(false);
        return;
    }

    d->renameButtonStates[1] = true;

    d->setRenameBtnStatus(true);
}

void RenameBar::onAddTextPatternChanged(const int &index) noexcept
{
    Q_D(RenameBar);

    using AddTextFlags = RenameBarPrivate::AddTextFlags;

    d->flag = (index == 0 ? AddTextFlags::kBefore : AddTextFlags::kAfter);

    ///###: here, call a slot, this function will set focus of QLineEdit in current mode.
    this->onVisibleChanged(true);
}

void RenameBar::onCustomOperatorFileNameChanged() noexcept
{

    Q_D(RenameBar);

    QLineEdit *lineEditForFileName { std::get<1>(d->customOPeratorItems) };

    d->updateLineEditText(lineEditForFileName);

    if (lineEditForFileName->text().isEmpty()) {   //###: must be input filename.
        d->renameButtonStates[2] = false;
        d->setRenameBtnStatus(false);

    } else {

        QLineEdit *lineEditForSNNumber { std::get<3>(d->customOPeratorItems) };

        if (lineEditForSNNumber->text().isEmpty()) {
            d->renameButtonStates[2] = false;
            d->setRenameBtnStatus(false);

        } else {
            d->renameButtonStates[2] = true;
            d->setRenameBtnStatus(true);
        }
    }
}

void RenameBar::onCustomOperatorSNNumberChanged()
{
    Q_D(RenameBar);

    QLineEdit *lineEditForSNNumber { std::get<3>(d->customOPeratorItems) };
    if (lineEditForSNNumber->text().isEmpty()) {   //###: must be input filename.
        d->renameButtonStates[2] = false;
        d->setRenameBtnStatus(false);

    } else {

        QLineEdit *lineEditForFileName { std::get<3>(d->customOPeratorItems) };

        if (lineEditForFileName->text().isEmpty()) {
            d->renameButtonStates[2] = false;
            d->setRenameBtnStatus(false);

        } else {
            d->renameButtonStates[2] = true;
            d->setRenameBtnStatus(true);
        }

        ///###: renew from exception.
        std::string content { lineEditForSNNumber->text().toStdString() };
        try {
            Q_UNUSED(std::stoull(content));
        } catch (const std::out_of_range &err) {
            (void)err;
            lineEditForSNNumber->setText(QString { "1" });

        } catch (...) {
            lineEditForSNNumber->setText(QString { "1" });
        }
    }
}

void RenameBar::eventDispatcher()
{
    Q_D(RenameBar);

    QList<QUrl> urls = getSelectFiles();

    using RenamePattern = RenameBarPrivate::RenamePattern;

    if (d->currentPattern == RenamePattern::kReplace) {
        QString forFindingStr { std::get<1>(d->replaceOperatorItems)->text() };
        QString forReplaceStr { std::get<3>(d->replaceOperatorItems)->text() };
        QPair<QString, QString> pair { forFindingStr, forReplaceStr };

        FileOperatorHelperIns->renameFilesByReplace(parentWidget(), urls, pair);
    } else if (d->currentPattern == RenamePattern::kAdd) {
        QString forAddingStr { std::get<1>(d->addOperatorItems)->text() };
        QPair<QString, AbstractJobHandler::FileNameAddFlag> pair { forAddingStr, static_cast<AbstractJobHandler::FileNameAddFlag>(d->flag) };

        FileOperatorHelperIns->renameFilesByAdd(parentWidget(), urls, pair);
    } else if (d->currentPattern == RenamePattern::kCustom) {
        QString forCustomStr { std::get<1>(d->customOPeratorItems)->text() };
        QString numberStr { std::get<3>(d->customOPeratorItems)->text() };
        QPair<QString, QString> pair { forCustomStr, numberStr };

        FileOperatorHelperIns->renameFilesByCustom(parentWidget(), urls, pair);
    }

    setVisible(false);
    reset();

    if (parentWidget())
        parentWidget()->setFocus();
}

void RenameBar::hideRenameBar()
{
    setVisible(false);
    reset();
}

void RenameBar::initConnect()
{
    Q_D(RenameBar);

    using funcType = void (QComboBox::*)(int index);

    QObject::connect(d->comboBox, static_cast<funcType>(&QComboBox::activated), this, &RenameBar::onRenamePatternChanged);

    QObject::connect(std::get<0>(d->buttonsArea), &QPushButton::clicked, this, &RenameBar::clickCancelButton);
    QObject::connect(std::get<1>(d->replaceOperatorItems), &QLineEdit::textChanged, this, &RenameBar::onReplaceOperatorFileNameChanged);
    QObject::connect(std::get<3>(d->replaceOperatorItems), &QLineEdit::textChanged, this, &RenameBar::onReplaceOperatorDestNameChanged);
    QObject::connect(std::get<1>(d->addOperatorItems), &QLineEdit::textChanged, this, &RenameBar::onAddOperatorAddedContentChanged);

    QObject::connect(d->renameBtn, &DSuggestButton::clicked, this, &RenameBar::eventDispatcher);
    QObject::connect(std::get<3>(d->addOperatorItems), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &RenameBar::onAddTextPatternChanged);

    QObject::connect(std::get<1>(d->customOPeratorItems), &QLineEdit::textChanged, this, &RenameBar::onCustomOperatorFileNameChanged);
    QObject::connect(std::get<3>(d->customOPeratorItems), &QLineEdit::textChanged, this, &RenameBar::onCustomOperatorSNNumberChanged);

    QObject::connect(this, &RenameBar::visibleChanged, this, &RenameBar::onVisibleChanged);
    QObject::connect(this, &RenameBar::clickRenameButton, this, &RenameBar::eventDispatcher);
    QObject::connect(this, &RenameBar::clickCancelButton, this, &RenameBar::hideRenameBar);
}

QList<QUrl> RenameBar::getSelectFiles()
{
    auto widget = qobject_cast<WorkspaceWidget *>(parentWidget());
    if (widget) {
        auto view = dynamic_cast<FileView *>(widget->currentView());
        if (view)
            return view->selectedUrlList();
    }

    return {};
}
