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
    m_list = new QListWidget(this);
    m_stringListMode = new QStringListModel(this);
    m_list->setWindowFlags(Qt::ToolTip);

    m_dirModel = new QDirModel;
    m_dirModel->setFilter(QDir::Dirs);

    QIcon icon(":/images/images/light/appbar.close.png");
    m_clearAction = new QAction(icon,"", this);

    setFixedHeight(20);
    setObjectName("DSearchBar");
    setMinimumWidth(1);

    setFocusPolicy(Qt::ClickFocus);
    setClearAction();

    m_list->installEventFilter(this);
}

QStringList DSearchBar::splitPath(const QString &path)
{
    QString pathCopy = QDir::toNativeSeparators(path);
    QString sep = QDir::separator();

    QRegExp re(QLatin1Char('[') + QRegExp::escape(sep) + QLatin1Char(']'));
    QStringList parts = pathCopy.split(re);

    if (pathCopy[0] == sep[0])
        parts[0] = QDir::fromNativeSeparators(QString(sep[0]));

    return parts;
}

DSearchBar::~DSearchBar()
{

}

void DSearchBar::setPopup(QListWidget *popup)
{
    if(popup == NULL)
        return;

    if(m_list)
    {
        m_list->removeEventFilter(this);
        delete m_list;
    }

    m_list = popup;
    m_list->installEventFilter(this);
    m_list->setWindowFlags(Qt::ToolTip);
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
    if(text().isEmpty())
        m_clearAction->setVisible(false);
    else
        m_clearAction->setVisible(true);
}

void DSearchBar::initConnections()
{
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(doTextChanged(QString)));
    connect(this, &DSearchBar::returnPressed, this, &DSearchBar::historySaved);
    //connect(this, &DSearchBar::textChanged, this, &DSearchBar::setCompleter);
    connect(m_list, &QListWidget::itemClicked, this, &DSearchBar::completeText);
}

void DSearchBar::doTextChanged(QString text)
{
    m_text = text;
    if(text.isEmpty())
        m_clearAction->setVisible(false);
    else
        m_clearAction->setVisible(true);
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

void DSearchBar::setCompleter(const QString &text)
{
    if (text.isEmpty())
    {
        m_list->hide();
        return;
    }

    m_list->clear();

    if(hasScheme() || isPath())
    {
        QFileInfo fileInfo(text);
        QDir dir(fileInfo.absolutePath());
        QStringList localList = splitPath(text);
        QStringList sl;
        foreach(QString word, dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot, QDir::Name))
        {
            QString temp = localList.last();
            if (word.contains(temp))
            {
                if(temp.isEmpty())
                    sl << word;
                else if(temp.at(0) == word.at(0))
                    sl << word;
            }
        }
        if(sl.isEmpty())
            return;
        m_list->addItems(sl);
    }
    else
    {
        QStringList sl;
        foreach(QString word, m_historyList)
        {
            if(word.contains(text) && word.at(0) == text.at(0))
            {
                sl << word;
            }
        }
        m_stringListMode->setStringList(sl);
        m_list->addItems(m_stringListMode->stringList());
        if (m_stringListMode->rowCount() == 0) {
            return;
        }
    }

    // Position the text edit
    m_list->setMinimumWidth(width());
    m_list->setMaximumWidth(width());
    QPoint p(0, height());
    int x = mapToGlobal(p).x();
    int y = mapToGlobal(p).y() + 1;
    m_list->move(x, y);
    m_list->show();
}

void DSearchBar::completeText(QListWidgetItem *item)
{
    QString text = item->text();
    setText(text);
    m_list->hide();
}

void DSearchBar::focusInEvent(QFocusEvent *e)
{

}

void DSearchBar::focusOutEvent(QFocusEvent *e)
{
    if(!m_list->rect().contains(m_list->mapFromGlobal(QCursor::pos())))
    {
        m_list->hide();
    }
}

bool DSearchBar::event(QEvent *e)
{
    if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent * keyEvent = static_cast<QKeyEvent*> (e);
        if(keyEvent->key() == Qt::Key_Tab)
            keyPressEvent(keyEvent);
        else
            return QLineEdit::event(e);
        e->accept();
    }
    else
        return QLineEdit::event(e);
}

bool DSearchBar::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type() == QEvent::FocusOut)
    {
        m_list->hide();
        return true;
    }
    else if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent * keyEvent = static_cast<QKeyEvent*> (e);
        keyPressEvent(keyEvent);
        return true;
    }
    return false;
}

void DSearchBar::keyUpDown(int key)
{
    int row;
    int count = m_list->count();
    QModelIndex currentIndex = m_list->currentIndex();
    if(Qt::Key_Down == key)
    {
        row = currentIndex.row() + 1;
        if (row >= count)
            row = -1;
    }
    else if(Qt::Key_Up)
    {
        row = currentIndex.row() - 1;
        if (row < -1)
            row = count - 1;
    }
    else
        return;

    QModelIndex index = m_list->model()->index(row, 0);
    m_list->setCurrentIndex(index);
    if(row != -1)
    {
        QString modelText = index.model()->data(index).toString();
        setText(modelText);
        QStringList list = splitPath(m_text);
        QString last = list.last();
        list.removeLast();
        list.append(modelText);
        setText(list.join("/").replace(0,1,""));
        setSelection(text().length() + last.length() - modelText.length(), text().length());
    }
    else
        setText(m_text);
}

void DSearchBar::recomended()
{
    if(m_list->count() == 1)
    {
        QStringList list = splitPath(m_text);
        QString modelText = m_list->item(0)->text();
        QString last = list.last();
        if(isPath())
        {
            list.removeLast();
            list.append(modelText);
            setText(list.join("/").replace(0,1,""));
            setSelection(text().length() + last.length() - modelText.length(), text().length());
        }
        else
        {
            QString tempText = text();
            setText(modelText);
            setSelection(m_text.length(), modelText.length());
        }
        m_text = text();
    }
}

void DSearchBar::complete(const QString &str)
{
    QStringList list = splitPath(m_text);
    QString modelText = str;
    QString last = list.last();
    if(isPath())
    {
        list.removeLast();
        list.append(modelText);
        setText(list.join("/").replace(0,1,""));
    }
    else
    {
        setText(str);
    }
    m_text = text();
}

void DSearchBar::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    //if (!m_list->isHidden())
    {
        QModelIndex currentIndex = m_list->currentIndex();

        switch(key)
        {
            case Qt::Key_Down:
            case Qt::Key_Up:
                keyUpDown(key);
                break;
            case Qt::Key_Enter:
            case Qt::Key_Return:
            {
                if (currentIndex.isValid())
                {
                    QString text = m_list->currentIndex().data().toString();
                    complete(text);
                }
                m_list->hide();
                QLineEdit::keyPressEvent(e);
                break;
            }
            case Qt::Key_Right:
            case Qt::Key_Tab:
                if(selectedText().isEmpty())
                {
                    if(m_list->count() == 1)
                    {
                        QStringList list = splitPath(m_text);
                        QString modelText = m_list->item(0)->text();
                        QString last = list.last();
                        list.removeLast();
                        list.append(modelText);
                        setText(list.join("/").replace(0,1,""));
                        setSelection(text().length() + last.length() - modelText.length(), text().length());
                        m_text = text();
                    }
                    else
                        break;
                }
                m_list->hide();
                end(false);
                if(isPath())
                {
                    setText(text() + "/");
                    m_text = text();
                }
                setCompleter(m_text);
                QLineEdit::keyPressEvent(e);
                break;
            case Qt::Key_Delete:
            case Qt::Key_Backspace:
                m_list->hide();
                QLineEdit::keyPressEvent(e);
                break;
            default:
            {
                if(e->modifiers() == Qt::NoModifier)
                    m_list->hide();
                QString before = text();
                QLineEdit::keyPressEvent(e);
                QString after = text();
                bool textModified = (before != after);
                if(textModified)
                {
                    setCompleter(after);
                    recomended();
                }
            }
        }
    }
    //else
//    {
//        QLineEdit::keyPressEvent(e);
//    }
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

bool DSearchBar::isSearchFile()
{
    DUrl url(text());
    return url.isSearchFile();
}

bool DSearchBar::isBookmarkFile()
{
    DUrl url(text());
    return url.isSearchFile();
}

bool DSearchBar::isComputerFile()
{
    DUrl url(text());
    return url.isComputerFile();
}

bool DSearchBar::isLocalFile()
{
    DUrl url(text());
    return url.isLocalFile();
}

bool DSearchBar::isTrashFile()
{
    DUrl url(text());
    return url.isTrashFile();
}

bool DSearchBar::isPath()
{
    if(text().isEmpty())
        return false;
    if(text().at(0) == '/')
        return true;
    else
        return false;
}
