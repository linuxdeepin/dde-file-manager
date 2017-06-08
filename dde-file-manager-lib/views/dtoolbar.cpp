#include "dtoolbar.h"
#include "dicontextbutton.h"
#include "dcheckablebutton.h"
#include "dsearchbar.h"
#include "dcrumbwidget.h"
#include "historystack.h"
#include "dhoverbutton.h"
#include "historystack.h"
#include "dhoverbutton.h"
#include "windowmanager.h"
#include "dfileservices.h"
#include "dfmeventdispatcher.h"

#include "dfmevent.h"
#include "app/define.h"
#include "app/filesignalmanager.h"

#include "dfilemenumanager.h"

#include "singleton.h"
#include "views/dfileview.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmactionbutton.h"

#include <DGraphicsClipEffect>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QToolButton>

const int DToolBar::ButtonWidth = 24;
const int DToolBar::ButtonHeight = 24;

DToolBar::DToolBar(QWidget *parent) : QFrame(parent)
{
    initData();
    initUI();
    initConnect();
}

DToolBar::~DToolBar()
{
    delete m_navStack;
}

void DToolBar::initData()
{
//    m_navStack = new HistoryStack(65536);
}

void DToolBar::initUI()
{
    setFocusPolicy(Qt::NoFocus);
    initAddressToolBar();
    initContollerToolBar();

    m_settingsButton = new QPushButton(this);
    m_settingsButton->setFixedWidth(ButtonWidth);
    m_settingsButton->setFixedHeight(ButtonHeight);
    m_settingsButton->setObjectName("settingsButton");
    m_settingsButton->setCheckable(true);
    m_settingsButton->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_addressToolBar);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_contollerToolBar);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_settingsButton);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(14, 0, 14, 0);
    setLayout(mainLayout);
}

void DToolBar::initAddressToolBar()
{
    m_addressToolBar = new QFrame;
    m_addressToolBar->setObjectName("AddressToolBar");
    m_addressToolBar->setFixedHeight(40);

    QHBoxLayout * backForwardLayout = new QHBoxLayout;


    m_backButton = new QPushButton(this);
    m_backButton->setObjectName("backButton");
    m_backButton->setFixedWidth(ButtonWidth);
    m_backButton->setFixedHeight(ButtonHeight);
    m_backButton->setDisabled(true);
    m_backButton->setFocusPolicy(Qt::NoFocus);
    m_forwardButton = new QPushButton(this);
    m_forwardButton->setObjectName("forwardButton");
    m_forwardButton->setFixedWidth(ButtonWidth);
    m_forwardButton->setFixedHeight(ButtonHeight);
    m_forwardButton->setDisabled(true);
    m_forwardButton->setFocusPolicy(Qt::NoFocus);

    m_searchButton = new QPushButton(this);
    m_searchButton->setObjectName("searchButton");
    m_searchButton->setFixedWidth(ButtonWidth);
    m_searchButton->setFixedHeight(ButtonHeight);
    m_searchButton->setFocusPolicy(Qt::NoFocus);

    backForwardLayout->addWidget(m_backButton);
    backForwardLayout->addWidget(m_forwardButton);
    backForwardLayout->setSpacing(0);
    backForwardLayout->setContentsMargins(0, 0, 0, 0);


    QFrame * crumbAndSearch = new QFrame;
    m_searchBar = new DSearchBar(this);
    m_searchBar->hide();
    m_searchBar->setAlignment(Qt::AlignHCenter);
    m_crumbWidget = new DCrumbWidget(this);
    crumbAndSearch->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QHBoxLayout * comboLayout = new QHBoxLayout;
    comboLayout->addWidget(m_crumbWidget);
    comboLayout->addWidget(m_searchBar);
    comboLayout->addWidget(m_searchButton);
    comboLayout->setSpacing(10);
    comboLayout->setContentsMargins(0, 0, 0, 0);

    crumbAndSearch->setLayout(comboLayout);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(backForwardLayout);
    mainLayout->addWidget(crumbAndSearch);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    m_addressToolBar->setLayout(mainLayout);
}

void DToolBar::initContollerToolBar()
{
    m_contollerToolBar = new QFrame;
    m_contollerToolBar->setObjectName("ContollerToolBar");
    m_contollerToolBar->setFrameShape(QFrame::NoFrame);
    m_contollerToolBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QWidget *content = new QWidget();

    m_contollerToolBarClipMask = new DGraphicsClipEffect(content);
    content->setGraphicsEffect(m_contollerToolBarClipMask);

    m_contollerToolBarContentLayout = new QHBoxLayout;
    m_contollerToolBarContentLayout->setContentsMargins(1, 1, 1, 1);
    m_contollerToolBarContentLayout->setSpacing(0);

    content->setLayout(m_contollerToolBarContentLayout);
    content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(m_contollerToolBar);

    layout->addWidget(content);
    layout->setContentsMargins(0, 0, 0, 0);
}

void DToolBar::initConnect()
{
    connect(m_backButton, &DStateButton::clicked,this, &DToolBar::onBackButtonClicked);
    connect(m_forwardButton, &DStateButton::clicked,this, &DToolBar::onForwardButtonClicked);
    connect(m_searchBar, &DSearchBar::returnPressed, this, &DToolBar::searchBarTextEntered);
    connect(m_crumbWidget, &DCrumbWidget::crumbSelected, this, &DToolBar::crumbSelected);
    connect(m_crumbWidget, &DCrumbWidget::searchBarActivated, this, &DToolBar::searchBarActivated);
    connect(m_searchButton, &DStateButton::clicked, this, &DToolBar::searchBarClicked);
    connect(m_searchBar, &DSearchBar::focusedOut, this,  &DToolBar::searchBarDeactivated);
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged, this, &DToolBar::crumbChanged);
    connect(fileSignalManager, &FileSignalManager::requestSearchCtrlF, this, &DToolBar::handleHotkeyCtrlF);
    connect(fileSignalManager, &FileSignalManager::requestSearchCtrlL, this, &DToolBar::handleHotkeyCtrlL);
}

DSearchBar *DToolBar::getSearchBar()
{
    return m_searchBar;
}

DCrumbWidget *DToolBar::getCrumWidget()
{
    return m_crumbWidget;
}

QPushButton *DToolBar::getSettingsButton()
{
    return m_settingsButton;
}

void DToolBar::searchBarClicked()
{
    searchBarActivated();
    m_searchBar->setText("");
}

void DToolBar::searchBarActivated()
{
    m_searchBar->setPlaceholderText(tr("Search or enter address"));
    m_searchBar->show();
    m_crumbWidget->hide();
    m_searchBar->setAlignment(Qt::AlignLeft);
    m_searchBar->clear();
    m_searchBar->setActive(true);
    m_searchBar->setFocus();
    m_searchBar->setCurrentUrl(m_crumbWidget->getCurrentUrl());
    m_searchButton->hide();
}

void DToolBar::searchBarDeactivated()
{
    int winId = WindowManager::getWindowId(this);
    DFileManagerWindow* window = qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(winId));
    if (window){
        if (window->currentUrl().isSearchFile()){

        }
        else{
            m_searchBar->setPlaceholderText("");
            m_searchBar->hide();
            m_crumbWidget->show();
            m_searchBar->clear();
            m_searchBar->setAlignment(Qt::AlignHCenter);
            m_searchBar->setActive(false);
            m_searchBar->window()->setFocus();
            m_searchButton->show();
        }
    }

    emit fileSignalManager->requestFoucsOnFileView(winId);
}

/**
 * @brief DToolBar::searchBarTextEntered
 *
 * Set the tab bar when return press is detected
 * on search bar.
 */
void DToolBar::searchBarTextEntered()
{
    QString text = m_searchBar->text();

    if (text.isEmpty()) {
        m_searchBar->clearText();
        return;
    }

    const QString &currentDir = QDir::currentPath();
    const DUrl &currentUrl = m_crumbWidget->getCurrentUrl();

    if (currentUrl.isLocalFile())
        QDir::setCurrent(currentUrl.toLocalFile());

    DUrl inputUrl = DUrl::fromUserInput(text, false);

    QDir::setCurrent(currentDir);

    DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, inputUrl, window());
}

void DToolBar::crumbSelected(const DFMEvent &e)
{
    if (e.windowId() != WindowManager::getWindowId(this))
        return;

    DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(m_crumbWidget, e.fileUrl(), window());
}

void DToolBar::crumbChanged(const DFMEvent &event)
{
    if (event.windowId() != WindowManager::getWindowId(this))
        return;

    if (event.sender() == m_crumbWidget)
    {
        checkNavHistory(event.fileUrl());
        return;
    }

    if (event.fileUrl().isSearchFile()){
        m_searchBar->show();
        m_crumbWidget->hide();
        m_searchBar->setAlignment(Qt::AlignLeft);
        m_searchBar->clear();
        m_searchBar->setActive(true);
        m_searchBar->setFocus();
        m_searchBar->setText(event.fileUrl().searchKeyword());
        m_searchBar->getPopupList()->hide();
    }else{
        m_searchBar->hide();
        m_crumbWidget->show();
        setCrumb(event.fileUrl());
    }

    if (event.sender() == this)
        return;

    checkNavHistory(event.fileUrl());
}

/**
 * @brief DToolBar::upButtonClicked
 *
 * Move or shrink to the given index of the tabs depending
 * on the amount of tabs shown. This will be triggered when
 * up button is clicked.
 */

void DToolBar::searchBarChanged(QString path)
{
    m_searchBar->setText(path);
}

void DToolBar::back()
{
    DUrl url = m_navStack->back();

    if(!url.isEmpty())
    {
        updateBackForwardButtonsState();
        DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, url, window());
    }
}

void DToolBar::forward()
{
    DUrl url = m_navStack->forward();

    if(!url.isEmpty())
    {
        updateBackForwardButtonsState();
        DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, url, window());
    }
}

void DToolBar::handleHotkeyCtrlF(quint64 winId)
{
    if (winId == WindowManager::getWindowId(this)) {
        searchBarClicked();
    }
}

void DToolBar::handleHotkeyCtrlL(quint64 winId)
{
    if (winId == WindowManager::getWindowId(this)) {
        searchBarActivated();
    }
}

void DToolBar::checkNavHistory(DUrl url)
{

    if (!m_navStack)
        return;

    m_navStack->append(url);
    updateBackForwardButtonsState();
}

void DToolBar::addHistoryStack(){
    m_navStacks.append(new HistoryStack(65536));
}

void DToolBar::switchHistoryStack(const int index){
    m_navStack = m_navStacks.at(index);
    if(!m_navStack)
        return;
    updateBackForwardButtonsState();
}

void DToolBar::removeNavStackAt(int index){
    m_navStacks.removeAt(index);

    if(index < m_navStacks.count())
        m_navStack = m_navStacks.at(index);
    else
        m_navStack = m_navStacks.at(m_navStacks.count()-1);

    if(!m_navStack)
        return;
    if(m_navStack->size() > 1)
        m_backButton->setEnabled(true);
    else
        m_backButton->setEnabled(false);

    if(m_navStack->isLast())
        m_forwardButton->setEnabled(false);
    else
        m_forwardButton->setEnabled(true);
}

void DToolBar::moveNavStacks(int from, int to){
    m_navStacks.move(from,to);
}

int DToolBar::navStackCount() const{
    return m_navStacks.count();
}

void DToolBar::setCrumb(const DUrl &url)
{
    m_crumbWidget->setCrumb(url);
}

void DToolBar::updateBackForwardButtonsState()
{
    if(m_navStack->size() <= 1){
        m_backButton->setEnabled(false);
        m_forwardButton->setEnabled(false);
    }
    else{
        if(m_navStack->isFirst())
            m_backButton->setEnabled(false);
        else
            m_backButton->setEnabled(true);

        if(m_navStack->isLast())
            m_forwardButton->setEnabled(false);
        else
            m_forwardButton->setEnabled(true);
    }
}

void DToolBar::setCustomActionList(const QList<QAction *> &list)
{
    for (DFMActionButton *button : m_contollerToolBar->findChild<QWidget*>()->findChildren<DFMActionButton*>()) {
        m_contollerToolBarContentLayout->removeWidget(button);
        button->deleteLater();
    }

    for (int i = 0; i < list.count(); ++i) {
        DFMActionButton *button = new DFMActionButton(this);
        button->setFixedSize(ButtonWidth - 2, ButtonHeight - 2);
        button->setFocusPolicy(Qt::NoFocus);
        button->setAction(list.at(i));

        m_contollerToolBarContentLayout->addWidget(button);
        button->show();
    }

    m_contollerToolBar->setHidden(list.isEmpty());

    QPainterPath path;

    path.addRoundedRect(QRectF(QPointF(0, 0), m_contollerToolBar->sizeHint()).adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
    m_contollerToolBarClipMask->setClipPath(path);
}

void DToolBar::onBackButtonClicked()
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMBackEvent>(this), qobject_cast<DFileManagerWindow*>(window()));
}

void DToolBar::onForwardButtonClicked()
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMForwardEvent>(this), qobject_cast<DFileManagerWindow*>(window()));
}
