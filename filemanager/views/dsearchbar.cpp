#include "dsearchbar.h"
#include "ddirmodel.h"

#include "../app/global.h"

#include "../controllers/searchhistroymanager.h"

#include <QDirModel>
#include <QDebug>

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
    setMinimumWidth(1);

    setFocusPolicy(Qt::ClickFocus);
    setClearAction();
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
    connect(m_clearAction, &QAction::triggered, this, &DSearchBar::clear);
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

        m_list->addItems(stringList);
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
    QString selected = selectedText();
    if(e->key() == Qt::Key_Left || e->key() == Qt::Key_Tab)
    {
        if(!selected.isEmpty())
        {
            setCursorPosition(text().length() - selected.length());
            deselect();
        }
        else if(e->key() == Qt::Key_Left)
            QLineEdit::keyPressEvent(e);
        return;
    }
    //press right key to deselect the text and jump to the left end
    if(e->key() == Qt::Key_Right)
    {
        deselect();
        QLineEdit::keyPressEvent(e);
        return;
    }
    //press enter to deselect the text and return
    if(e->key() == Qt::Key_Return)
    {
        deselect();
        QLineEdit::keyPressEvent(e);
        return;
    }
    //If any other key is detected and there is selected text,
    //then remove the selected text.
    if(selected != "")
    {
        QString temp = text().left(text().length() - selected.length());
        setText(temp);
        if(e->key() != Qt::Key_Backspace && e->key() != Qt::Key_Delete)
            QLineEdit::keyPressEvent(e);
    }
    else
        QLineEdit::keyPressEvent(e);
    //If there is only one recommended line of text then do the completion
    if(m_list->count() == 1 && e->key() != Qt::Key_Delete && e->key() != Qt::Key_Backspace)
    {
        QString t = m_list->item(0)->text();
        QString localText = text();
        if(t.contains(text()))
        {
            QStringList list = t.split(text());
            QString rightText = list.at(1);
            if(rightText != "")
            {
                setText(t);
                int start = localText.length();
                int end = t.length() - 1;
                qDebug() << rightText << start << end;
                setSelection(start, end);
            }
        }
    }
}

QAction *DSearchBar::getClearAction()
{
    return m_clearAction;
}

bool DSearchBar::hasScheme()
{
    DUrl url(text());
    if( url.isBookMarkFile() ||
            url.isComputerFile() ||
            url.isLocalFile() ||
            url.isRecentFile() ||
            url.isTrashFile() ||
            url.isSearchFile())
        return true;
    else
        return false;
}

bool DSearchBar::isPath()
{
    if(text().at(0) == '/')
        return true;
    else
        return false;
}
