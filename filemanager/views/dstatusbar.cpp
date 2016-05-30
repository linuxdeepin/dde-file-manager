#include "dstatusbar.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"
#include "../app/global.h"
#include "windowmanager.h"

DStatusBar::DStatusBar(QWidget *parent)
    : QFrame(parent)
{
    m_counted = tr("%1 items");
    m_selected = tr("%1 items selected");
    m_layout = new QHBoxLayout(this);
    m_label = new QLabel(m_counted.arg("0"), this);
    m_layout->addWidget(m_label, 1, Qt::AlignCenter);
    m_layout->setContentsMargins(0,0,0,0);
    setFixedHeight(25);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet("QFrame{\
                  background-color: white;\
              }");
    setLayout(m_layout);
    connect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, this, &DStatusBar::itemSelected);
    connect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, this, &DStatusBar::itemCounted);
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
