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
#include "renamebar_p.h"
#include "views/renamebar.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

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

    QComboBox *comboBox { nullptr };
    QLabel *label { std::get<0>(replaceOperatorItems) };
    QLineEdit *lineEdit { std::get<1>(replaceOperatorItems) };

    label->setText(QObject::tr("Find"));
    label->setObjectName(QString { "RenameBarLabel" });
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);

    label = std::get<2>(replaceOperatorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    lineEdit = std::get<3>(replaceOperatorItems);
    label->setText(QObject::tr("Replace"));
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

    QRegExp regStr { QString { "[0-9]+" } };
    validator = new QRegExpValidator { regStr, lineEdit };

    lineEdit->setValidator(validator);
    label->setBuddy(lineEdit);
    label = std::get<4>(customOPeratorItems);
    label->setObjectName(QString { "RenameBarLabel" });
    label->setText(QObject::tr("Tips: Sort by selected file order"));

    QPushButton *button { std::get<0>(buttonsArea) };
    button->setText(QObject::tr("Cancel", "button"));
    button = std::get<1>(buttonsArea);
    button->setText(QObject::tr("Rename", "button"));

    button->setEnabled(false);
}

void RenameBarPrivate::layoutItems() noexcept
{

    QHBoxLayout *hBoxLayout { nullptr };
    QFrame *frame { nullptr };

    hBoxLayout = frameForLayoutReplaceArea.first;
    frame = frameForLayoutReplaceArea.second;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    hBoxLayout->addWidget(std::get<0>(replaceOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(replaceOperatorItems));
    hBoxLayout->addSpacing(32);
    hBoxLayout->addWidget(std::get<2>(replaceOperatorItems));
    hBoxLayout->setSpacing(13);
    hBoxLayout->addWidget(std::get<3>(replaceOperatorItems));
    hBoxLayout->addStretch(0);
    frame->setLayout(hBoxLayout);
    stackWidget->addWidget(frame);

    hBoxLayout = frameForLayoutAddArea.first;
    frame = frameForLayoutAddArea.second;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    hBoxLayout->addWidget(std::get<0>(addOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(addOperatorItems));
    hBoxLayout->addSpacing(32);
    hBoxLayout->addWidget(std::get<2>(addOperatorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<3>(addOperatorItems));
    hBoxLayout->addStretch();
    frame->setLayout(hBoxLayout);
    stackWidget->addWidget(frame);

    hBoxLayout = frameForLayoutCustomArea.first;
    frame = frameForLayoutCustomArea.second;
    hBoxLayout->setSpacing(0);
    hBoxLayout->setMargin(0);
    hBoxLayout->addWidget(std::get<0>(customOPeratorItems));
    hBoxLayout->setSpacing(5);
    hBoxLayout->addWidget(std::get<1>(customOPeratorItems));
    hBoxLayout->addSpacing(32);
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
    hBoxLayout->setMargin(0);
    frame = std::get<3>(buttonsArea);
    hBoxLayout->addWidget(std::get<0>(buttonsArea));
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(std::get<1>(buttonsArea));
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hBoxLayout);

    mainLayout->addWidget(comboBox);
    mainLayout->addSpacing(32);
    mainLayout->addWidget(stackWidget);
    mainLayout->addStretch(0);
    mainLayout->addWidget(frame);
    stackWidget->setCurrentIndex(0);

    q_ptr->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    q_ptr->setLayout(mainLayout);
}

void RenameBarPrivate::setRenameBtnStatus(const bool &value) noexcept
{
    QPushButton *button { std::get<1>(buttonsArea) };

    button->setEnabled(value);
}

QString RenameBarPrivate::filteringText(const QString &text)
{
    if (text.isEmpty())
        return text;

    QString readyText = text;
    return readyText.remove(QRegExp("[\\\\/:\\*\\?\"<>|%&]"));
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
