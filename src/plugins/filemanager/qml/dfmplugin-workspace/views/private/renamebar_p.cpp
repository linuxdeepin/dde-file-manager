// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "renamebar_p.h"
#include "views/renamebar.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

using namespace dfmplugin_workspace;

RenameBarPrivate::RenameBarPrivate(RenameBar *const qPtr)
    : q_ptr(qPtr)
{
    initUI();
    setUIParameters();
    layoutItems();
}

void RenameBarPrivate::initUI()
{
    mainLayout = new QHBoxLayout(q_ptr);
    comboBox = new QComboBox;
    stackWidget = new QStackedWidget;

    replaceOperatorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QLineEdit);
    frameForLayoutReplaceArea = QPair<QHBoxLayout *, QFrame *> { new QHBoxLayout, new QFrame };

    addOperatorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QComboBox);
    frameForLayoutAddArea = QPair<QHBoxLayout *, QFrame *> { new QHBoxLayout, new QFrame };

    customOPeratorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QLineEdit, new QLabel);
    frameForLayoutCustomArea = QPair<QHBoxLayout *, QFrame *> { new QHBoxLayout, new QFrame };

    buttonsArea = std::make_tuple(new QPushButton, new QPushButton, new QHBoxLayout, new QFrame);
}

void RenameBarPrivate::setUIParameters()
{
    comboBox->addItems(QList<QString> { QObject::tr("Replace Text"), QObject::tr("Add Text"), QObject::tr("Custom Text") });
    comboBox->setFixedWidth(180);

    QComboBox *comboBox { nullptr };
    QLabel *label { std::get<0>(replaceOperatorItems) };
    QLineEdit *lineEdit { std::get<1>(replaceOperatorItems) };

    label->setText(QObject::tr("Find"));
    lineEdit->setFixedWidth(180);
    label->setObjectName(QString { "RenameBarLabel" });
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);

    label = std::get<2>(replaceOperatorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    lineEdit = std::get<3>(replaceOperatorItems);
    label->setText(QObject::tr("Replace"));
    lineEdit->setFixedWidth(180);
    lineEdit->setPlaceholderText(QObject::tr("Optional"));
    label->setBuddy(lineEdit);

    label = std::get<0>(addOperatorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    lineEdit = std::get<1>(addOperatorItems);
    label->setText(QObject::tr("Add"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);

    label = std::get<2>(addOperatorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    comboBox = std::get<3>(addOperatorItems);
    label->setText(QObject::tr("Location"));
    comboBox->addItems(QList<QString> { QObject::tr("Before file name"), QObject::tr("After file name") });
    label->setBuddy(comboBox);

    label = std::get<0>(customOPeratorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    lineEdit = std::get<1>(customOPeratorItems);
    label->setText(QObject::tr("File name"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);

    label = std::get<2>(customOPeratorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    lineEdit = std::get<3>(customOPeratorItems);
    label->setText(QObject::tr("Start at"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    lineEdit->setText(QString { "1" });

    QRegularExpression regStr { QString { "[0-9]+" } };
    validator = new QRegularExpressionValidator { regStr, lineEdit };

    lineEdit->setValidator(validator);
    label->setBuddy(lineEdit);
    label = std::get<4>(customOPeratorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    label->setText(QObject::tr("Tips: Sort by selected file order"));

    QPushButton *button { std::get<0>(buttonsArea) };
    button->setText(QObject::tr("Cancel", "button"));
    button->setFixedWidth(82);
    renameBtn = new DSuggestButton();
    renameBtn->setText(QObject::tr("Rename", "button"));
    renameBtn->setFixedWidth(82);
    button->setEnabled(true);
}

void RenameBarPrivate::layoutItems() noexcept
{

    QHBoxLayout *hBoxLayout { nullptr };
    QFrame *frame { nullptr };

    hBoxLayout = frameForLayoutReplaceArea.first;
    frame = frameForLayoutReplaceArea.second;
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->addWidget(std::get<0>(replaceOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(replaceOperatorItems));
    hBoxLayout->addSpacing(20);
    hBoxLayout->addWidget(std::get<2>(replaceOperatorItems));
    hBoxLayout->addWidget(std::get<3>(replaceOperatorItems));
    hBoxLayout->addStretch(0);
    frame->setLayout(hBoxLayout);
    stackWidget->addWidget(frame);

    hBoxLayout = frameForLayoutAddArea.first;
    frame = frameForLayoutAddArea.second;
    hBoxLayout->addSpacing(20);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->addWidget(std::get<0>(addOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(addOperatorItems));
    hBoxLayout->addSpacing(20);
    hBoxLayout->addWidget(std::get<2>(addOperatorItems));
    hBoxLayout->addWidget(std::get<3>(addOperatorItems));
    hBoxLayout->addSpacing(20);
    hBoxLayout->addStretch();
    frame->setLayout(hBoxLayout);
    stackWidget->addWidget(frame);

    hBoxLayout = frameForLayoutCustomArea.first;
    frame = frameForLayoutCustomArea.second;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    hBoxLayout->addWidget(std::get<0>(customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<2>(customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<3>(customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<4>(customOPeratorItems));
    hBoxLayout->addStretch(0);
    frame->setLayout(hBoxLayout);
    stackWidget->addWidget(frame);

    hBoxLayout = std::get<2>(buttonsArea);
    hBoxLayout->setSpacing(0);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame = std::get<3>(buttonsArea);
    hBoxLayout->addSpacing(50);
    hBoxLayout->addWidget(std::get<0>(buttonsArea));
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(renameBtn);
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hBoxLayout);

    mainLayout->addWidget(comboBox);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(stackWidget);
    mainLayout->addStretch(0);
    mainLayout->addWidget(frame);
    stackWidget->setCurrentIndex(0);

    q_ptr->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    q_ptr->setLayout(mainLayout);
}

void RenameBarPrivate::setRenameBtnStatus(const bool &value) noexcept
{
    renameBtn->setEnabled(value);
}

QString RenameBarPrivate::filteringText(const QString &text)
{
    if (text.isEmpty())
        return text;

    QString readyText = text;
    return readyText.remove(QRegularExpression("[\\\\/:\\*\\?\"<>|%&]"));
}

void RenameBarPrivate::updateLineEditText(QLineEdit *lineEdit, const QString &defaultValue)
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
