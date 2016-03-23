#include "dsearchbar.h"
#include <QDirModel>
#include <QDebug>
#include "ddirmodel.h"
#include "../app/global.h"

DSearchBar::DSearchBar(QWidget *parent):QLineEdit(parent)
{
    initUI();
    initData();
    initConnections();
}

void DSearchBar::initData()
{
    searchHistoryLoaded(searchHistoryManager->toStringList());
}

void DSearchBar::initUI()
{
    m_list = new QListWidget;
    m_hList = new QListWidget;
    m_list->setStyleSheet("QListWidget::item:hover {background:lightGray;}");
    m_list->setMouseTracking(true);
    m_completer = new QCompleter;
    m_completer->setModel(new QDirModel(m_completer));

    m_stringListMode = new QStringListModel;
    m_historyCompleter = new QCompleter;
    m_historyCompleter->setModel(m_stringListMode);

    QIcon icon(":/images/images/light/appbar.close.png");
    m_clearAction = new QAction(icon,"", this);

    setFixedHeight(20);
    setObjectName("DSearchBar");
    setMinimumWidth(48);
}


DSearchBar::~DSearchBar()
{

}

QListWidget *DSearchBar::getPopupList()
{
    return m_list;
}

QAction * DSearchBar::setClearAction()
{
    addAction(m_clearAction, QLineEdit::TrailingPosition);
    return m_clearAction;
}

QAction * DSearchBar::removeClearAction()
{
    removeAction(m_clearAction);
    return m_clearAction;
}

void DSearchBar::initConnections()
{
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(doTextChanged(QString)));
    //connect(fileSignalManager, &FileSignalManager::searchHistoryLoaded, this, &DSearchBar::searchHistoryLoaded);
    connect(this, &DSearchBar::returnPressed, this, &DSearchBar::historySaved);
}

void DSearchBar::doTextChanged(QString text)
{
    m_list->clear();
    m_hList->clear();
    QStringList stringList;
    m_completer->setCompletionPrefix(text);
    for (int i = 0; m_completer->setCurrentRow(i); i++)
        stringList << m_completer->currentCompletion();
    if(!stringList.isEmpty())
    {
        m_list->addItems(stringList);
        m_completer->setPopup(m_list);
        setCompleter(m_completer);
    }
    else
    {
        m_historyCompleter->setCompletionPrefix(text);
        for (int i = 0; m_historyCompleter->setCurrentRow(i); i++)
            stringList << m_historyCompleter->currentCompletion();

        m_hList->addItems(stringList);
        m_historyCompleter->setPopup(m_hList);
        setCompleter(m_historyCompleter);
    }

}

void DSearchBar::searchHistoryLoaded(const QStringList &list)
{
    m_historyList.append(list);
    m_stringListMode->setStringList(m_historyList);
}

void DSearchBar::historySaved()
{
    if(text().isEmpty())
        return;
    QString str = text();
    if(!m_historyList.contains(str))
    {
        m_historyList.append(str);
        m_stringListMode->setStringList(m_historyList);
        searchHistoryManager->writeIntoSearchHistory(str);
    }
}

void DSearchBar::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    emit searchBarFocused();
}
