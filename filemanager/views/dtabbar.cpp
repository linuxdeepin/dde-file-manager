#include "dtabbar.h"
#include "dtabbaritem.h"
#include <QDebug>

DTabBar::DTabBar(QWidget *parent) : QFrame(parent)
{
    m_count = 3;
    setFixedWidth(100);
}

/**
 * @brief TabBar::addTab
 * @param text
 *
 * Add a tab with the given text.
 */
void DTabBar::addTab(const QString &text)
{
    deSelectAll();
    DTabBarItem * item = new DTabBarItem(this, text, m_items.size());
    m_items.append(item);
    item->show();
    doLayout();
}

/**
 * @brief TabBar::removeTab
 * @param index
 *
 * Remove the tab with the given index.
 */
void DTabBar::removeTab(int index)
{
    DTabBarItem * item = m_items.at(index);
    m_items.removeAt(index);
    delete item;
    doLayout();
}

/**
 * @brief TabBar::count
 * @return
 *
 * Return the amount of tabs.
 */
int DTabBar::count()
{
    return m_items.size();
}

/**
 * @brief TabBar::doLayout
 *
 * Calculate the layout of all tabs
 */
void DTabBar::doLayout()
{
    double growWidth = 0;
    double h = height();
    for(int i = 0; i < count(); i++)
    {
        DTabBarItem * item = m_items.at(i);
        double w = item->width();
        item->setGeometry(growWidth, 0, w, h);
        growWidth += w - 6;
    }
}

/**
 * @brief TabBar::deSelectAll
 *
 * Deselect all tabs. White is the default color.
 */
void DTabBar::deSelectAll()
{
    for(int i = 0; i < count(); i++)
    {
        DTabBarItem * item = m_items.at(i);
        item->setSelected(false);
    }
}

/**
 * @brief DTabBar::setTabBarText
 * @param text
 *
 * Split text into tabs wherever '/' meets.
 * Note that an valid path must be given in this case.
 */
void DTabBar::setTabBarPath(const QString &text)
{
    if(text.isEmpty() || text.at(0) != '/')
        return;

    m_currentAddress = text;
    QStringList list;
    list.append(text.split("/"));
    list.replace(0, "/");
    list.removeAll("");
    clearAllTabs();
    m_addressOffset = (list.size() < m_count) ? 0 : list.size() - m_count;
    for(int i = (list.size() < m_count) ? 0 : list.size() - m_count; i < list.size(); i++)
        addTab((QString)list.at(i));
    m_items.last()->setSelected(true);
    emit tabBarChanged(m_currentAddress);
}

/**
 * @brief DTabBar::shrinkToIndex
 * @param index
 * @return
 *
 * Shrink the number of tabs to the given index number.
 */
QString DTabBar::shrinkToIndex(int index)
{
    QString tabText;
    QStringList list = m_currentAddress.split("/");
    list.replace(0, "/");
    list.removeAll("");

    tabText += list.at(0);
    for(int i = 1; i <= index + m_addressOffset; i++)
        tabText += list.at(i) + "/";

    if(tabText != "/")
        tabText.remove(tabText.length() -1 ,1);

    setTabBarPath(tabText);
    return m_currentAddress;
}

/**
 * @brief DTabBar::moveToIndex
 * @param index
 * @return
 *
 * Move to the tab with the given index
 * and select the tab. Return the path at
 * the index.
 */
QString DTabBar::moveToIndex(int index)
{
    QString tabText;
    QStringList list = m_currentAddress.split("/");
    list.replace(0, "/");
    list.removeAll("");

    deSelectAll();
    tabText += list.at(0);
    for(int i = 1; i <= index + m_addressOffset; i++)
    {
        tabText += list.at(i) + "/";
    }
    if(tabText != "/")
        tabText.remove(tabText.length() -1 ,1);
    m_items.at(index)->setSelected(true);
    repaint();
    emit tabBarChanged(tabText);
    return tabText;
}

/**
 * @brief DTabBar::setTabSelected
 * @param index
 *
 * set the tab to selected state with the given index.
 */
void DTabBar::setTabSelected(int index)
{
    if(index < m_items.size())
    {
        deSelectAll();
        m_items.at(index)->setSelected(true);
    }
}

/**
 * @brief DTabBar::getSelectedIndex
 * @return
 *
 * Return the index of selected item or
 * return -1 if none of items is selected.
 */
int DTabBar::getSelectedIndex()
{
    for(int i = 0; i < m_items.size(); i++)
    {
        DTabBarItem * item = m_items.at(i);
        if(item->isSelected())
            return i;
    }
    return -1;
}

/**
 * @brief DTabBar::clearAllTabs
 *
 * Clear all the tabs
 */
void DTabBar::clearAllTabs()
{
    for(int i = 0; i < count();)
    {
        removeTab(i);
    }
}

/**
 * @brief DTabBar::setMaximumTabCount
 * @param count
 *
 * Set the maximum number of tabs can be displayed.
 */
void DTabBar::setMaximumTabCount(int count)
{
    m_count = count;
}

/**
 * @brief DTabBar::setTabs
 * @param list
 *
 * Create tabs with the given string list.
 */
void DTabBar::setTabs(QStringList list)
{
    clearAllTabs();
    int size = (list.size() < m_count) ? list.size() : m_count;
    for(int i = 0; i < size; i++)
        addTab((QString)list.at(i));
}



