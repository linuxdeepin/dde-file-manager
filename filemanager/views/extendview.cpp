#include "extendview.h"
#include <QListWidget>
#include <QHBoxLayout>
#include <QDir>
#include <QDebug>
#include "ddetailview.h"
#include "dfileview.h"


ExtendView::ExtendView(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnect();
}

ExtendView::~ExtendView()
{

}

void ExtendView::initUI()
{
    m_extendListView = new QListWidget;
    m_extendListView->setFlow(QListWidget::LeftToRight);
    m_detailView = new DDetailView;
    m_detailView->setFixedWidth(200);
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_extendListView);
    mainLayout->addWidget(m_detailView);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

void ExtendView::initConnect()
{

}

void ExtendView::setStartUrl(const DUrl &url)
{
    if (url == m_currentUrl){
        return;
    }
//    m_extendListView->clear();
//    DUrlList urls = m_startUrl.childrenList(url);

//    foreach (DUrl url, urls) {
//        qDebug()<< url;
//        QListWidgetItem* item = new QListWidgetItem;
//        item->setSizeHint(QSize(200, 10));
//        m_extendListView->addItem(item);

//        DFileView* view = new DFileView;
//        view->setViewModeToExtend();
//        view->setFixedWidth(200);
//        view->setCurrentUrl(url);
//        m_extendListView->setItemWidget(item, view);
//    }

    m_currentUrl = url;
}

