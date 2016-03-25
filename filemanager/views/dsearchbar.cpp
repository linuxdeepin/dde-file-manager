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
    m_isActive = false;
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

    setFocusPolicy(Qt::ClickFocus);
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

bool DSearchBar::isActive()
{
    return m_isActive;
}

void DSearchBar::setActive(bool active)
{
    m_isActive = active;
}

void DSearchBar::initConnections()
{
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(doTextChanged(QString)));
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
    if(!m_isActive)
        emit searchBarFocused();
}

void DSearchBar::keyPressEvent(QKeyEvent *e)
{
    //press right key to deselect the text
    if(e->key() == Qt::Key_Right)
    {
        deselect();
        return;
    }
    if(e->key() == Qt::Key_Return)
    {
        deselect();
        QLineEdit::keyPressEvent(e);
        return;
    }
    QString selected = selectedText();
    if(selected != "")
    {
        QString temp = text().remove(selected);
        setText(temp);
        qDebug() << selected << text();
    }
    QLineEdit::keyPressEvent(e);
    if(m_list->count() == 1 && e->key() != Qt::Key_Delete && e->key() != Qt::Key_Backspace)
    {
        QString t = m_list->item(0)->text();
        if(t.contains(text()))
        {
            QStringList list = t.split(text());
            QString rightText = list.at(1);
            if(rightText != "")
            {
                setText(t);
                int start = t.indexOf(rightText);
                int end = t.length() - 1;
                setSelection(start, end);
            }
        }
    }
}

QAction *DSearchBar::getClearAction()
{
    return m_clearAction;
}
