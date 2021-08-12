/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "drenamebar.h"
#include "windowmanager.h"
#include "controllers/appcontroller.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfileservices.h"
#include "accessibility/ac-lib-file-manager.h"

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QValidator>
#include <QDebug>
#include <QLabel>
#include <QFrame>
#include <QList>
#include <QPair>
#include <tuple>
#include <array>
#include <QList>
#include <QObject>


using namespace Dtk::Widget;


template<typename Type, typename ...Types>
RecordRenameBarState::RecordRenameBarState(Type &&arg, Types &&... args)
{
    static_assert(sizeof...(args) == 6, "Must be 6 parameters!");
    auto tuple = std::make_tuple(std::forward<Types>(args)...);

    m_patternFirst = std::forward<Type>(arg);

    m_patternSecond.first = std::get<0>(tuple).first;
    m_patternSecond.second = std::get<0>(tuple).second;


    m_patternThird.first = std::get<1>(tuple).first;
    m_patternThird.second = std::get<1>(tuple).second;

    m_buttonStateInThreePattern = std::get<2>(tuple);
    m_currentPattern = std::get<3>(tuple);

    m_selectedUrls = std::get<4>(tuple);
    m_visible.store(std::get<5>(tuple), std::memory_order_seq_cst);
}


RecordRenameBarState::RecordRenameBarState(const RecordRenameBarState &other)
    : m_patternFirst{ other.m_patternFirst },
      m_patternSecond{ other.m_patternSecond },
      m_patternThird{ other.m_patternThird },
      m_buttonStateInThreePattern{ other.m_buttonStateInThreePattern },
      m_currentPattern{ other.m_currentPattern },
      m_selectedUrls{ other.m_selectedUrls }
{
    m_visible.store(other.m_visible.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
}


RecordRenameBarState::RecordRenameBarState(RecordRenameBarState &&other)
    : m_patternFirst{ std::move(other.m_patternFirst) },
      m_patternSecond{ std::move(other.m_patternSecond) },
      m_patternThird{ std::move(other.m_patternThird) },
      m_buttonStateInThreePattern{ std::move(other.m_buttonStateInThreePattern) },
      m_currentPattern{ std::move(other.m_currentPattern) },
      m_selectedUrls{ std::move(other.m_selectedUrls) }
{
    m_visible.store(other.m_visible.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
}



RecordRenameBarState &RecordRenameBarState::operator=(const RecordRenameBarState &other)
{
    m_patternFirst = other.m_patternFirst;
    m_patternSecond = other.m_patternSecond;
    m_patternThird = other.m_patternThird;
    m_buttonStateInThreePattern = other.m_buttonStateInThreePattern;
    m_currentPattern = other.m_currentPattern;
    m_selectedUrls = other.m_selectedUrls;
    m_visible.store(other.m_visible.load(std::memory_order_seq_cst), std::memory_order_seq_cst);

    return *this;
}




RecordRenameBarState &RecordRenameBarState::operator=(RecordRenameBarState &&other)
{
    m_patternFirst = std::move(other.m_patternFirst);
    m_patternSecond = std::move(other.m_patternSecond);
    m_patternThird = std::move(other.m_patternThird);
    m_buttonStateInThreePattern = std::move(other.m_buttonStateInThreePattern);
    m_currentPattern = std::move(other.m_currentPattern);
    m_selectedUrls = std::move(other.m_selectedUrls);
    m_visible.store(other.m_visible.load(std::memory_order_seq_cst), std::memory_order_seq_cst);

    return *this;
}

void RecordRenameBarState::clear()
{
    m_patternFirst.first.clear();
    m_patternFirst.second.clear();
    m_patternSecond.first.clear();
    m_patternSecond.second = DFileService::AddTextFlags::Before;
    m_patternThird.first.clear();
    m_patternThird.second.clear();
    m_buttonStateInThreePattern = std::array<bool, 3> {false, false, false};
    m_currentPattern = 0;
    m_selectedUrls.clear();
    m_visible.store(false, std::memory_order_seq_cst);
}



class DRenameBarPrivate
{
private:
    template<typename... Types>
    using QTuple = std::tuple<Types...>;

    template<std::size_t size>
    using QArray = std::array<bool, size>;


public:
    explicit DRenameBarPrivate(DRenameBar *const qPtr);
    ~DRenameBarPrivate() = default;

    ///###: in fact, you can not also std::move too.
    DRenameBarPrivate(const DRenameBarPrivate &) = delete;
    DRenameBarPrivate &operator=(DRenameBarPrivate &&) = delete;


    void initUi();
    void setUiParameters();
    void layoutItems()noexcept;
    void setRenameBtnStatus(const bool &value)noexcept;

    QString filteringText(const QString &text); //bug 26533
    void updateLineEditText(QLineEdit *lineEdit, const QString &defaultValue = "");//bug 26533


    DRenameBar *q_ptr{ nullptr };
    QHBoxLayout *m_mainLayout{ nullptr };
    QComboBox *m_comboBox{ nullptr };
    QStackedWidget *m_stackWidget{ nullptr };
    QArray<3> m_renameButtonStates{{false}};  //###: this is a array for recording the state of rename button in current pattern.
    std::size_t m_currentPattern{0};               //###: this number record current pattern.
    QList<DUrl> m_urlList{};                      //###: this list stores the url of file which is waiting to be modified!
    DFileService::AddTextFlags m_flag{ DFileService::AddTextFlags::Before }; //###: the loacation of text should be added.

    QTuple<QLabel *, QLineEdit *, QLabel *, QLineEdit *> m_replaceOperatorItems{};
    QPair<QHBoxLayout *, QFrame *> m_frameForLayoutReplaceArea{};

    QTuple<QLabel *, QLineEdit *, QLabel *, QComboBox *> m_addOperatorItems{};
    QPair<QHBoxLayout *, QFrame *> m_frameForLayoutAddArea{};


    QTuple<QLabel *, QLineEdit *, QLabel *, QLineEdit *, QLabel *> m_customOPeratorItems{};
    QPair<QHBoxLayout *, QFrame *> m_frameForLayoutCustomArea{};
    QRegExpValidator *m_validator{ nullptr };


    QTuple<QPushButton *, QPushButton *, QHBoxLayout *, QFrame *> m_buttonsArea{};



public slots:
    void onRenamePatternChanged(const int &index)noexcept;
};


DRenameBarPrivate::DRenameBarPrivate(DRenameBar *const qPtr)
    : q_ptr{ qPtr }
{
    this->initUi();
    this->setUiParameters();
    this->layoutItems();
}

void DRenameBarPrivate::initUi()
{
    m_mainLayout = new QHBoxLayout(q_ptr);
    m_comboBox = new QComboBox;
    m_stackWidget = new QStackedWidget;

    AC_SET_OBJECT_NAME(m_comboBox, AC_COMPUTER_RENAME_BAR_SELECT_TYPE);
    AC_SET_ACCESSIBLE_NAME(m_comboBox, AC_COMPUTER_RENAME_BAR_SELECT_TYPE);
    AC_SET_OBJECT_NAME(m_stackWidget, AC_COMPUTER_RENAME_BAR_STACK_WIDGET);
    AC_SET_ACCESSIBLE_NAME(m_stackWidget, AC_COMPUTER_RENAME_BAR_STACK_WIDGET);

    m_replaceOperatorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QLineEdit);
    m_frameForLayoutReplaceArea = QPair<QHBoxLayout *, QFrame *> { new QHBoxLayout, new QFrame };

    m_addOperatorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QComboBox);
    m_frameForLayoutAddArea = QPair<QHBoxLayout *, QFrame *> { new QHBoxLayout, new QFrame };

    m_customOPeratorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QLineEdit, new QLabel);
    m_frameForLayoutCustomArea = QPair<QHBoxLayout *, QFrame *> { new QHBoxLayout, new QFrame };

    m_buttonsArea = std::make_tuple(new QPushButton, new QPushButton, new QHBoxLayout, new QFrame);
}


void DRenameBarPrivate::setUiParameters()
{
    m_comboBox->addItems(QList<QString> { QObject::tr("Replace Text"), QObject::tr("Add Text"), QObject::tr("Custom Text") });

    QComboBox *comboBox{ nullptr };
    QLabel *label{ std::get<0>(m_replaceOperatorItems) };
    QLineEdit *lineEdit{ std::get<1>(m_replaceOperatorItems) };

    label->setText(QObject::tr("Find"));
    label->setObjectName(QString{"DRenameBarLabel"});
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);

    label = std::get<2>(m_replaceOperatorItems);
    label->setObjectName(QString{"DRenameBarLabel"});
    lineEdit = std::get<3>(m_replaceOperatorItems);
    label->setText(QObject::tr("Replace"));
    lineEdit->setPlaceholderText(QObject::tr("Optional"));
    label->setBuddy(lineEdit);

    label = std::get<0>(m_addOperatorItems);
    label->setObjectName(QString{"DRenameBarLabel"});
    lineEdit = std::get<1>(m_addOperatorItems);
    label->setText(QObject::tr("Add"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);

    label = std::get<2>(m_addOperatorItems);
    label->setObjectName(QString{"DRenameBarLabel"});
    comboBox = std::get<3>(m_addOperatorItems);
    label->setText(QObject::tr("Location"));
    comboBox->addItems(QList<QString> { QObject::tr("Before file name"), QObject::tr("After file name") });
    label->setBuddy(comboBox);

    label = std::get<0>(m_customOPeratorItems);
    label->setObjectName(QString{"DRenameBarLabel"});
    lineEdit = std::get<1>(m_customOPeratorItems);
    label->setText(QObject::tr("File name"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);

    label = std::get<2>(m_customOPeratorItems);
    label->setObjectName(QString{"DRenameBarLabel"});
    lineEdit = std::get<3>(m_customOPeratorItems);
    label->setText(QObject::tr("Start at"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    lineEdit->setText(QString{"1"});

    QRegExp regStr{ QString{"[0-9]+"} };
    m_validator = new QRegExpValidator{ regStr, lineEdit };

    lineEdit->setValidator(m_validator);
    label->setBuddy(lineEdit);
    label = std::get<4>(m_customOPeratorItems);
    label->setObjectName(QString{"DRenameBarLabel"});
    label->setText(QObject::tr("Tips: Sort by selected file order"));

    QPushButton *button{ std::get<0>(m_buttonsArea) };
    button->setText(QObject::tr("Cancel","button"));
    button = std::get<1>(m_buttonsArea);
    button->setText(QObject::tr("Rename","button"));

    button->setEnabled(false);
}

void DRenameBarPrivate::layoutItems()noexcept
{
    QHBoxLayout *hBoxLayout{ nullptr };
    QFrame *frame{ nullptr };

    hBoxLayout = m_frameForLayoutReplaceArea.first;
    frame = m_frameForLayoutReplaceArea.second;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    hBoxLayout->addWidget(std::get<0>(m_replaceOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(m_replaceOperatorItems));
    hBoxLayout->addSpacing(32);
    hBoxLayout->addWidget(std::get<2>(m_replaceOperatorItems));
    hBoxLayout->setSpacing(13);
    hBoxLayout->addWidget(std::get<3>(m_replaceOperatorItems));
    hBoxLayout->addStretch(0);
    frame->setLayout(hBoxLayout);
    m_stackWidget->addWidget(frame);


    hBoxLayout = m_frameForLayoutAddArea.first;
    frame = m_frameForLayoutAddArea.second;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    hBoxLayout->addWidget(std::get<0>(m_addOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(m_addOperatorItems));
    hBoxLayout->addSpacing(32);
    hBoxLayout->addWidget(std::get<2>(m_addOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<3>(m_addOperatorItems));
    hBoxLayout->addStretch();
    frame->setLayout(hBoxLayout);
    m_stackWidget->addWidget(frame);


    hBoxLayout = m_frameForLayoutCustomArea.first;
    frame = m_frameForLayoutCustomArea.second;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    hBoxLayout->addWidget(std::get<0>(m_customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(m_customOPeratorItems));
    hBoxLayout->addSpacing(32);
    hBoxLayout->addWidget(std::get<2>(m_customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<3>(m_customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<4>(m_customOPeratorItems));
    hBoxLayout->addStretch(0);
    frame->setLayout(hBoxLayout);
    m_stackWidget->addWidget(frame);


    hBoxLayout = std::get<2>(m_buttonsArea);
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    frame = std::get<3>(m_buttonsArea); //###: !!!
    hBoxLayout->addWidget(std::get<0>(m_buttonsArea));
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(std::get<1>(m_buttonsArea));
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hBoxLayout);


    m_mainLayout->addWidget(m_comboBox);
    m_mainLayout->addSpacing(32);
    m_mainLayout->addWidget(m_stackWidget);
    m_mainLayout->addStretch(0);
    m_mainLayout->addWidget(frame);//###: !!!
//    m_mainLayout->setContentsMargins(0,0,0,0); //do not open this.
//    m_mainLayout->setSpacing(0);
    m_stackWidget->setCurrentIndex(0);

    q_ptr->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    q_ptr->setLayout(m_mainLayout);
}


void DRenameBarPrivate::setRenameBtnStatus(const bool &value)noexcept
{
    QPushButton *button{ std::get<1>(m_buttonsArea) };

    button->setEnabled(value);
}

QString DRenameBarPrivate::filteringText(const QString &text)
{
    if (text.isEmpty())
        return text;

    QString readyText = text;
    return readyText.remove(QRegExp("[\\\\/:\\*\\?\"<>|%&]"));
}

void DRenameBarPrivate::updateLineEditText(QLineEdit *lineEdit, const QString &defaultValue)
{
    QString olderText = lineEdit->text();
    QString text = filteringText(olderText);
    if (olderText != text) {
        lineEdit->setText(text);
    }
    if (text.isEmpty()) {
        lineEdit->setText(defaultValue);
    }
}

DRenameBar::DRenameBar(QWidget *parent)
    : QFrame{parent},
      d_ptr{ new DRenameBarPrivate{this} }
{
    this->initConnect();
    this->hide();
    this->setObjectName(QString{"DRenameBar"});
    AC_SET_ACCESSIBLE_NAME(this, AC_COMPUTER_RENAME_BAR);
}


void DRenameBar::onRenamePatternChanged(const int &index)noexcept
{
    DRenameBarPrivate *const d{ d_func() };

    d->m_currentPattern = static_cast<std::size_t>(index);

    bool state{ d->m_renameButtonStates[static_cast<std::size_t>(index)] }; //###: we get the value of state of button in current mode.
    d->m_stackWidget->setCurrentIndex(index);
    std::get<1>(d->m_buttonsArea)->setEnabled(state);

    ///###: here, call a slot, this function will set focus of QLineEdit in current mode.
    this->onVisibleChanged(true);
}


void DRenameBar::onReplaceOperatorFileNameChanged(const QString &text)noexcept
{
    DRenameBarPrivate *const d{ d_func() };

    QLineEdit *lineEdit{ std::get<1>(d->m_replaceOperatorItems) };
    d->updateLineEditText(lineEdit);

    if (text.isEmpty() == true) {
        d->m_renameButtonStates[0] = false; //###: record the states of rename button

        d->setRenameBtnStatus(false);
        return;
    }

    d->m_renameButtonStates[0] = true;   //###: record the states of rename button

    d->setRenameBtnStatus(true);
    return;
}

void DRenameBar::onReplaceOperatorDestNameChanged(const QString &textChanged)noexcept
{
    Q_UNUSED(textChanged);
    DRenameBarPrivate *const d{ d_func() };

    QLineEdit *lineEdit{ std::get<3>(d->m_replaceOperatorItems) };
    d->updateLineEditText(lineEdit);
}

void DRenameBar::onAddOperatorAddedContentChanged(const QString &text)noexcept
{
    DRenameBarPrivate *const d{ d_func() };

    QLineEdit *lineEdit{ std::get<1>(d->m_addOperatorItems)};
    d->updateLineEditText(lineEdit);

    if (text.isEmpty() == true) {
        d->m_renameButtonStates[1] = false;

        d->setRenameBtnStatus(false);
        return;
    }

    d->m_renameButtonStates[1] = true;

    d->setRenameBtnStatus(true);
    return;
}


void DRenameBar::eventDispatcher()
{
    DRenameBarPrivate *const d { d_func() };

    bool value{ false };

    if (d->m_currentPattern == 0) { //###: replace
        QString forFindingStr{ std::get<1>(d->m_replaceOperatorItems)->text() };
        QString forReplaceStr{ std::get<3>(d->m_replaceOperatorItems)->text() };

        QPair<QString, QString> pair{forFindingStr, forReplaceStr};

        value = DFileService::instance()->multiFilesReplaceName(d->m_urlList, pair);

    } else if (d->m_currentPattern == 1) { //###: add
        QString forAddingStr{ std::get<1>(d->m_addOperatorItems)->text() };

        QPair<QString, DFileService::AddTextFlags> pair{ forAddingStr, d->m_flag };

        value = DFileService::instance()->multiFilesAddStrToName(d->m_urlList, pair);

    } else if (d->m_currentPattern == 2) { //###:  custom
        QString forCustomStr{ std::get<1>(d->m_customOPeratorItems)->text() };
        QString numberStr{ std::get<3>(d->m_customOPeratorItems)->text() };

        QPair<QString, QString> pair{ forCustomStr, numberStr};

        value = DFileService::instance()->multiFilesCustomName(d->m_urlList, pair);
    }

    if (value == true) {
        if (QWidget *const parent = dynamic_cast<QWidget *>(this->parent())) {
            quint64 windowId{ WindowManager::getWindowId(parent) };
            AppController::multiSelectionFilesCache.second = windowId;
        }
    }

    this->setVisible(false);
    this->resetRenameBar();
}

void DRenameBar::onAddTextPatternChanged(const int &index)noexcept
{
    DRenameBarPrivate *const d{ d_func() };

    if (index == 0) { //###: add text to the front of filename.
        d->m_flag = DFileService::AddTextFlags::Before;
    } else {       //###: add text to the back of filename.
        d->m_flag = DFileService::AddTextFlags::After;
    }

    ///###: here, call a slot, this function will set focus of QLineEdit in current mode.
    this->onVisibleChanged(true);
}


void DRenameBar::onCustomOperatorFileNameChanged()noexcept
{
    DRenameBarPrivate *const d{ d_func() };

    QLineEdit *lineEditForFileName{ std::get<1>(d->m_customOPeratorItems) };

    d->updateLineEditText(lineEditForFileName);

    if (lineEditForFileName->text().isEmpty() == true) { //###: must be input filename.
        d->m_renameButtonStates[2] = false;
        d->setRenameBtnStatus(false);

    } else {

        QLineEdit *lineEditForSNNumber{ std::get<3>(d->m_customOPeratorItems) };

        if (lineEditForSNNumber->text().isEmpty() == true) {
            d->m_renameButtonStates[2] = false;
            d->setRenameBtnStatus(false);

        } else {
            d->m_renameButtonStates[2] = true;
            d->setRenameBtnStatus(true);

        }

    }
}

void DRenameBar::onCustomOperatorSNNumberChanged()
{
    DRenameBarPrivate *const d{ d_func() };

    QLineEdit *lineEditForSNNumber{ std::get<3>(d->m_customOPeratorItems) };
    if (lineEditForSNNumber->text().isEmpty() == true) { //###: must be input filename.
        d->m_renameButtonStates[2] = false;
        d->setRenameBtnStatus(false);

    } else {

        QLineEdit *lineEditForFileName{ std::get<3>(d->m_customOPeratorItems) };

        if (lineEditForFileName->text().isEmpty() == true) {
            d->m_renameButtonStates[2] = false;
            d->setRenameBtnStatus(false);

        } else {
            d->m_renameButtonStates[2] = true;
            d->setRenameBtnStatus(true);

        }

        ///###: renew from exception.
        std::string content{ lineEditForSNNumber->text().toStdString() };
        try {
            Q_UNUSED(std::stoull(content));
        } catch (const std::out_of_range &err) {
            (void)err;
            lineEditForSNNumber->setText(QString{"1"});

        } catch (...) {
            lineEditForSNNumber->setText(QString{"1"});
        }

    }


}



void DRenameBar::storeUrlList(const QList<DUrl> &list)noexcept
{
    DRenameBarPrivate *const d{ d_func() };
    d->m_urlList = list;
}

void DRenameBar::initConnect()
{
    DRenameBarPrivate *const d{ d_func() };
    using funcType = void (QComboBox::*)(int index);


    QObject::connect(d->m_comboBox, static_cast<funcType>(&QComboBox::activated), this, &DRenameBar::onRenamePatternChanged);

    QObject::connect(std::get<0>(d->m_buttonsArea), &QPushButton::clicked, this, &DRenameBar::clickCancelButton);
    QObject::connect(std::get<1>(d->m_replaceOperatorItems), &QLineEdit::textChanged, this, &DRenameBar::onReplaceOperatorFileNameChanged);
    QObject::connect(std::get<3>(d->m_replaceOperatorItems), &QLineEdit::textChanged, this, &DRenameBar::onReplaceOperatorDestNameChanged);
    QObject::connect(std::get<1>(d->m_addOperatorItems), &QLineEdit::textChanged, this, &DRenameBar::onAddOperatorAddedContentChanged);

    QObject::connect(std::get<1>(d->m_buttonsArea), &QPushButton::clicked, this, &DRenameBar::eventDispatcher);
    QObject::connect(std::get<3>(d->m_addOperatorItems), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &DRenameBar::onAddTextPatternChanged);


    QObject::connect(std::get<1>(d->m_customOPeratorItems), &QLineEdit::textChanged, this, &DRenameBar::onCustomOperatorFileNameChanged);
    QObject::connect(std::get<3>(d->m_customOPeratorItems), &QLineEdit::textChanged, this, &DRenameBar::onCustomOperatorSNNumberChanged);

    QObject::connect(this, &DRenameBar::visibleChanged, this, &DRenameBar::onVisibleChanged);
    QObject::connect(this, &DRenameBar::clickRenameButton, this, &DRenameBar::eventDispatcher);
}


void DRenameBar::resetRenameBar()noexcept
{
    DRenameBarPrivate *const d{ d_func() };

    ///replace
    QLineEdit *lineEdit{ std::get<1>(d->m_replaceOperatorItems) };
    lineEdit->clear();
    lineEdit = std::get<3>(d->m_replaceOperatorItems);
    lineEdit->clear();

    ///add
    lineEdit = std::get<1>(d->m_addOperatorItems);
    lineEdit->clear();

    ///custom
    lineEdit = std::get<1>(d->m_customOPeratorItems);
    lineEdit->clear();
    lineEdit = std::get<3>(d->m_customOPeratorItems);
    lineEdit->setText(QString{"1"});

    d->m_flag = DFileService::AddTextFlags::Before;
    d->m_currentPattern = 0;
    d->m_renameButtonStates = std::array<bool, 3> {false, false, false};

    d->m_comboBox->setCurrentIndex(0);
    d->m_stackWidget->setCurrentIndex(0);
    std::get<3>(d->m_addOperatorItems)->setCurrentIndex(0);
}

std::unique_ptr<RecordRenameBarState> DRenameBar::getCurrentState()const
{
    const DRenameBarPrivate *const d{ d_func() };
    QPair<QString, QString> patternOneContent{ std::get<1>(d->m_replaceOperatorItems)->text(),
                                               std::get<3>(d->m_replaceOperatorItems)->text()
                                             };
    QPair<QString, DFileService::AddTextFlags> patternTwoContent{ std::get<1>(d->m_addOperatorItems)->text(),
                                                                  d->m_flag };
    QPair<QString, QString> patternThreeContent{ std::get<1>(d->m_customOPeratorItems)->text(),
                                                 std::get<3>(d->m_customOPeratorItems)->text()
                                               };

    std::array<bool, 3> buttonsState{ d->m_renameButtonStates };
    std::size_t currentPattern{ d->m_currentPattern };
    QList<DUrl> selectedUrls{ d->m_urlList };

    bool visibleValue{ this->isVisible() };

    std::unique_ptr<RecordRenameBarState> state{
        new RecordRenameBarState{
            patternOneContent, patternTwoContent, patternThreeContent,
            buttonsState, currentPattern, selectedUrls, visibleValue }
    };
    return state;
}

void DRenameBar::loadState(std::unique_ptr<RecordRenameBarState> &state)
{
    DRenameBarPrivate *const d{ d_func() };

    if (static_cast<bool>(state) == true) {

        QPair<QString, QString> patternOneContent{ state->getPatternFirstItemContent() };
        std::get<1>(d->m_replaceOperatorItems)->setText(patternOneContent.first);
        std::get<3>(d->m_replaceOperatorItems)->setText(patternOneContent.second);

        QPair<QString, DFileService::AddTextFlags> patternTwoContent{ state->getPatternSecondItemContent() };
        std::get<1>(d->m_addOperatorItems)->setText(patternTwoContent.first);
        d->m_flag = patternTwoContent.second;
        std::get<3>(d->m_addOperatorItems)->setCurrentIndex(static_cast<int>(patternTwoContent.second));

        QPair<QString, QString> patternThreeContent{ state->getPatternThirdItemContent() };
        std::get<1>(d->m_customOPeratorItems)->setText(patternThreeContent.first);
        std::get<3>(d->m_customOPeratorItems)->setText(patternThreeContent.second);

        d->m_renameButtonStates = state->getbuttonStateInThreePattern();
        d->m_currentPattern = state->getCurrentPattern();
        d->m_stackWidget->setCurrentIndex(static_cast<int>(state->getCurrentPattern()));
        std::get<1>(d->m_buttonsArea)->setEnabled(d->m_renameButtonStates[d->m_currentPattern]);

        d->m_urlList = state->getSelectedUrl();
        this->setVisible(state->getVisibleValue());
    }
}

void DRenameBar::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        emit this->clickRenameButton();
        break;
    }
    case Qt::Key_Escape: {
        emit this->clickCancelButton();
        break;
    }
    default:
        break;
    }
}

void DRenameBar::setVisible(bool value)
{
    this->QFrame::setVisible(value);
    emit visibleChanged(value);
}

void DRenameBar::onVisibleChanged(bool value)noexcept
{
    DRenameBarPrivate *const d{ d_func() };

    if (value == true) {

        switch (d->m_currentPattern) {
        case 0: {
            QLineEdit *lineEdit{ std::get<1>(d->m_replaceOperatorItems) };
            lineEdit->setFocus();
            break;
        }
        case 1: {
            QLineEdit *lineEdit{ std::get<1>(d->m_addOperatorItems) };
            lineEdit->setFocus();
            break;
        }
        case 2: {
            QLineEdit *lineEdit{ std::get<1>(d->m_customOPeratorItems) };
            lineEdit->setFocus();
            break;
        }
        default: {
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
