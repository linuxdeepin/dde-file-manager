#include "dstatusbar.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"
#include "../app/global.h"
#include "windowmanager.h"


DStatusBar::DStatusBar(QWidget *parent)
    : QFrame(parent)
{
    initUI();
    initConnect();
}

void DStatusBar::initUI()
{
    m_counted = tr("%1 items");
    m_selected = tr("%1 items selected");
    m_layout = new QHBoxLayout(this);

    QStringList seq;
    for (int i(1); i != 91; ++i)
        seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));

    m_loadingIndicator = new DPictureSequenceView(this);
    m_loadingIndicator->setPictureSequence(seq);
    m_loadingIndicator->setFixedSize(32, 32);
    m_loadingIndicator->setSpeed(20);
    m_loadingIndicator->hide();

    m_label = new QLabel(m_counted.arg("0"), this);
    m_layout->addStretch();
    m_layout->addWidget(m_loadingIndicator);
    m_layout->addWidget(m_label);
    m_layout->addStretch();
    m_layout->setSpacing(5);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setFixedHeight(32);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet("QFrame{\
                  background-color: white;\
                  color: #797979;\
              }");
    setLayout(m_layout);

    m_sizeGrip = new QSizeGrip(this);
}

void DStatusBar::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, this, &DStatusBar::itemSelected);
    connect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, this, &DStatusBar::itemCounted);
    connect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, this, &DStatusBar::showLoadingIncator);
}

void DStatusBar::itemSelected(const FMEvent &event, int number)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;

    if(number > 1)
    {
        m_label->setText(m_selected.arg(QString::number(number)));
    }
    else
    {
        m_label->setText(m_selected.arg(QString::number(number)));
    }
}

void DStatusBar::itemCounted(const FMEvent &event, int number)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;

    if(number > 1)
    {
        m_label->setText(m_counted.arg(QString::number(number)));
    }
    else
    {
        m_label->setText(m_counted.arg(QString::number(number)));
    }
}

void DStatusBar::showLoadingIncator(const FMEvent &event, bool loading)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;
    m_loadingIndicator->setVisible(loading);
    if (loading)
        m_label->setText(tr("Loading..."));
}

void DStatusBar::resizeEvent(QResizeEvent *event)
{
    m_loadingIndicator->move((event->size().width() - m_loadingIndicator->width()) / 2, 0);
    m_sizeGrip->move(event->size().width() - 10, event->size().height() - 10);
    QFrame::resizeEvent(event);
}
