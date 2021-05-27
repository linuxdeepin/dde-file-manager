/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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

#include <tuple>
#include <atomic>

#include <QPair>
#include <QLabel>
#include <QWindow>
#include <QValidator>
#include <QPushButton>
#include <QVBoxLayout>

#include "ddesktoprenamedialog.h"
#include "private/ddesktoprenamedialog_p.h"

DDesktopRenameDialogPrivate::DDesktopRenameDialogPrivate(DDesktopRenameDialog *const qPtr)
    : q_ptr{ qPtr}
{
    this->initUi();
    this->initUiParameters();
    this->initUiLayout();

}

void DDesktopRenameDialogPrivate::initUi()
{

    m_titleLabel = new QLabel{};

    m_itemsForSelecting = std::make_tuple(new QLabel{}, new QComboBox{}, new QHBoxLayout{});

    m_modeOneItemsForFinding = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{});
    m_modeOneItemsForReplacing = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{});
    m_modeOneLayout = QPair<QVBoxLayout *, QFrame *> {new QVBoxLayout{}, new QFrame{}};


    m_modeTwoItemsForAdding = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout);
    m_modeTwoItemsForLocating = std::make_tuple(new QLabel{}, new QComboBox{}, new QHBoxLayout{});
    m_modeTwoLayout = QPair<QVBoxLayout *, QFrame *> {new QVBoxLayout{}, new QFrame{}};


    m_modeThreeItemsForFileName = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{});
    m_modeThreeItemsForSNNumber = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{});
    m_modeThreeLayout = QPair<QVBoxLayout *, QFrame *> {new QVBoxLayout{}, new QFrame{}};

    QRegExp regStr{ QString{"[0-9]+"} };
    m_validator = new QRegExpValidator{ regStr, this->q_ptr };

    m_mainFrame = new QFrame(this->q_ptr);
    m_mainLayout = new QVBoxLayout(m_mainFrame);
}


void DDesktopRenameDialogPrivate::initUiParameters()
{
    m_titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *tagLabel{ nullptr };
    QComboBox *modeChoseBox{ nullptr };
    QLineEdit *contentLineEdit{ nullptr };

    tagLabel = std::get<0>(m_itemsForSelecting);
    tagLabel->setText(QObject::tr("Mode:"));
    modeChoseBox = std::get<1>(m_itemsForSelecting);
    modeChoseBox->addItems(QList<QString> { QObject::tr("Replace Text"), QObject::tr("Add Text"), QObject::tr("Custom Text") });
    modeChoseBox->setFixedSize(QSize{275, 25});

    ///###: mode 1
    tagLabel = std::get<0>(m_modeOneItemsForFinding);
    tagLabel->setText(QObject::tr("Find:"));
    contentLineEdit = std::get<1>(m_modeOneItemsForFinding);
    contentLineEdit->setFocus();
    contentLineEdit->setPlaceholderText(QObject::tr("Required"));
    contentLineEdit->setFixedSize(QSize{275, 25});

    tagLabel = std::get<0>(m_modeOneItemsForReplacing);
    tagLabel->setText(QObject::tr("Replace:"));
    contentLineEdit = std::get<1>(m_modeOneItemsForReplacing);
    contentLineEdit->setPlaceholderText(QObject::tr("Optional"));
    contentLineEdit->setFixedSize(QSize{275, 25});

    ///###: mode 2
    tagLabel = std::get<0>(m_modeTwoItemsForAdding);
    tagLabel->setText(QObject::tr("Add:"));
    contentLineEdit = std::get<1>(m_modeTwoItemsForAdding);
    contentLineEdit->setPlaceholderText(QObject::tr("Required"));
    contentLineEdit->setFixedSize(QSize{275, 25});
    contentLineEdit->setMaxLength(300);

    tagLabel = std::get<0>(m_modeTwoItemsForLocating);
    tagLabel->setText(QObject::tr("Location:"));
    modeChoseBox = std::get<1>(m_modeTwoItemsForLocating);
    modeChoseBox->addItems(QList<QString> { QObject::tr("Before file name"), QObject::tr("After file name") });
    modeChoseBox->setFixedSize(QSize{275, 25});

    ///###: mode3
    tagLabel = std::get<0>(m_modeThreeItemsForFileName);
    tagLabel->setText(QObject::tr("File name:"));
    contentLineEdit = std::get<1>(m_modeThreeItemsForFileName);
    contentLineEdit->setPlaceholderText(QObject::tr("Required"));
    contentLineEdit->setFixedSize(QSize{275, 25});

    tagLabel = std::get<0>(m_modeThreeItemsForSNNumber);
    tagLabel->setText(QObject::tr("Start at:"));
    contentLineEdit = std::get<1>(m_modeThreeItemsForSNNumber);
    contentLineEdit->setPlaceholderText(QObject::tr("Required"));
    contentLineEdit->setFixedSize(QSize{275, 25});
    contentLineEdit->setValidator(m_validator);
    contentLineEdit->setText(QString{"1"});

    tagLabel = nullptr;
    modeChoseBox = nullptr;
    contentLineEdit = nullptr;
}

void DDesktopRenameDialogPrivate::initUiLayout()
{
    QLabel *tagLabel{ nullptr };
    QComboBox *modeChoseBox{ nullptr };
    QLineEdit *contentLineEdit{ nullptr };
    QHBoxLayout *hLayout{ nullptr };


    tagLabel = std::get<0>(m_itemsForSelecting);
    modeChoseBox = std::get<1>(m_itemsForSelecting);
    tagLabel->setBuddy(modeChoseBox);
    hLayout = std::get<2>(m_itemsForSelecting);
    hLayout->addWidget(tagLabel);
    hLayout->addSpacing(30);
    hLayout->addWidget(modeChoseBox);
    hLayout->setMargin(0);


    ///###: mode 1
    tagLabel = std::get<0>(m_modeOneItemsForFinding);
    contentLineEdit = std::get<1>(m_modeOneItemsForFinding);
    tagLabel->setBuddy(contentLineEdit);
    hLayout = std::get<2>(m_modeOneItemsForFinding);
    hLayout->addWidget(tagLabel);
    hLayout->addSpacing(30);
    hLayout->addWidget(contentLineEdit);
    hLayout->setMargin(0);


    tagLabel = std::get<0>(m_modeOneItemsForReplacing);
    contentLineEdit = std::get<1>(m_modeOneItemsForReplacing);
    tagLabel->setBuddy(contentLineEdit);
    hLayout = std::get<2>(m_modeOneItemsForReplacing);
    hLayout->addWidget(tagLabel);
    hLayout->addSpacing(30);
    hLayout->addWidget(contentLineEdit);


    m_modeOneLayout.first->addLayout(std::get<2>(m_modeOneItemsForFinding));
    m_modeOneLayout.first->addSpacing(10);
    m_modeOneLayout.first->addLayout(std::get<2>(m_modeOneItemsForReplacing));
    m_modeOneLayout.first->setSpacing(0);
    m_modeOneLayout.first->setMargin(0);
    m_modeOneLayout.second->setLayout(m_modeOneLayout.first);




    ///###: mode 2
    hLayout = std::get<2>(m_modeTwoItemsForAdding);
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    tagLabel = std::get<0>(m_modeTwoItemsForAdding);
    contentLineEdit = std::get<1>(m_modeTwoItemsForAdding);
    tagLabel->setBuddy(contentLineEdit);
    hLayout->addWidget(tagLabel);
    hLayout->addSpacing(30);
    hLayout->addWidget(contentLineEdit);
    hLayout->setMargin(0);


    tagLabel = std::get<0>(m_modeTwoItemsForLocating);
    modeChoseBox = std::get<1>(m_modeTwoItemsForLocating);
    tagLabel->setBuddy(modeChoseBox);
    hLayout = std::get<2>(m_modeTwoItemsForLocating);
    hLayout->addWidget(tagLabel);
    hLayout->addWidget(modeChoseBox);


    m_modeTwoLayout.first->addLayout(std::get<2>(m_modeTwoItemsForAdding));
    m_modeTwoLayout.first->addSpacing(10);
    m_modeTwoLayout.first->addLayout(std::get<2>(m_modeTwoItemsForLocating));
    m_modeTwoLayout.first->setSpacing(0);
    m_modeTwoLayout.first->setMargin(0);
    m_modeTwoLayout.second->setLayout(m_modeTwoLayout.first);


    ///###: mode 3
    tagLabel = std::get<0>(m_modeThreeItemsForFileName);
    contentLineEdit = std::get<1>(m_modeThreeItemsForFileName);
    tagLabel->setBuddy(contentLineEdit);
    hLayout = std::get<2>(m_modeThreeItemsForFileName);
    hLayout->addWidget(tagLabel);
    hLayout->addWidget(contentLineEdit);

    tagLabel = std::get<0>(m_modeThreeItemsForSNNumber);
    contentLineEdit = std::get<1>(m_modeThreeItemsForSNNumber);
    tagLabel->setBuddy(contentLineEdit);
    hLayout = std::get<2>(m_modeThreeItemsForSNNumber);
    hLayout->addWidget(tagLabel);
    hLayout->addWidget(contentLineEdit);


    m_modeThreeLayout.first->addLayout(std::get<2>(m_modeThreeItemsForFileName));
    m_modeThreeLayout.first->addSpacing(10);
    m_modeThreeLayout.first->addLayout(std::get<2>(m_modeThreeItemsForSNNumber));
    m_modeThreeLayout.first->setSpacing(0);
    m_modeThreeLayout.first->setMargin(0);
    m_modeThreeLayout.second->setLayout(m_modeThreeLayout.first);

    ///###: total layout.
    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(0);
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addSpacing(30);
    m_mainLayout->addLayout(std::get<2>(m_itemsForSelecting));
    m_mainLayout->addSpacing(10);

    ///（Qt5.15.0）wayland下必须在此处创建m_stackedLayout并设置父子关系
    ///不设置父子关系会导致在m_stackedLayout中点击鼠标时无法获取焦点
    ///提前创建对象，再在此处设置父子关系无效，依然存在点击鼠标无焦点问题
    ///提前创建对象并设置父子关系，会导致m_stackedLayout位置在布局的最上面
    m_stackedLayout = new QStackedLayout(m_mainLayout);
    m_stackedLayout->addWidget(m_modeOneLayout.second);
    m_stackedLayout->addWidget(m_modeTwoLayout.second);
    m_stackedLayout->addWidget(m_modeThreeLayout.second);
    m_stackedLayout->setCurrentIndex(0);
    m_mainLayout->addLayout(m_stackedLayout);

    m_mainLayout->addSpacing(15);

    m_mainFrame->setLayout(m_mainLayout);
}

QString DDesktopRenameDialogPrivate::filteringText(const QString &text)
{
    if (text.isEmpty())
        return text;

    QString readyText = text;
    return readyText.remove(QRegExp("[\\\\/:\\*\\?\"<>|%&]"));
}

void DDesktopRenameDialogPrivate::updateLineEditText(QLineEdit *lineEdit, const QString &defaultValue)
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

DDesktopRenameDialog::DDesktopRenameDialog(QWidget *const parent)
    : DDialog{ parent },
      d_ptr{ new DDesktopRenameDialogPrivate{ this } }
{
    this->initUi();
    this->initConnect();

    this->installEventFilter(this);
}


///###: I initialize buttons here.
///   and other widgets in DDesktopRenameDialog is initialized when new DDesktopRenameDialogPrivate.
void DDesktopRenameDialog::initUi()
{
    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    Q_D(const DDesktopRenameDialog);
    this->addContent(d->m_mainFrame, Qt::AlignCenter);
    this->addButton(QObject::tr("Cancel","button"));
    this->addButton(QObject::tr("Rename","button"), true, ButtonRecommend);
}

void DDesktopRenameDialog::initConnect()
{
    Q_D(const DDesktopRenameDialog);
    using funcType = void (QComboBox::*)(int index);


    QObject::connect(std::get<1>(d->m_itemsForSelecting), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &DDesktopRenameDialog::onCurrentModeChanged);
    QObject::connect(std::get<1>(d->m_modeTwoItemsForLocating), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &DDesktopRenameDialog::onCurrentAddModeChanged);
    QObject::connect(std::get<1>(d->m_modeThreeItemsForSNNumber), &QLineEdit::textChanged, this, &DDesktopRenameDialog::onContentChangedForCustomzedSN);
    QObject::connect(this, &DDesktopRenameDialog::visibleChanged, this, &DDesktopRenameDialog::onVisibleChanged);

    connect(std::get<1>(d->m_modeOneItemsForFinding), &QLineEdit::textChanged, this, [ = ]() {
        onVisibleChanged(true);
    });
    connect(std::get<1>(d->m_modeOneItemsForReplacing), &QLineEdit::textChanged, this, [ = ]() {
        onReplaceTextChanged();
    });
    connect(std::get<1>(d->m_modeTwoItemsForAdding), &QLineEdit::textChanged, this, [ = ]() {
        onVisibleChanged(true);
    });
    connect(std::get<1>(d->m_modeThreeItemsForFileName), &QLineEdit::textChanged, this, [ = ]() {
        onVisibleChanged(true);
    });

    try {

        if (QPushButton *cancelButton = dynamic_cast<QPushButton *>(this->getButton(0))) {
            QObject::connect(this, &DDesktopRenameDialog::clickCancelButton, cancelButton, &QPushButton::click);
        }

        if (QPushButton *renameButton = dynamic_cast<QPushButton *>(this->getButton(1))) {
            QObject::connect(this, &DDesktopRenameDialog::clickRenameButton, renameButton, &QPushButton::click);
        }

    } catch (const std::bad_cast &error) {
        (void)error;

        throw std::runtime_error{ "badly dynamic cast in DDesktopRenameDialog" };
    }

}


void DDesktopRenameDialog::onCurrentModeChanged(const std::size_t &index)noexcept
{
    DDesktopRenameDialogPrivate *const d{ d_func() };

    d->m_currentmode = index;
    d->m_stackedLayout->setCurrentIndex(index);

    ///###: here, make lineEdit has focus immediately, after change mode.
    this->onVisibleChanged(true);
}

void DDesktopRenameDialog::onCurrentAddModeChanged(const std::size_t &index)noexcept
{
    DDesktopRenameDialogPrivate *d{ d_func() };

    if (index == 0) {
        d->m_flagForAdding = DFileService::AddTextFlags::Before;

    } else {
        d->m_flagForAdding = DFileService::AddTextFlags::After;
    }

    ///###: here, make lineEdit has focus immediately, after change mode.
    this->onVisibleChanged(true);
}

std::size_t DDesktopRenameDialog::getCurrentModeIndex()const noexcept
{
    const DDesktopRenameDialogPrivate *const d{ d_func() };

    return d->m_currentmode;
}

DFileService::AddTextFlags DDesktopRenameDialog::getAddMode()const noexcept
{
    const DDesktopRenameDialogPrivate *const d{ d_func() };

    return d->m_flagForAdding;
}

QPair<QString, QString> DDesktopRenameDialog::getModeOneContent()const noexcept
{
    const DDesktopRenameDialogPrivate *const d{ d_func() };

    QString findStr{ std::get<1>(d->m_modeOneItemsForFinding)->text() };
    QString replaceStr{ std::get<1>(d->m_modeOneItemsForReplacing)->text() };

    return QPair<QString, QString> { findStr, replaceStr };
}


QPair<QString, DFileService::AddTextFlags> DDesktopRenameDialog::getModeTwoContent()const noexcept
{
    const DDesktopRenameDialogPrivate *const d{ d_func() };

    QString addStr{ std::get<1>(d->m_modeTwoItemsForAdding)->text() };
    DFileService::AddTextFlags flag{ d->m_flagForAdding };

    return QPair<QString, DFileService::AddTextFlags> { addStr, flag };
}


QPair<QString, QString> DDesktopRenameDialog::getModeThreeContent()const noexcept
{
    const DDesktopRenameDialogPrivate *const d{ d_func() };

    QString fileName{ std::get<1>(d->m_modeThreeItemsForFileName)->text() };
    QString numberStr{ std::get<1>(d->m_modeThreeItemsForSNNumber)->text() };

    if (numberStr.isEmpty()) { // if the number is empty should set one default value
        numberStr = QString{"1"};
    }

    return QPair<QString, QString> { fileName, numberStr};
}


///###: This function will change enable of property of the button and
///     style sheet.
void DDesktopRenameDialog::setRenameButtonStatus(const bool &enabled)
{
    if (QPushButton *renameButton = dynamic_cast<QPushButton *>(this->getButton(1))) {

        renameButton->setEnabled(enabled);
    }
}


void DDesktopRenameDialog::onContentChangedForFinding(const QString &content)
{
    DDesktopRenameDialogPrivate *const d{ d_func() };

    if (content.isEmpty() == false) {
        d->m_currentEnabled[0] = true;
        this->setRenameButtonStatus(true);

    } else {
        d->m_currentEnabled[0] = false;
        this->setRenameButtonStatus(false);
    }
}


void DDesktopRenameDialog::onContentChangedForAdding(const QString &content)
{
    DDesktopRenameDialogPrivate *const d{ d_func() };

    if (content.isEmpty() == false) {
        d->m_currentEnabled[1] = true;
        this->setRenameButtonStatus(true);

    } else {
        d->m_currentEnabled[1] = false;
        this->setRenameButtonStatus(false);
    }
}

void DDesktopRenameDialog::onContentChangedForCustomzedSN(const QString &content)
{
    DDesktopRenameDialogPrivate *d{ d_func() };

    QLineEdit *lineEditForSNNumber{ std::get<1>(d->m_modeThreeItemsForSNNumber) };

    std::string numberStr{ content.toStdString() };
    try {
        Q_UNUSED(std::stoull(numberStr));

    } catch (const std::out_of_range &err) {
        (void)err;
        lineEditForSNNumber->setText(QString{"1"});

    } catch (...) {
        if (!numberStr.empty()) { // bug 26538: can edit the number
            lineEditForSNNumber->setText(QString{"1"});
        }
    }
}

void DDesktopRenameDialog::setDialogTitle(const QString &tile)noexcept
{
    DDesktopRenameDialogPrivate *const d{ d_func() };
    d->m_titleLabel->setText(tile);
}


void DDesktopRenameDialog::setVisible(bool visible)
{
    this->DDialog::setVisible(visible);
    emit visibleChanged(visible);
}

void DDesktopRenameDialog::onReplaceTextChanged()noexcept
{
    DDesktopRenameDialogPrivate *const d{ d_func() };

    QLineEdit *lineEdit{ std::get<1>(d->m_modeOneItemsForReplacing) };

    d->updateLineEditText(lineEdit);
}

void DDesktopRenameDialog::onVisibleChanged(bool visible)noexcept
{
    DDesktopRenameDialogPrivate *const d{ d_func() };

    if (visible == true) {

        switch (d->m_currentmode) {
        case 0: {
            // for finding
            QLineEdit *lineEdit = { std::get<1>(d->m_modeOneItemsForFinding) };

            d->updateLineEditText(lineEdit);
            setRenameButtonStatus(!lineEdit->text().isEmpty());
            lineEdit->setFocus();
            break;
        }
        case 1: {
            QLineEdit *lineEdit{ std::get<1>(d->m_modeTwoItemsForAdding) };

            d->updateLineEditText(lineEdit);
            setRenameButtonStatus(!lineEdit->text().isEmpty());
            lineEdit->setFocus();
            break;
        }
        case 2: {
            QLineEdit *lineEdit{ std::get<1>(d->m_modeThreeItemsForFileName) };

            d->updateLineEditText(lineEdit);
            setRenameButtonStatus(!lineEdit->text().isEmpty());
            lineEdit->setFocus();

            QLineEdit *lineEditForSNNumber{ std::get<1>(d->m_modeThreeItemsForSNNumber) };
            d->updateLineEditText(lineEditForSNNumber, "1");

            break;
        }
        default:
            break;

        }
    }
}


