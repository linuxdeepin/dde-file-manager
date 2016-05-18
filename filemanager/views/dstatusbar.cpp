#include "dstatusbar.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"
#include "../app/global.h"
#include "windowmanager.h"

DStatusBar::DStatusBar(QWidget *parent)
    : QFrame(parent)
{
    m_layout = new QHBoxLayout(this);
    m_label = new QLabel(tr("%1 item selected").arg("0"), this);
    m_layout->addWidget(m_label, 1, Qt::AlignCenter);
    m_layout->setContentsMargins(0,0,0,0);
    setFixedHeight(25);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet("QFrame{\
                  background-color: white;\
              }");
    setLayout(m_layout);
    connect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, this, &DStatusBar::itemSelected);
}

void DStatusBar::itemSelected(const FMEvent &event, int number)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;

    if(number > 1)
    {
        m_label->setText(tr("%1 items selected").arg(QString::number(number)));
    }
    else
    {
        m_label->setText(tr("%1 item selected").arg(QString::number(number)));
    }
}
