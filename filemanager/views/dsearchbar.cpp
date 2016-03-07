#include "dsearchbar.h"

DSearchBar::DSearchBar(QWidget *parent):QLineEdit(parent)
{
    m_menu = new QMenu(this);
    m_list = new QListWidget(m_menu);
    m_list->setStyleSheet("QListWidget::item:hover {background-color:lightGray;}");

    m_menuLayout = new QHBoxLayout;
    m_menuLayout->addWidget(m_list);
    m_menuLayout->setContentsMargins(0,0,0,0);
    m_menu->setLayout(m_menuLayout);
}

DSearchBar::~DSearchBar()
{

}

QListWidget *DSearchBar::getList()
{
    return m_list;
}

void DSearchBar::openDropDown()
{
    QRect rect = geometry();
    double w = width();
    QPoint widgetPos = mapToGlobal(rect.bottomLeft());
    m_menu->setFixedWidth(w);
    m_menu->exec(widgetPos);
}

void DSearchBar::closeDropDown()
{
    m_menu->hide();
}
