

#include <tuple>
#include <atomic>

#include <QPair>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QValidator>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedLayout>

#include "ddesktoprenamedialog.h"

class DDesktopRenameDialogPrivate
{
public:
    explicit DDesktopRenameDialogPrivate(DDesktopRenameDialog* const qPtr);
    ~DDesktopRenameDialogPrivate()=default;
    DDesktopRenameDialogPrivate(const DDesktopRenameDialogPrivate& other)=delete;
    DDesktopRenameDialogPrivate& operator=(const DDesktopRenameDialogPrivate& other)=delete;


    void initUi();
    void initUiParamenters();
    void initUiLayout();

    DDesktopRenameDialog* q_ptr{ nullptr };

    QLabel* m_titleLabel{ nullptr };

    std::tuple<QLabel*, QComboBox*, QHBoxLayout*, QFrame*> m_itemsForSelecting{};
    QStackedLayout* m_stackedLayout{ nullptr };

    std::tuple<QLabel*, QLineEdit*, QHBoxLayout*> m_modeOneItemsForFinding{};
    std::tuple<QLabel*, QLineEdit*, QHBoxLayout*> m_modeOneItemsForReplacing{};
    QPair<QVBoxLayout*, QFrame*> m_modeOneLayout{};


    std::tuple<QLabel*, QLineEdit*, QHBoxLayout*> m_modeTwoItemsForAdding{};
    std::tuple<QLabel*, QComboBox*, QHBoxLayout*> m_modeTwoItemsForLocating{};
    QPair<QVBoxLayout*, QFrame*> m_modeTwoLayout{};


    std::tuple<QLabel*, QLineEdit*, QHBoxLayout*> m_modeThreeItemsForFileName{};
    std::tuple<QLabel*, QLineEdit*, QHBoxLayout*> m_modeThreeItemsForSNNumber{};
    QPair<QVBoxLayout*, QFrame*> m_modeThreeLayout{};

    QVBoxLayout* m_mainLayout{ nullptr };
    QFrame* m_mainFrame{ nullptr };
    QValidator* m_validatorForSN{ nullptr };//###: this validator validate SN,
    std::size_t m_currentmode{ 0 };      //###: reocord current mode.
    std::array<bool, 3>  m_currentEnabled{ false }; //###: this array record current status of renamebutton at current mode.
    DFileService::AddTextFlags m_flagForAdding{ DFileService::AddTextFlags::Before }; //###: this flag is useful in mode 2. It record what to append str to name.


    ///###: style sheet for QLineEdit and button.
    const QPair<QString, QString> m_renameButtonStyles{ QString{ "" }, QString{ "color: blue;" } };
    const QPair<QString, QString> m_lineEditStyles{QString{ "" }, QString{"border: 1px solid blue; border-radius: 3px;"} };
};


DDesktopRenameDialogPrivate::DDesktopRenameDialogPrivate(DDesktopRenameDialog * const qPtr)
                        :q_ptr{ qPtr}
{
    this->initUi();
    this->initUiLayout();
    this->initUiParamenters();

}

void DDesktopRenameDialogPrivate::initUi()
{

    m_titleLabel = new QLabel{};

    m_itemsForSelecting = std::make_tuple(new QLabel{}, new QComboBox{}, new QHBoxLayout{}, new QFrame{});
    m_stackedLayout = new QStackedLayout{};

    m_modeOneItemsForFinding = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{}/*, new QFrame{}*/);
    m_modeOneItemsForReplacing = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{}/*, new QFrame{}*/);
    m_modeOneLayout = QPair<QVBoxLayout*, QFrame*>{new QVBoxLayout{}, new QFrame{}};


    m_modeTwoItemsForAdding = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout/*, new QFrame{}*/);
    m_modeTwoItemsForLocating = std::make_tuple(new QLabel{}, new QComboBox{}, new QHBoxLayout{}/*, new QFrame{}*/);
    m_modeTwoLayout = QPair<QVBoxLayout*, QFrame*>{new QVBoxLayout{}, new QFrame{}};


    m_modeThreeItemsForFileName = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{}/*, new QFrame{}*/);
    m_modeThreeItemsForSNNumber = std::make_tuple(new QLabel{}, new QLineEdit{}, new QHBoxLayout{}/*, new QFrame{}*/);
    m_modeThreeLayout = QPair<QVBoxLayout*, QFrame*>{new QVBoxLayout{}, new QFrame{}};

    m_mainLayout = new QVBoxLayout{};
    m_mainFrame = new QFrame{};
    QRegExp regx{ "[0-9]{1,255}" }; //limit the value for SN, make value of QLineEdit be number.
    m_validatorForSN = new QRegExpValidator{regx, q_ptr};
}


void DDesktopRenameDialogPrivate::initUiParamenters()
{
    m_titleLabel->setAlignment(Qt::AlignCenter);

    std::get<0>(m_itemsForSelecting)->setText(QObject::tr("Mode:"));
    std::get<1>(m_itemsForSelecting)->addItems(QList<QString>{ QObject::tr("Replace Text"), QObject::tr("Add Text"), QObject::tr("Custom Text") });
    std::get<1>(m_itemsForSelecting)->setFixedSize(QSize{275, 25});


    ///###: mode 1
    std::get<0>(m_modeOneItemsForFinding)->setText(QObject::tr("Find:"));
    std::get<1>(m_modeOneItemsForFinding)->setPlaceholderText(QObject::tr("Required"));
    std::get<1>(m_modeOneItemsForFinding)->setFixedSize(QSize{275, 25});
    std::get<1>(m_modeOneItemsForFinding)->setStyleSheet(m_lineEditStyles.second);
    std::get<0>(m_modeOneItemsForReplacing)->setText(QObject::tr("Replace:"));
    std::get<1>(m_modeOneItemsForReplacing)->setPlaceholderText(QObject::tr("Required"));
    std::get<1>(m_modeOneItemsForReplacing)->setFixedSize(QSize{275, 25});
    std::get<1>(m_modeOneItemsForReplacing)->setStyleSheet(m_lineEditStyles.second);


    ///###: mode 2
    std::get<0>(m_modeTwoItemsForAdding)->setText(QObject::tr("Add:"));
    std::get<1>(m_modeTwoItemsForAdding)->setPlaceholderText(QObject::tr("Required:"));
    std::get<1>(m_modeTwoItemsForAdding)->setFixedSize(QSize{275, 25});
    std::get<1>(m_modeTwoItemsForAdding)->setStyleSheet(m_lineEditStyles.second);
    std::get<0>(m_modeTwoItemsForLocating)->setText(QObject::tr("Location:"));
    std::get<1>(m_modeTwoItemsForLocating)->addItems(QList<QString>{ QObject::tr("Before file name"), QObject::tr("After file name") });
    std::get<1>(m_modeTwoItemsForLocating)->setFixedSize( QSize{275, 25} );

    ///###: mode3
    std::get<0>(m_modeThreeItemsForFileName)->setText(QObject::tr("File name:"));
    std::get<1>(m_modeThreeItemsForFileName)->setPlaceholderText(QObject::tr("Required"));
    std::get<1>(m_modeThreeItemsForFileName)->setFixedSize(QSize{275, 25});
    std::get<1>(m_modeThreeItemsForFileName)->setStyleSheet(m_lineEditStyles.second);
    std::get<0>(m_modeThreeItemsForSNNumber)->setText(QObject::tr("+SN:"));
    std::get<1>(m_modeThreeItemsForSNNumber)->setPlaceholderText(QObject::tr("Required"));
    std::get<1>(m_modeThreeItemsForSNNumber)->setFixedSize(QSize{275, 25});
    std::get<1>(m_modeThreeItemsForSNNumber)->setStyleSheet(m_lineEditStyles.second);
    std::get<1>(m_modeThreeItemsForSNNumber)->setValidator(m_validatorForSN);
}

void DDesktopRenameDialogPrivate::initUiLayout()
{
    std::get<0>(m_itemsForSelecting)->setBuddy(std::get<1>(m_itemsForSelecting));
    std::get<2>(m_itemsForSelecting)->addWidget(std::get<0>(m_itemsForSelecting));
    std::get<2>(m_itemsForSelecting)->addSpacing(30);
    std::get<2>(m_itemsForSelecting)->addWidget(std::get<1>(m_itemsForSelecting));
    std::get<2>(m_itemsForSelecting)->setMargin(0);


    ///###: mode 1
    std::get<0>(m_modeOneItemsForFinding)->setBuddy(std::get<1>(m_modeOneItemsForFinding));
    std::get<2>(m_modeOneItemsForFinding)->addWidget(std::get<0>(m_modeOneItemsForFinding));
    std::get<2>(m_modeOneItemsForFinding)->addSpacing(30);
    std::get<2>(m_modeOneItemsForFinding)->addWidget(std::get<1>(m_modeOneItemsForFinding));
    std::get<2>(m_modeOneItemsForFinding)->setMargin(0);


    std::get<0>(m_modeOneItemsForReplacing)->setBuddy(std::get<1>(m_modeOneItemsForReplacing));
    std::get<2>(m_modeOneItemsForReplacing)->addWidget(std::get<0>(m_modeOneItemsForReplacing));
    std::get<2>(m_modeOneItemsForReplacing)->addSpacing(30);
    std::get<2>(m_modeOneItemsForReplacing)->addWidget(std::get<1>(m_modeOneItemsForReplacing));


    m_modeOneLayout.first->addLayout(std::get<2>(m_modeOneItemsForFinding));
    m_modeOneLayout.first->addSpacing(20);
    m_modeOneLayout.first->addLayout(std::get<2>(m_modeOneItemsForReplacing));
    m_modeOneLayout.first->setSpacing(0);
    m_modeOneLayout.first->setMargin(0);
    m_modeOneLayout.second->setLayout(m_modeOneLayout.first);




    ///###: mode 2
    std::get<0>(m_modeTwoItemsForAdding)->setBuddy(std::get<1>(m_modeTwoItemsForAdding));
    std::get<2>(m_modeTwoItemsForAdding)->addWidget(std::get<0>(m_modeTwoItemsForAdding));
    std::get<2>(m_modeTwoItemsForAdding)->addSpacing(30);
    std::get<2>(m_modeTwoItemsForAdding)->addWidget(std::get<1>(m_modeTwoItemsForAdding));
    std::get<2>(m_modeTwoItemsForAdding)->setMargin(0);


    std::get<0>(m_modeTwoItemsForLocating)->setBuddy(std::get<1>(m_modeTwoItemsForLocating));
    std::get<2>(m_modeTwoItemsForLocating)->addWidget(std::get<0>(m_modeTwoItemsForLocating));
    std::get<2>(m_modeTwoItemsForLocating)->addWidget(std::get<1>(m_modeTwoItemsForLocating));


    m_modeTwoLayout.first->addLayout(std::get<2>(m_modeTwoItemsForAdding));
    m_modeTwoLayout.first->addSpacing(20);
    m_modeTwoLayout.first->addLayout(std::get<2>(m_modeTwoItemsForLocating));
    m_modeTwoLayout.first->setSpacing(0);
    m_modeTwoLayout.first->setMargin(0);
    m_modeTwoLayout.second->setLayout(m_modeTwoLayout.first);


    ///###: mode 3
    std::get<0>(m_modeThreeItemsForFileName)->setBuddy(std::get<1>(m_modeThreeItemsForFileName));
    std::get<2>(m_modeThreeItemsForFileName)->addWidget(std::get<0>(m_modeThreeItemsForFileName));
    std::get<2>(m_modeThreeItemsForFileName)->addWidget(std::get<1>(m_modeThreeItemsForFileName));

    std::get<0>(m_modeThreeItemsForSNNumber)->setBuddy(std::get<1>(m_modeThreeItemsForSNNumber));
    std::get<2>(m_modeThreeItemsForSNNumber)->addWidget(std::get<0>(m_modeThreeItemsForSNNumber));
    std::get<2>(m_modeThreeItemsForSNNumber)->addWidget(std::get<1>(m_modeThreeItemsForSNNumber));


    m_modeThreeLayout.first->addLayout(std::get<2>(m_modeThreeItemsForFileName));
    m_modeThreeLayout.first->addSpacing(20);
    m_modeThreeLayout.first->addLayout(std::get<2>(m_modeThreeItemsForSNNumber));
    m_modeThreeLayout.first->setSpacing(0);
    m_modeThreeLayout.first->setMargin(0);
    m_modeThreeLayout.second->setLayout(m_modeThreeLayout.first);

    ///###: total layout.
    m_stackedLayout->addWidget(m_modeOneLayout.second);
    m_stackedLayout->addWidget(m_modeTwoLayout.second);
    m_stackedLayout->addWidget(m_modeThreeLayout.second);
    m_stackedLayout->setCurrentIndex(0);


    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addSpacing(25);
    m_mainLayout->addLayout(std::get<2>(m_itemsForSelecting));
    m_mainLayout->addSpacing(20);
    m_mainLayout->addLayout(m_stackedLayout);

    m_mainFrame->setLayout(m_mainLayout);
}



DDesktopRenameDialog::DDesktopRenameDialog(QWidget* const parent)
                  :DDialog{ parent },
                   d_ptr{ new DDesktopRenameDialogPrivate{ this } }
{
    this->initUi();
    this->initConnect();
}


///###: I initialize buttons here.
///   and other widgets in DDesktopRenameDialog is initialized when new DDesktopRenameDialogPrivate.
void DDesktopRenameDialog::initUi()
{
    DDesktopRenameDialogPrivate* const d{ d_func() };
    this->addContent(d->m_mainFrame, Qt::AlignCenter);
    this->addButtons(QList<QString>{ QObject::tr("Cancel"), QObject::tr("Rename") });

    this->setRenameButtonStatus(false);

}


void DDesktopRenameDialog::initConnect()noexcept
{
    DDesktopRenameDialogPrivate* const d{ d_func() };
    using funcType = void (QComboBox::*)(int index);


    QObject::connect(std::get<1>(d->m_itemsForSelecting), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &DDesktopRenameDialog::onCurrentModeChanged);
    QObject::connect(std::get<1>(d->m_modeTwoItemsForLocating), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &DDesktopRenameDialog::onCurrentAddModeChanged);
    QObject::connect(std::get<1>(d->m_modeOneItemsForFinding), &QLineEdit::textChanged, this, &DDesktopRenameDialog::onContentChangedForFinding);
    QObject::connect(std::get<1>(d->m_modeOneItemsForReplacing), &QLineEdit::textChanged, this, &DDesktopRenameDialog::onContentChangedForReplacing);
    QObject::connect(std::get<1>(d->m_modeTwoItemsForAdding), &QLineEdit::textChanged, this, &DDesktopRenameDialog::onContentChangedForAdding);
    QObject::connect(std::get<1>(d->m_modeThreeItemsForFileName), &QLineEdit::textChanged, this, &DDesktopRenameDialog::onContentChangedForCustomizing);
    QObject::connect(std::get<1>(d->m_modeThreeItemsForSNNumber), &QLineEdit::textChanged, this, &DDesktopRenameDialog::onSNNumberChanged);
}


void DDesktopRenameDialog::onCurrentModeChanged(const std::size_t &index)noexcept
{
    DDesktopRenameDialogPrivate* const d{ d_func() };

    d->m_currentmode = index;
    d->m_stackedLayout->setCurrentIndex(index);

    this->setRenameButtonStatus(d->m_currentEnabled[index]);//###: here, we change the rename-button's style sheet and enable button.
}

void DDesktopRenameDialog::onCurrentAddModeChanged(const std::size_t &index)noexcept
{
    DDesktopRenameDialogPrivate* d{ d_func() };

    if(index == 0){
        d->m_flagForAdding = DFileService::AddTextFlags::Before;

    }else{
        d->m_flagForAdding = DFileService::AddTextFlags::After;
    }
}

std::size_t DDesktopRenameDialog::getCurrentModeIndex()const noexcept
{
    const DDesktopRenameDialogPrivate* const d{ d_func() };

    return d->m_currentmode;
}

DFileService::AddTextFlags DDesktopRenameDialog::getAddMode()const noexcept
{
    const DDesktopRenameDialogPrivate* const d{ d_func() };

    return d->m_flagForAdding;
}

QPair<QString, QString> DDesktopRenameDialog::getModeOneContent()const noexcept
{
    const DDesktopRenameDialogPrivate* const d{ d_func() };

    QString findStr{ std::get<1>(d->m_modeOneItemsForFinding)->text() };
    QString replaceStr{ std::get<1>(d->m_modeOneItemsForReplacing)->text() };

    return QPair<QString, QString>{ findStr, replaceStr };
}


QPair<QString, DFileService::AddTextFlags> DDesktopRenameDialog::getModeTwoContent()const noexcept
{
    const DDesktopRenameDialogPrivate* const d{ d_func() };

    QString addStr{ std::get<1>(d->m_modeTwoItemsForAdding)->text() };
    DFileService::AddTextFlags flag{ d->m_flagForAdding };

    return QPair<QString, DFileService::AddTextFlags>{ addStr, flag };
}


QPair<QString, std::size_t> DDesktopRenameDialog::getModeThreeContent()const noexcept
{
    const DDesktopRenameDialogPrivate* const d{ d_func() };

    QString fileName{ std::get<1>(d->m_modeThreeItemsForFileName)->text() };
    std::size_t sn{ std::stoul( std::get<1>(d->m_modeThreeItemsForSNNumber)->text().toStdString() ) };

    return QPair<QString, std::size_t>{ fileName, sn };
}


///###: This function will change enable of property of the button and
///     style sheet.
void DDesktopRenameDialog::setRenameButtonStatus(const bool &enabled)
{
    DDesktopRenameDialogPrivate* const d{ d_func() };

    if(QPushButton* renameButton = dynamic_cast<QPushButton*>(this->getButton(1))){

        renameButton->setEnabled(enabled);

        if(enabled == true){
            renameButton->setStyleSheet(d->m_renameButtonStyles.second);

        }else{
            renameButton->setStyleSheet(d->m_renameButtonStyles.first);
        }
    }
}


void DDesktopRenameDialog::onContentChangedForFinding(const QString &content)
{
    DDesktopRenameDialogPrivate* const d{ d_func() };


    QLineEdit* lineEdit{ std::get<1>(d->m_modeOneItemsForFinding) };

    if(content.isEmpty() == false){
        d->m_currentEnabled[0] = true;
        this->setRenameButtonStatus(true);
        lineEdit->setStyleSheet(d->m_lineEditStyles.first);

    }else{
        d->m_currentEnabled[0] = false;
        this->setRenameButtonStatus(false);
        lineEdit->setStyleSheet(d->m_lineEditStyles.second);
    }
}

void DDesktopRenameDialog::onContentChangedForReplacing(const QString &content)noexcept
{
    DDesktopRenameDialogPrivate* const d{ d_func() };

    if(content.isEmpty() == false){
        std::get<1>(d->m_modeOneItemsForReplacing)->setStyleSheet(d->m_lineEditStyles.first);

    }else{
        std::get<1>(d->m_modeOneItemsForReplacing)->setStyleSheet(d->m_lineEditStyles.second);
    }
}


void DDesktopRenameDialog::onContentChangedForAdding(const QString &content)
{
    DDesktopRenameDialogPrivate* const d{ d_func() };

    QLineEdit* lineEdit{ std::get<1>(d->m_modeTwoItemsForAdding) };

    if(content.isEmpty() == false){
        d->m_currentEnabled[1] = true;
        this->setRenameButtonStatus(true);

        lineEdit->setStyleSheet(d->m_lineEditStyles.first);

    }else{
        d->m_currentEnabled[1] = false;
        this->setRenameButtonStatus(false);

        lineEdit->setStyleSheet(d->m_lineEditStyles.second);
    }
}


void DDesktopRenameDialog::onContentChangedForCustomizing(const QString &content)
{
    DDesktopRenameDialogPrivate* const d{ d_func() };

    QLineEdit* lineEdit{ std::get<1>(d->m_modeThreeItemsForFileName) };

    if(content.isEmpty() == false && lineEdit->text().isEmpty() == false){
        d->m_currentEnabled[2] = true;
        this->setRenameButtonStatus(true);

        lineEdit->setStyleSheet(d->m_lineEditStyles.first);

    }else{
        d->m_currentEnabled[2] = false;
        this->setRenameButtonStatus(false);


        lineEdit->setStyleSheet(d->m_lineEditStyles.second);
    }
}


void DDesktopRenameDialog::onSNNumberChanged(const QString &content)
{
    DDesktopRenameDialogPrivate* d{ d_func() };

    QLineEdit* lineEdit{ std::get<1>(d->m_modeThreeItemsForSNNumber) };

    if(content.isEmpty() == false && lineEdit->text().isEmpty() == false){
        d->m_currentEnabled[2] = true;
        this->setRenameButtonStatus(true);

        lineEdit->setStyleSheet(d->m_lineEditStyles.first);
    }else{
        d->m_currentEnabled[2] = false;
        this->setRenameButtonStatus(false);

        lineEdit->setStyleSheet(d->m_lineEditStyles.second);
    }
}


void DDesktopRenameDialog::setDialogTitle(const QString &tile)noexcept
{
    DDesktopRenameDialogPrivate* const d{ d_func() };
    d->m_titleLabel->setText(tile);
}



