#include "drenamebar.h"
#include "windowmanager.h"
#include "controllers/appcontroller.h"
#include "views/dfilemanagerwindow.h"
#include "interfaces/dfileservices.h"

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





template<typename Type, typename ...Types>
RecordRenameBarState::RecordRenameBarState(Type &&arg, Types&&... args)
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
                     :m_patternFirst{ other.m_patternFirst },
                      m_patternSecond{ other.m_patternSecond },
                      m_patternThird{ other.m_patternThird },
                      m_buttonStateInThreePattern{ other.m_buttonStateInThreePattern },
                      m_currentPattern{ other.m_currentPattern },
                      m_selectedUrls{ other.m_selectedUrls }
{
    m_visible.store(other.m_visible.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
}


RecordRenameBarState::RecordRenameBarState(RecordRenameBarState&& other)
                     :m_patternFirst{ std::move(other.m_patternFirst) },
                      m_patternSecond{ std::move(other.m_patternSecond) },
                      m_patternThird{ std::move(other.m_patternThird) },
                      m_buttonStateInThreePattern{ std::move(other.m_buttonStateInThreePattern) },
                      m_currentPattern{ std::move(other.m_currentPattern) },
                      m_selectedUrls{ std::move(other.m_selectedUrls) }
{
    m_visible.store(other.m_visible.load(std::memory_order_seq_cst), std::memory_order_seq_cst);
}



RecordRenameBarState& RecordRenameBarState::operator=(const RecordRenameBarState& other)
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




RecordRenameBarState& RecordRenameBarState::operator=(RecordRenameBarState&& other)
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
    m_buttonStateInThreePattern = std::array<bool, 3>{false, false, false};
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
    DRenameBarPrivate(DRenameBar * const qPtr);
    ~DRenameBarPrivate()=default;

    ///###: in fact, you can not also std::move too.
    DRenameBarPrivate(const DRenameBarPrivate&)=delete;
    DRenameBarPrivate& operator=(DRenameBarPrivate&&)=delete;


    void initUi();
    void setUiParameters();
    void layoutItems()noexcept;


    DRenameBar* q_ptr{ nullptr };
    QHBoxLayout* m_mainLayout{ nullptr };
    QComboBox* m_comboBox{ nullptr };
    QFrame* m_frame{ nullptr };
    QStackedWidget* m_stackWidget{ nullptr };
    QValidator* m_validator{ nullptr };
    QArray<3> m_renameButtonStates{false};                //###: this is a array for recording the state of rename button in current pattern.
    QPair<QString, QString> m_lineEditStyleSheets{};//###: this includes normal state and highlight state for QLineEdit.
    std::size_t m_currentPattern{0};               //###: this number record current pattern.
    QList<DUrl> m_urlList{};                      //###: this list stores the url of file which is waiting to be modified!
    DFileService::AddTextFlags m_flag{ DFileService::AddTextFlags::Before }; //###: the loacation of text should be added.

    QTuple<QLabel*, QLineEdit*, QLabel*, QLineEdit*> m_replaceOperatorItems{};
    QPair<QHBoxLayout*, QFrame*> m_frameForLayoutReplaceArea{};

    QTuple<QLabel*, QLineEdit*, QLabel*, QComboBox*> m_addOperatorItems{};
    QPair<QHBoxLayout*, QFrame*> m_frameForLayoutAddArea{};


    QTuple<QLabel*, QLineEdit*, QLabel*, QLineEdit*, QLabel*> m_customOPeratorItems{};
    QPair<QHBoxLayout*, QFrame*> m_frameForLayoutCustomArea{};


    QTuple<QPushButton*, QPushButton*, QHBoxLayout*, QFrame*> m_buttonsArea{};



public slots:
    void onRenamePatternChanged(const int& index)noexcept;
};


DRenameBarPrivate::DRenameBarPrivate(DRenameBar * const qPtr)
                  :q_ptr{ qPtr }
{
    this->initUi();
    this->setUiParameters();
    this->layoutItems();
}

void DRenameBarPrivate::initUi()
{
    m_mainLayout = new QHBoxLayout{ q_ptr };
    m_comboBox = new QComboBox;
    m_frame = new QFrame;
    m_stackWidget = new QStackedWidget;


    QRegExp regx{ "[0-9]{1,8}" }; //limit the value for SN, make value of QLineEdit be number.
    m_validator = new QRegExpValidator{regx, q_ptr};

    m_lineEditStyleSheets = QPair<QString, QString>{ QString{""}, QString{"border: 1px solid blue; border-radius: 3px;"} };

    m_replaceOperatorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QLineEdit);
    m_frameForLayoutReplaceArea = QPair<QHBoxLayout*, QFrame*>{ new QHBoxLayout, new QFrame };

    m_addOperatorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QComboBox);
    m_frameForLayoutAddArea = QPair<QHBoxLayout*, QFrame*>{ new QHBoxLayout, new QFrame };

    m_customOPeratorItems = std::make_tuple(new QLabel, new QLineEdit, new QLabel, new QLineEdit, new QLabel);
    m_frameForLayoutCustomArea = QPair<QHBoxLayout*, QFrame*>{ new QHBoxLayout, new QFrame };

    m_buttonsArea = std::make_tuple(new QPushButton, new QPushButton, new QHBoxLayout, new QFrame);
}


void DRenameBarPrivate::setUiParameters()
{
    q_ptr->setFixedHeight(38);

    m_comboBox->addItems(QList<QString>{ QObject::tr("Replace Text"), QObject::tr("Add Text"), QObject::tr("Custom Text") });

    QComboBox* comboBox{ nullptr };
    QLabel* label{ std::get<0>(m_replaceOperatorItems) };
    QLineEdit* lineEdit{ std::get<1>(m_replaceOperatorItems) };

    label->setText(QObject::tr("Find"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    lineEdit->setStyleSheet(m_lineEditStyleSheets.second);
    label->setBuddy(lineEdit);

    label = std::get<2>(m_replaceOperatorItems);
    lineEdit = std::get<3>(m_replaceOperatorItems);
    label->setText(QObject::tr("Replace"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    label->setBuddy(lineEdit);


    label = std::get<0>(m_addOperatorItems);
    lineEdit = std::get<1>(m_addOperatorItems);
    label->setText(QObject::tr("Add"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    lineEdit->setStyleSheet(m_lineEditStyleSheets.second);
    label->setBuddy(lineEdit);


    label = std::get<2>(m_addOperatorItems);
    comboBox = std::get<3>(m_addOperatorItems);
    label->setText(QObject::tr("Location"));
    comboBox->addItems(QList<QString>{ QObject::tr("Before file name"), QObject::tr("After file name") });
    label->setBuddy(comboBox);

    label = std::get<0>(m_customOPeratorItems);
    lineEdit = std::get<1>(m_customOPeratorItems);
    label->setText(QObject::tr("File name"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    lineEdit->setStyleSheet(m_lineEditStyleSheets.second);
    label->setBuddy(lineEdit);


    label = std::get<2>(m_customOPeratorItems);
    lineEdit = std::get<3>(m_customOPeratorItems);
    label->setText(QObject::tr("SN"));
    lineEdit->setPlaceholderText(QObject::tr("Required"));
    lineEdit->setStyleSheet(m_lineEditStyleSheets.second);
    lineEdit->setValidator(m_validator);
    label->setBuddy(lineEdit);
    label = std::get<4>(m_customOPeratorItems);
    label->setText(QObject::tr("Tips: Sort by selected file order"));


    QPushButton* button{ std::get<0>(m_buttonsArea) };
    button->setText(QObject::tr("Cancel"));
    button = std::get<1>(m_buttonsArea);
    button->setText(QObject::tr("Rename"));
    button->setEnabled(false);

}

void DRenameBarPrivate::layoutItems()noexcept
{
    QHBoxLayout* hBoxLayout{ nullptr };
    QFrame* frame{ nullptr };

    hBoxLayout = m_frameForLayoutReplaceArea.first;
    frame = m_frameForLayoutReplaceArea.second;
    hBoxLayout->addWidget(std::get<0>(m_replaceOperatorItems));
    hBoxLayout->addWidget(std::get<1>(m_replaceOperatorItems));
    hBoxLayout->addSpacing(50);
    hBoxLayout->addWidget(std::get<2>(m_replaceOperatorItems));
    hBoxLayout->addWidget(std::get<3>(m_replaceOperatorItems));
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hBoxLayout);
    m_stackWidget->addWidget(frame);


    hBoxLayout = m_frameForLayoutAddArea.first;
    frame = m_frameForLayoutAddArea.second;
    hBoxLayout->addWidget(std::get<0>(m_addOperatorItems));
    hBoxLayout->addWidget(std::get<1>(m_addOperatorItems));
    hBoxLayout->addSpacing(50);
    hBoxLayout->addWidget(std::get<2>(m_addOperatorItems));
    hBoxLayout->addWidget(std::get<3>(m_addOperatorItems));
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hBoxLayout);
    m_stackWidget->addWidget(frame);


    hBoxLayout = m_frameForLayoutCustomArea.first;
    frame = m_frameForLayoutCustomArea.second;
    hBoxLayout->addWidget(std::get<0>(m_customOPeratorItems));
    hBoxLayout->addWidget(std::get<1>(m_customOPeratorItems));
    hBoxLayout->addSpacing(40);
    hBoxLayout->addWidget(std::get<2>(m_customOPeratorItems));
    hBoxLayout->addWidget(std::get<3>(m_customOPeratorItems));
    hBoxLayout->addWidget(std::get<4>(m_customOPeratorItems));
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hBoxLayout);
    m_stackWidget->addWidget(frame);


    hBoxLayout = std::get<2>(m_buttonsArea);
    frame = std::get<3>(m_buttonsArea); //###: !!!
    hBoxLayout->addWidget(std::get<0>(m_buttonsArea));
    hBoxLayout->addWidget(std::get<1>(m_buttonsArea));
    hBoxLayout->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(hBoxLayout);

    m_mainLayout->addWidget(m_comboBox);
    m_mainLayout->addWidget(m_stackWidget);
    m_mainLayout->addStretch(0);
    m_mainLayout->addWidget(frame);//###: !!!
//    m_mainLayout->setContentsMargins(0,0,0,0); //do not open this.
//    m_mainLayout->setSpacing(0);
    m_stackWidget->setCurrentIndex(0);


    q_ptr->setLayout(m_mainLayout);
}


DRenameBar::DRenameBar(QWidget *parent)
        : QFrame{parent},
          d_ptr{ new DRenameBarPrivate{this} }
{
    this->initConnect();
    this->setVisible(false);
}


void DRenameBar::onRenamePatternChanged(const int &index)noexcept
{
    DRenameBarPrivate* const d{ d_func() };

    d->m_currentPattern = index;

    bool state{ d->m_renameButtonStates[index] }; //###: we get the state value of button in current pattern.
    d->m_stackWidget->setCurrentIndex(index);
    std::get<1>(d->m_buttonsArea)->setEnabled(state);
}


void DRenameBar::onReplaceOperatorFileNameChanged(const QString &text)noexcept
{
    DRenameBarPrivate* const d{ d_func() };

    if(text.isEmpty()){
         std::get<1>(d->m_buttonsArea)->setEnabled(false);
         d->m_renameButtonStates[0] = false; //###: record the states of rename button
         std::get<1>(d->m_replaceOperatorItems)->setStyleSheet(d->m_lineEditStyleSheets.second);//###: highlight
         return;
    }

    std::get<1>(d->m_buttonsArea)->setEnabled(true);
    d->m_renameButtonStates[0] = true;   //###: record the states of rename button
    std::get<1>(d->m_replaceOperatorItems)->setStyleSheet(d->m_lineEditStyleSheets.first);//###: normal
    return;
}


void DRenameBar::onAddOperatorAddedContentChanged(const QString &text)noexcept
{
    DRenameBarPrivate* const d{ d_func() };

    if(text.isEmpty()){
        std::get<1>(d->m_buttonsArea)->setEnabled(false);
        d->m_renameButtonStates[1] = false;
        std::get<1>(d->m_addOperatorItems)->setStyleSheet(d->m_lineEditStyleSheets.second);
        return;
    }

    std::get<1>(d->m_buttonsArea)->setEnabled(true);
    d->m_renameButtonStates[1] = true;
    std::get<1>(d->m_addOperatorItems)->setStyleSheet(d->m_lineEditStyleSheets.first);
    return;
}


void DRenameBar::eventDispatcher()
{
    DRenameBarPrivate* const d { d_func() };
    QSharedMap<DUrl, DUrl> pendingUrls{ nullptr };

    bool value{ false };

    if(d->m_currentPattern == 0){ //###: replace
        QString forFindingStr{ std::get<1>(d->m_replaceOperatorItems)->text() };
        QString forReplaceStr{ std::get<3>(d->m_replaceOperatorItems)->text() };

        QPair<QString, QString> pair{forFindingStr, forReplaceStr};

        value = DFileService::instance()->multiFilesReplaceName(d->m_urlList, pair);

    }else if(d->m_currentPattern == 1){ //###: add
        QString forAddingStr{ std::get<1>(d->m_addOperatorItems)->text() };


        QPair<QString, DFileService::AddTextFlags> pair{ forAddingStr, d->m_flag };

        value = DFileService::instance()->multiFilesAddStrToName(d->m_urlList, pair);

    }else if(d->m_currentPattern == 2){ //###:  custom
        QString forCustomStr{ std::get<1>(d->m_customOPeratorItems)->text() };
        std::size_t number{ std::get<3>(d->m_customOPeratorItems)->text().toInt() };

        QPair<QString, std::size_t> pair{ forCustomStr, number};

        value = DFileService::instance()->multiFilesCustomName(d->m_urlList, pair);
    }

    if(value == true){

        if(QWidget* const parent = dynamic_cast<QWidget*>(this->parent())){
            quint64 windowId{ WindowManager::getWindowId(parent) };
            AppController::multiSelectionFilesCache.second = windowId;
        }
    }

    this->hide();
    this->restoreRenameBar();
}

void DRenameBar::onAddTextPatternChanged(const int &index)noexcept
{
    DRenameBarPrivate* const d{ d_func() };

    if(index == 0){ //###: add text to the front of filename.
        d->m_flag = DFileService::AddTextFlags::Before;
    }else{        //###: add text to the back of filename.
        d->m_flag = DFileService::AddTextFlags::After;
    }

}


///###: this function is special.
///           it connect two QLineEdits,
void DRenameBar::onCustomOperatorFileNameAndSNChanged()noexcept
{
    DRenameBarPrivate* const d{ d_func() };

    QLineEdit* lineEdit1{ std::get<1>(d->m_customOPeratorItems) };
    QLineEdit* lineEdit2{ std::get<3>(d->m_customOPeratorItems) };

    if(lineEdit1->text().isEmpty() == false  &&  lineEdit2->text().isEmpty() == false){ //###: must be input filename and SN in thrid pattern.
        std::get<1>(d->m_buttonsArea)->setEnabled(true);
        d->m_renameButtonStates[2] = true;

        lineEdit1->setStyleSheet(d->m_lineEditStyleSheets.first);
        lineEdit2->setStyleSheet(d->m_lineEditStyleSheets.first);


    }else{
        std::get<1>(d->m_buttonsArea)->setEnabled(false);
        d->m_renameButtonStates[2] = false;

        lineEdit1->setStyleSheet(d->m_lineEditStyleSheets.second);
        lineEdit2->setStyleSheet(d->m_lineEditStyleSheets.second);
    }

}



void DRenameBar::storeUrlList(const QList<DUrl> &list)noexcept
{
    DRenameBarPrivate* const d{ d_func() };
    d->m_urlList = list;
}




void DRenameBar::initConnect()
{
    DRenameBarPrivate* const d{ d_func() };
    using funcType = void (QComboBox::*)(int index);


    QObject::connect(d->m_comboBox, static_cast<funcType>(&QComboBox::activated), this, &DRenameBar::onRenamePatternChanged);

    QObject::connect(std::get<0>(d->m_buttonsArea), &QPushButton::clicked, this, &DRenameBar::onClickCancelButton);
    QObject::connect(std::get<1>(d->m_replaceOperatorItems), &QLineEdit::textChanged, this, &DRenameBar::onReplaceOperatorFileNameChanged);
    QObject::connect(std::get<1>(d->m_addOperatorItems), &QLineEdit::textChanged, this, &DRenameBar::onAddOperatorAddedContentChanged);

    QObject::connect(std::get<1>(d->m_buttonsArea), &QPushButton::clicked, this, &DRenameBar::eventDispatcher);
    QObject::connect(std::get<3>(d->m_addOperatorItems), static_cast<funcType>(&QComboBox::currentIndexChanged), this, &DRenameBar::onAddTextPatternChanged);


    QObject::connect(std::get<1>(d->m_customOPeratorItems), &QLineEdit::textChanged, this, &DRenameBar::onCustomOperatorFileNameAndSNChanged);
    QObject::connect(std::get<3>(d->m_customOPeratorItems), &QLineEdit::textChanged, this,  &DRenameBar::onCustomOperatorFileNameAndSNChanged);
}


void DRenameBar::restoreRenameBar()noexcept
{
    DRenameBarPrivate* const d{ d_func() };

    ///replace
    QLineEdit* lineEdit{ std::get<1>(d->m_replaceOperatorItems) };
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
    lineEdit->clear();

    d->m_flag = DFileService::AddTextFlags::Before;
    d->m_currentPattern = 0;
    d->m_renameButtonStates = std::array<bool, 3>{false, false, false};

    d->m_comboBox->setCurrentIndex(0);
    d->m_stackWidget->setCurrentIndex(0);
    std::get<3>(d->m_addOperatorItems)->setCurrentIndex(0);
}

std::unique_ptr<RecordRenameBarState> DRenameBar::getCurrentState()const
{
    const DRenameBarPrivate* const d{ d_func() };
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
                                                  new RecordRenameBarState{ patternOneContent, patternTwoContent, patternThreeContent,
                                                                            buttonsState, currentPattern, selectedUrls, visibleValue}
                                               };
    return state;
}

void DRenameBar::loadState(std::unique_ptr<RecordRenameBarState>& state)
{
    DRenameBarPrivate* const d{ d_func() };

    if(static_cast<bool>(state) == true){

        QPair<QString, QString> patternOneContent{ state->getPatternFirstItemContent() };
        std::get<1>(d->m_replaceOperatorItems)->setText(patternOneContent.first);
        std::get<3>(d->m_replaceOperatorItems)->setText(patternOneContent.second);

        QPair<QString, DFileService::AddTextFlags> patternTwoContent{ state->getPatternSecondItemContent() };
        std::get<1>(d->m_addOperatorItems)->setText(patternTwoContent.first);
        d->m_flag = patternTwoContent.second;
        std::get<3>(d->m_addOperatorItems)->setCurrentIndex(static_cast<std::size_t>(patternTwoContent.second));

        QPair<QString, QString> patternThreeContent{ state->getPatternThirdItemContent() };
        std::get<1>(d->m_customOPeratorItems)->setText(patternThreeContent.first);
        std::get<3>(d->m_customOPeratorItems)->setText(patternThreeContent.second);

        d->m_renameButtonStates = state->getbuttonStateInThreePattern();
        d->m_currentPattern = state->getCurrentPattern();
        d->m_stackWidget->setCurrentIndex(state->getCurrentPattern());
        std::get<1>(d->m_buttonsArea)->setEnabled(d->m_renameButtonStates[d->m_currentPattern]);

        d->m_urlList = state->getSelectedUrl();
        this->setVisible(state->getVisibleValue());
    }
}



