// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "renamedialog.h"
#include "private/renamedialog_p.h"

#include <dfm-base/utils/windowutils.h>

#include <QPushButton>
#include <QWindow>
#include <QtGlobal>
#include <QDebug>

#define CONTENTSIZE QSize(275, 25)
#define HSPACEWIDTH 30
#define VSPACEWIDTH 10
#define MARGINWIDTH 0

using namespace ddplugin_canvas;
DFMBASE_USE_NAMESPACE

RenameDialogPrivate::RenameDialogPrivate(RenameDialog *qq)
    : q(qq)
{
    initUi();
    initParameters();
    initLayout();
    initConnect();
}

void RenameDialogPrivate::initUi()
{
    titleLabel = new QLabel;

    modeSelection = std::make_tuple(new QLabel, new QComboBox, new QHBoxLayout);

    replaceForFinding = std::make_tuple(new QLabel, new QLineEdit, new QHBoxLayout);
    replaceForReplacing = std::make_tuple(new QLabel, new QLineEdit, new QHBoxLayout);
    replaceLayout = QPair<QVBoxLayout *, QFrame *>(new QVBoxLayout, new QFrame);

    addForAdding = std::make_tuple(new QLabel, new QLineEdit, new QHBoxLayout);
    addForLocating = std::make_tuple(new QLabel, new QComboBox, new QHBoxLayout);
    addLayout = QPair<QVBoxLayout *, QFrame *>(new QVBoxLayout, new QFrame);

    customForName = std::make_tuple(new QLabel, new QLineEdit, new QHBoxLayout);
    customForNumber = std::make_tuple(new QLabel, new QLineEdit, new QHBoxLayout);
    customLayout = QPair<QVBoxLayout *, QFrame *>(new QVBoxLayout, new QFrame);

    mainFrame = new QFrame(q);
    mainLayout = new QVBoxLayout(mainFrame);

    QRegularExpression regStr("[0-9]+");
    validator = new QRegularExpressionValidator(regStr, q);
}

void RenameDialogPrivate::initParameters()
{
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *label = nullptr;
    QLineEdit *lineEdit = nullptr;
    QComboBox *comboBox = nullptr;

    label = std::get<0>(modeSelection);
    label->setText(tr("Mode:"));
    comboBox = std::get<1>(modeSelection);
    comboBox->addItems(QStringList() << tr("Replace Text") << tr("Add Text") << tr("Custom Text"));
    comboBox->setFixedSize(CONTENTSIZE);

    // mode:replace
    label = std::get<0>(replaceForFinding);
    label->setText(tr("Find:"));
    lineEdit = std::get<1>(replaceForFinding);
    lineEdit->setFocus();
    lineEdit->setPlaceholderText(tr("Required"));
    lineEdit->setFixedSize(CONTENTSIZE);

    label = std::get<0>(replaceForReplacing);
    label->setText(tr("Replace:"));
    lineEdit = std::get<1>(replaceForReplacing);
    lineEdit->setPlaceholderText(tr("Optional"));
    lineEdit->setFixedSize(CONTENTSIZE);

    // mode:add
    label = std::get<0>(addForAdding);
    label->setText(tr("Add:"));
    lineEdit = std::get<1>(addForAdding);
    lineEdit->setPlaceholderText(tr("Required"));
    lineEdit->setMaxLength(300);
    lineEdit->setFixedSize(CONTENTSIZE);

    label = std::get<0>(addForLocating);
    label->setText(tr("Location:"));
    comboBox = std::get<1>(addForLocating);
    comboBox->addItems(QStringList() << tr("Before file name") << tr("After file name"));
    comboBox->setFixedSize(CONTENTSIZE);

    // mode:custom
    label = std::get<0>(customForName);
    label->setText(tr("File name:"));
    lineEdit = std::get<1>(customForName);
    lineEdit->setPlaceholderText(tr("Required"));
    lineEdit->setFixedSize(CONTENTSIZE);

    label = std::get<0>(customForNumber);
    label->setText(tr("Start at:"));
    lineEdit = std::get<1>(customForNumber);
    lineEdit->setPlaceholderText(tr("Required"));
    lineEdit->setFixedSize(CONTENTSIZE);
    lineEdit->setValidator(validator);
    lineEdit->setText(QStringLiteral("1"));

    label = nullptr;
    lineEdit = nullptr;
    comboBox = nullptr;
}

void RenameDialogPrivate::initLayout()
{
    QLabel *label = nullptr;
    QLineEdit *lineEdit = nullptr;
    QComboBox *comboBox = nullptr;
    QHBoxLayout *hLayout = nullptr;

    label = std::get<0>(modeSelection);
    comboBox = std::get<1>(modeSelection);
    label->setBuddy(comboBox);
    hLayout = std::get<2>(modeSelection);
    hLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    hLayout->addWidget(label);
    hLayout->addSpacing(HSPACEWIDTH);
    hLayout->addWidget(comboBox);

    // mode:replace
    label = std::get<0>(replaceForFinding);
    lineEdit = std::get<1>(replaceForFinding);
    label->setBuddy(lineEdit);
    hLayout = std::get<2>(replaceForFinding);
    hLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    hLayout->addWidget(label);
    hLayout->addSpacing(HSPACEWIDTH);
    hLayout->addWidget(lineEdit);

    label = std::get<0>(replaceForReplacing);
    lineEdit = std::get<1>(replaceForReplacing);
    label->setBuddy(lineEdit);
    hLayout = std::get<2>(replaceForReplacing);
    hLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    hLayout->addWidget(label);
    hLayout->addSpacing(HSPACEWIDTH);
    hLayout->addWidget(lineEdit);

    replaceLayout.first->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    replaceLayout.first->addLayout(std::get<2>(replaceForFinding));
    replaceLayout.first->addSpacing(VSPACEWIDTH);
    replaceLayout.first->addLayout(std::get<2>(replaceForReplacing));
    replaceLayout.second->setLayout(replaceLayout.first);

    // mode:add
    label = std::get<0>(addForAdding);
    lineEdit = std::get<1>(addForAdding);
    label->setBuddy(lineEdit);
    hLayout = std::get<2>(addForAdding);
    hLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    hLayout->addWidget(label);
    hLayout->addSpacing(HSPACEWIDTH);
    hLayout->addWidget(lineEdit);

    label = std::get<0>(addForLocating);
    comboBox = std::get<1>(addForLocating);
    label->setBuddy(comboBox);
    hLayout = std::get<2>(addForLocating);
    hLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    hLayout->addWidget(label);
    hLayout->addSpacing(HSPACEWIDTH);
    hLayout->addWidget(comboBox);

    addLayout.first->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    addLayout.first->addLayout(std::get<2>(addForAdding));
    addLayout.first->addSpacing(VSPACEWIDTH);
    addLayout.first->addLayout(std::get<2>(addForLocating));
    addLayout.second->setLayout(addLayout.first);

    // mode:custom
    label = std::get<0>(customForName);
    lineEdit = std::get<1>(customForName);
    label->setBuddy(lineEdit);
    hLayout = std::get<2>(customForName);
    hLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    hLayout->addWidget(label);
    hLayout->addSpacing(HSPACEWIDTH);
    hLayout->addWidget(lineEdit);

    label = std::get<0>(customForNumber);
    lineEdit = std::get<1>(customForNumber);
    label->setBuddy(lineEdit);
    hLayout = std::get<2>(customForNumber);
    hLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    hLayout->addWidget(label);
    hLayout->addSpacing(HSPACEWIDTH);
    hLayout->addWidget(lineEdit);

    customLayout.first->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    customLayout.first->addLayout(std::get<2>(customForName));
    customLayout.first->addSpacing(VSPACEWIDTH);
    customLayout.first->addLayout(std::get<2>(customForNumber));
    customLayout.second->setLayout(customLayout.first);

    // total layout
    mainLayout->setContentsMargins(MARGINWIDTH, MARGINWIDTH, MARGINWIDTH, MARGINWIDTH);
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(30);
    mainLayout->addLayout(std::get<2>(modeSelection));
    mainLayout->addSpacing(VSPACEWIDTH);

    stackedLayout = new QStackedLayout(mainLayout);
    stackedLayout->addWidget(replaceLayout.second);
    stackedLayout->addWidget(addLayout.second);
    stackedLayout->addWidget(customLayout.second);
    stackedLayout->setCurrentIndex(0);
    mainLayout->addLayout(stackedLayout);

    mainLayout->addSpacing(15);

    mainFrame->setLayout(mainLayout);
}

void RenameDialogPrivate::initConnect()
{
    connect(std::get<1>(modeSelection), QOverload<int>::of(&QComboBox::currentIndexChanged), stackedLayout, &QStackedLayout::setCurrentIndex);
    connect(std::get<1>(modeSelection), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RenameDialogPrivate::updateStates);
    connect(q, &RenameDialog::visibleChanged, this, &RenameDialogPrivate::updateStates);

    // require content
    connect(std::get<1>(replaceForFinding), &QLineEdit::textChanged, this, &RenameDialogPrivate::updateStates);
    connect(std::get<1>(addForAdding), &QLineEdit::textChanged, this, &RenameDialogPrivate::updateStates);
    connect(std::get<1>(customForName), &QLineEdit::textChanged, this, &RenameDialogPrivate::updateStates);
    connect(std::get<1>(customForNumber), &QLineEdit::textChanged, this, &RenameDialogPrivate::updateStates);

    connect(std::get<1>(addForLocating), QOverload<int>::of(&QComboBox::currentIndexChanged), std::get<1>(addForAdding), QOverload<>::of(&QLineEdit::setFocus));
}

void RenameDialogPrivate::updateStates()
{
    int index = std::get<1>(modeSelection)->currentIndex();

    if (0 == index) {
        std::get<1>(replaceForFinding)->setFocus();
        bool enable = !std::get<1>(replaceForFinding)->text().isEmpty();
        setRenameButtonStatus(enable);
    } else if (1 == index) {
        std::get<1>(addForAdding)->setFocus();
        bool enable = !std::get<1>(addForAdding)->text().isEmpty();
        setRenameButtonStatus(enable);
    } else if (2 == index) {
        if (!std::get<1>(customForName)->hasFocus() && !std::get<1>(customForNumber)->hasFocus())
            std::get<1>(customForName)->setFocus();
        bool enable = !std::get<1>(customForName)->text().isEmpty() && !std::get<1>(customForNumber)->text().isEmpty();
        setRenameButtonStatus(enable);
    }
}

void RenameDialogPrivate::setRenameButtonStatus(const bool &enabled)
{
    if (QPushButton *renameButton = dynamic_cast<QPushButton *>(q->getButton(1))) {

        renameButton->setEnabled(enabled);
    }
}

RenameDialog::RenameDialog(int fileCount, QWidget *parent)
    : DDialog(parent), d(new RenameDialogPrivate(this))
{
    d->titleLabel->setText(tr("Rename %1 Files").arg(QString::number(fileCount)));

    initUi();
}

RenameDialog::ModifyMode RenameDialog::modifyMode() const
{
    return ModifyMode(std::get<1>(d->modeSelection)->currentIndex());
}

QPair<QString, QString> RenameDialog::getReplaceContent() const
{
    QString findStr { std::get<1>(d->replaceForFinding)->text() };
    QString replaceStr { std::get<1>(d->replaceForReplacing)->text() };

    return QPair<QString, QString> { findStr, replaceStr };
}

QPair<QString, AbstractJobHandler::FileNameAddFlag> RenameDialog::getAddContent() const
{
    QString addStr { std::get<1>(d->addForAdding)->text() };
    AbstractJobHandler::FileNameAddFlag flag;
    if (0 == std::get<1>(d->addForLocating)->currentIndex())
        flag = AbstractJobHandler::FileNameAddFlag::kPrefix;
    else
        flag = AbstractJobHandler::FileNameAddFlag::kSuffix;

    return QPair<QString, AbstractJobHandler::FileNameAddFlag> { addStr, flag };
}

QPair<QString, QString> RenameDialog::getCustomContent() const
{
    QString fileName { std::get<1>(d->customForName)->text() };
    QString numberStr { std::get<1>(d->customForNumber)->text() };

    // if the number is empty should set one default value
    if (numberStr.isEmpty()) {
        numberStr = QString { "1" };
    }

    return QPair<QString, QString> { fileName, numberStr };
}

void RenameDialog::initUi()
{
    if (WindowUtils::isWayLand()) {
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    this->addContent(d->mainFrame, Qt::AlignCenter);
    this->addButton(QObject::tr("Cancel", "button"));
    this->addButton(QObject::tr("Rename", "button"), true, ButtonRecommend);
}
