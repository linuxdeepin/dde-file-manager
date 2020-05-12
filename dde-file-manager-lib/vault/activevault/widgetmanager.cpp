#include "widgetmanager.h"
#include "widgetstartvault.h"
#include "widgetsetunlockmethod.h"
#include "widgetsavekey.h"
#include "widgetactivcevaultfinished.h"

#include <QDebug>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QMouseEvent>


WidgetManager::WidgetManager(QWidget *parent)
    : DDialog(parent)
    , m_pStackedWidget(nullptr)
    , m_pStartVaultWidget(nullptr)
    , m_pSetUnclockMethodWidget(nullptr)
    , m_SaveKeyWidget(nullptr)
    , m_ActiveVaultFinishedWidget(nullptr)
{
    this->setIcon(QIcon::fromTheme("dfm_safebox"));

//    QHBoxLayout *play = new QHBoxLayout(this);
//    play->setMargin(1);
//    play->setSpacing(1);

    // 初始化试图容器
    m_pStackedWidget = new QStackedWidget(this);

    // 初始化内部窗体
    m_pStartVaultWidget = new WidgetStartVault(this);
    connect(m_pStartVaultWidget, &WidgetStartVault::sigAccepted,
            this, &WidgetManager::slotNextWidget);
    m_pSetUnclockMethodWidget = new WidgetSetUnlockMethod(this);
    connect(m_pSetUnclockMethodWidget, &WidgetSetUnlockMethod::sigAccepted,
            this, &WidgetManager::slotNextWidget);
    m_SaveKeyWidget = new WidgetSaveKey(this);
    connect(m_SaveKeyWidget, &WidgetSaveKey::sigAccepted,
            this, &WidgetManager::slotNextWidget);
    m_ActiveVaultFinishedWidget = new WidgetActivceVaultFinished(this);
    connect(m_ActiveVaultFinishedWidget, &WidgetActivceVaultFinished::sigAccepted,
            this, &WidgetManager::slotNextWidget);

    m_pStackedWidget->addWidget(m_pStartVaultWidget);
    m_pStackedWidget->addWidget(m_pSetUnclockMethodWidget);
    m_pStackedWidget->addWidget(m_SaveKeyWidget);
    m_pStackedWidget->addWidget(m_ActiveVaultFinishedWidget);

    this->addContent(m_pStackedWidget);

//    play->addWidget(m_pStackedWidget);
}

void WidgetManager::slotNextWidget()
{
    if(m_pStackedWidget){
        int nIndex = m_pStackedWidget->currentIndex();
        int nCount = m_pStackedWidget->count();
        if(nIndex < nCount-1)
        {
            int nNextIndex = nIndex + 1;
            m_pStackedWidget->setCurrentIndex(nNextIndex);
        }else{
            emit accept();
        }
    }
}
