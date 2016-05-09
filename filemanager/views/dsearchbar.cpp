#include "dsearchbar.h"
#include "ddirmodel.h"

#include "../app/global.h"

#include "../controllers/searchhistroymanager.h"
#include "../app/filesignalmanager.h"

#include <QDirModel>
#include <QDebug>
#include "dscrollbar.h"
#include "../app/fmevent.h"

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
    m_list->setVerticalScrollBar(new DScrollBar);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
        delete m_list;
    }

    m_list = popup;
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
    m_searchStart = false;
    if(active)
        connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
                this, &DSearchBar::currentUrlChanged);
    else
        disconnect(fileSignalManager, &FileSignalManager::currentUrlChanged,
                this, &DSearchBar::currentUrlChanged);
    if(text().isEmpty())
        m_clearAction->setVisible(false);
    else
        m_clearAction->setVisible(true);
}

void DSearchBar::initConnections()
{
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(doTextChanged(QString)));
    connect(this, &DSearchBar::returnPressed, this, &DSearchBar::historySaved);
    connect(this, &DSearchBar::textChanged, this, &DSearchBar::setCompleter);
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

    if(m_disableCompletion)
    {
        m_disableCompletion = false;
        return;
    }

    m_list->clear();

    if(hasScheme() || isPath())
    {
        QFileInfo fileInfo;
        if(isLocalFile())
        {
            DUrl url(text);
            fileInfo.setFile(url.toLocalFile());
        }
        else
        {
            fileInfo.setFile(text);
        }
        QDir dir(fileInfo.absolutePath());
        QStringList localList = splitPath(text);
        QStringList sl;

        foreach(QFileInfo info, dir.entryInfoList(QDir::AllDirs|QDir::NoDotAndDotDot, QDir::Name))
        {
            QString temp = localList.last();
            qDebug() << temp << info.absoluteFilePath() << fileInfo.absoluteFilePath();
            if (info.absoluteFilePath().mid(0, fileInfo.absoluteFilePath().length())
                    == fileInfo.absoluteFilePath())
            {
                if(temp.isEmpty())
                    sl << info.fileName();
                else if(temp.at(0) == info.fileName().at(0))
                    sl << info.fileName();
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
    recomended();
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
    m_disableCompletion = true;
    QStringList list = splitPath(m_text);
    QString modelText = item->text();
    QString last = list.last();
    if(isPath())
    {
        list.removeLast();
        list.append(modelText);
        setText(list.join("/").replace(0,1,""));
    }
    else if(isLocalFile())
    {
        list.removeLast();
        list.append(modelText);
        setText(list.join("/"));
    }
    else
    {
        setText(modelText);
    }
    m_text = text();
    m_list->hide();
}

void DSearchBar::currentUrlChanged(const FMEvent &event)
{
    if(event.fileUrl().isSearchFile())
        return;
    if(event.fileUrl() != m_currentPath)
        emit focusedOut();
    qDebug() << event.fileUrl() << m_currentPath;
}

void DSearchBar::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
}

void DSearchBar::focusOutEvent(QFocusEvent *e)
{
    qDebug() << "focus out";
    if(m_searchStart)
        return;
    if(window()->rect().contains(window()->mapFromGlobal(QCursor::pos())))
    {
        if(m_list->isHidden())
        {
            QLineEdit::focusOutEvent(e);
            emit focusedOut();
        }
        else if(!m_list->rect().contains(m_list->mapFromGlobal(QCursor::pos())))
        {
            m_list->hide();
            QLineEdit::focusOutEvent(e);
            emit focusedOut();
        }
        else
        {
            m_list->hide();
            QLineEdit::focusOutEvent(e);
        }
    }
    else
    {
        m_list->hide();
        QLineEdit::focusOutEvent(e);
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
        this->window()->activateWindow();
        setFocus();

        if(m_searchStart)
            return false;
        if(!m_list->rect().contains(m_list->mapFromGlobal(QCursor::pos())))
            emit focusedOut();

        return false;
    }
    else if(e->type() == QEvent::MouseButtonPress)
    {
        m_list->hide();
        this->window()->activateWindow();
        setFocus();
        return true;
    }
    else if(e->type() == QEvent::FocusIn)
    {
        return true;
    }
    else if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent * keyEvent = static_cast<QKeyEvent*> (e);
        keyPressEvent(keyEvent);
        this->window()->activateWindow();
        setFocus();
        return true;
    }
    return false;
}

void DSearchBar::resizeEvent(QResizeEvent *e)
{
    m_list->hide();
    QLineEdit::resizeEvent(e);
}

void DSearchBar::moveEvent(QMoveEvent *e)
{
    m_list->hide();
    QLineEdit::moveEvent(e);
}

void DSearchBar::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "mouse press";
    QLineEdit::mousePressEvent(e);
}

void DSearchBar::keyUpDown(int key)
{
    m_disableCompletion = true;
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
        QStringList list = splitPath(m_text);
        QString last = list.last();
        if(isPath())
        {
            list.removeLast();
            list.append(modelText);
            setText(list.join("/").replace(0,1,""));
            setSelection(text().length() + last.length() - modelText.length(), text().length());
        }
        else if(isLocalFile())
        {
            list.removeLast();
            list.append(modelText);
            setText(list.join("/"));
            setSelection(text().length() + last.length() - modelText.length(), text().length());
        }
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
        else if(isLocalFile())
        {
            list.removeLast();
            list.append(modelText);
            setText(list.join("/"));
            setSelection(text().length() + last.length() - modelText.length(), text().length());
        }
        else
        {
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
    else if(isLocalFile())
    {
        list.removeLast();
        list.append(modelText);
        setText(list.join("/"));
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
    QModelIndex currentIndex = m_list->currentIndex();

    switch(key)
    {
        case Qt::Key_Escape:
            emit focusedOut();
            break;
        case Qt::Key_Down:
        case Qt::Key_Up:
            keyUpDown(key);
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
        {
            if(!(hasScheme() || isPath()))
                m_searchStart = true;
            if (currentIndex.isValid())
            {
                QString text = m_list->currentIndex().data().toString();
                complete(text);
            }
            deselect();
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

                    if(isPath())
                    {
                        list.removeLast();
                        list.append(modelText);
                        setText(list.join("/").replace(0,1,""));
                    }
                    else if(isLocalFile())
                    {
                        list.removeLast();
                        list.append(modelText);
                        setText(list.join("/"));
                    }
                    else
                    {
                        setText(m_text);
                    }
                    setSelection(text().count() + last.count() - modelText.count(), text().count());
                    m_text = text();
                }
                else
                {
                    if(key != Qt::Key_Tab)
                        QLineEdit::keyPressEvent(e);
                    break;
                }
            }
            m_list->hide();
            m_list->clear();
            end(false);
            if(isPath() || isLocalFile())
            {
                setText(text() + "/");
                m_text = text();
            }
            if(key != Qt::Key_Tab)
                QLineEdit::keyPressEvent(e);
            break;
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            m_list->hide();
            if(text().length() > 1)
                m_disableCompletion = true;
            QLineEdit::keyPressEvent(e);

            break;
        default:
        {
            if(e->modifiers() == Qt::NoModifier)
                m_list->hide();
            QLineEdit::keyPressEvent(e);
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

void DSearchBar::setCurrentPath(const DUrl &path)
{
    m_currentPath = path;
}
