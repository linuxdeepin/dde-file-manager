#include "dsearchbar.h"
#include <QDirModel>
#include <QDebug>

DSearchBar::DSearchBar(QWidget *parent):QLineEdit(parent)
{
    m_list = new QListWidget;
    m_list->setStyleSheet("QListWidget::item:hover {background:lightGray;}");
    m_list->setMouseTracking(true);
    m_completer = new QCompleter;
    m_completer->setModel(new QDirModel(m_completer));
    m_completer->setPopup(m_list);
    setCompleter(m_completer);
    QIcon icon(":/images/images/light/appbar.close.png");
    m_clearAction = new QAction(icon,"", this);
    initConnections();
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
}

void DSearchBar::doTextChanged(QString text)
{
    m_list->clear();
    QStringList stringList;
    m_completer->setCompletionPrefix(text);
    for (int i = 0; m_completer->setCurrentRow(i); i++)
        stringList << m_completer->currentCompletion();
    m_list->addItems(stringList);
}

void DSearchBar::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    emit searchBarFocused();
}
