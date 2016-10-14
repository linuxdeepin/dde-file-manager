#include "dsearchbar.h"

#include "windowmanager.h"
#include "deditorwidgetmenu.h"

#include "controllers/searchhistroymanager.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "fmevent.h"

#include "widgets/singleton.h"

#include <dscrollbar.h>

#include <QDirModel>
#include <QLabel>
#include <QDebug>
#include <QApplication>
#include <QFocusEvent>

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
    m_list->setFocusPolicy(Qt::NoFocus);
    m_stringListMode = new QStringListModel(this);
    m_list->setWindowFlags(Qt::ToolTip);
    m_list->viewport()->setContentsMargins(4, 4, 4, 4);
    m_list->setObjectName("SearchList");

    m_dirModel = new QDirModel;
    m_dirModel->setFilter(QDir::Dirs);

    QIcon icon(":/icons/images/icons/input_normalclear_normal.png");
    m_clearAction = new QAction(icon, "", this);

    m_inputClearButton = new QPushButton(this);
    m_inputClearButton->setFixedSize(14, 14);
    m_inputClearButton->setObjectName("InputClearButton");
    m_inputClearButton->hide();

    setFixedHeight(24);
    setObjectName("DSearchBar");
    setMinimumWidth(1);

    setFocusPolicy(Qt::ClickFocus);
    setClearAction();

    m_list->installEventFilter(this);
    m_list->setVerticalScrollBar(new DScrollBar);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    Q_UNUSED(new DEditorWidgetMenu(this))
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
    connect(m_clearAction, &QAction::triggered, this, &DSearchBar::clearText);
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
    connect(qApp, &QApplication::focusChanged, this, &DSearchBar::handleApplicationChanged);
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
    if (!hasScheme()){
        if(!m_historyList.contains(str))
        {

            m_historyList.append(str);
            m_stringListMode->setStringList(m_historyList);
            searchHistoryManager->writeIntoSearchHistory(str);
        }
    }
    m_list->hide();
}

void DSearchBar::setCompleter(const QString &text)
{
    if(text.isEmpty())
        m_clearAction->setVisible(false);
    else
        m_clearAction->setVisible(true);

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

    const DUrl &url = DUrl::fromUserInput(text);

//    qDebug() << text << url << url.isLocalFile() << url.path().isEmpty() << url.toLocalFile();
    if(/*hasScheme() || */url.isLocalFile())
    {
        QFileInfo fileInfo;
        if (text.endsWith(".")){
            fileInfo = QFileInfo(url.path().isEmpty() ? "/" : url.toLocalFile() + "/");
        }else{
            fileInfo = QFileInfo(url.path().isEmpty() ? "/" : url.toLocalFile());
        }
//        qDebug() << url.toLocalFile() << fileInfo.absolutePath() << fileInfo.path() << QDir::cleanPath(url.toLocalFile());
        QDir dir;
        if(fileInfo.exists())
            dir = QDir(fileInfo.absoluteFilePath());
        else
            dir = QDir(fileInfo.absolutePath());
        QStringList localList = splitPath(text);
        QStringList sl;
//        qDebug() << fileInfo.absolutePath();
        foreach(QFileInfo info, dir.entryInfoList(QDir::AllDirs| QDir::Hidden |QDir::NoDotAndDotDot, QDir::Name))
        {
            QString temp = localList.last();
//            qDebug() << temp << info.absoluteFilePath() << fileInfo.absoluteFilePath();
            if (info.absoluteFilePath().mid(0, fileInfo.absoluteFilePath().length())
                    == fileInfo.absoluteFilePath())
            {
                if(temp.isEmpty())
                    sl << info.fileName();
                else if(temp.at(0) == info.fileName().at(0))
                    sl << info.fileName();
            }
        }
        if(sl.isEmpty()){
            m_list->hide();
            return;
        }
        foreach (QString itemText, sl) {
            QListWidgetItem* item = new QListWidgetItem(itemText);
            item->setTextAlignment(Qt::AlignVCenter);
            QPixmap p(":/icons/images/light/space16.png");
            item->setIcon(QIcon(p));
            m_list->addItem(item);
        }
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
//        m_list->addItems(m_stringListMode->stringList());

        foreach (QString itemText, m_stringListMode->stringList()) {
            QListWidgetItem* item = new QListWidgetItem(itemText);
            item->setTextAlignment(Qt::AlignVCenter);
            QPixmap p(":/icons/images/light/space16.png");
            item->setIcon(QIcon(p));
            m_list->addItem(item);
        }

        if (m_stringListMode->rowCount() == 0) {
            return;
        }
    }
    recomended(text);

    if (m_list->count() < 10){
        m_list->setFixedHeight(24 * m_list->count() + 8);
    }else{
        m_list->setFixedHeight(24 * 10);
    }


    m_list->setMinimumWidth(width());
    m_list->setMaximumWidth(width());
    QPoint p(0, height());
    int x = mapToGlobal(p).x();
    int y = mapToGlobal(p).y() + 4;
    m_list->move(x, y);

    if (m_list->count() >= 1){
        m_list->show();
    }
}

void DSearchBar::completeText(QListWidgetItem *item)
{
    qDebug() << item;
    m_disableCompletion = true;
    QStringList list = splitPath(m_text);
    QString modelText = item->text();
    QString last = list.last();
    if(isPath())
    {
        list.removeLast();
        list.append(modelText);
        qDebug() << list;
        if (list.length() >=1 && list.at(0) == "/"){
            setText(list.join("/").replace(0, 1, ""));
        }else{
            setText(list.join("/"));
        }
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
//    m_text = text();
    m_list->hide();
}

void DSearchBar::currentUrlChanged(const FMEvent &event)
{
    if(event.fileUrl().isSearchFile())
    {
        return;
    }
    if(event.fileUrl() != m_currentPath)
        emit focusedOut();
}

void DSearchBar::clearText()
{
    clear();
    m_searchStart = false;
    FMEvent event;

    event << WindowManager::getWindowId(this);
    event << FMEvent::SearchBar;
    event << m_currentPath;


    emit fileSignalManager->requestChangeCurrentUrl(event);
    emit focusedOut();
}

void DSearchBar::hideCompleter()
{
    m_list->hide();
}

void DSearchBar::handleApplicationChanged(QWidget *old, QWidget *now)
{
    if (old == this && now == NULL){
        m_list->hide();
    }else if (old == NULL && now == this){
        if (m_list->count() > 0){
//            m_list->show();
        }
    }
}

void DSearchBar::setText(const QString &text)
{
    m_text = text;
    QLineEdit::setText(text);
}

void DSearchBar::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
}

void DSearchBar::focusOutEvent(QFocusEvent *e)
{
    if(e->reason() == Qt::ActiveWindowFocusReason)
        return;
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
//    qDebug() << e;
    if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent * keyEvent = static_cast<QKeyEvent*> (e);
        if(keyEvent->key() == Qt::Key_Tab)
        {
            keyPressEvent(keyEvent);
            return true;
        }
        else
            return QLineEdit::event(e);
    }
    else
        return QLineEdit::event(e);
}

bool DSearchBar::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj);
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
    m_inputClearButton->move(width() - 16, (height() - 16)/ 2);
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

            if (list.length() >=1 && list.at(0) == "/"){
                setText(list.join("/").replace(0, 1, ""));
            }else{
                setText(list.join("/"));
            }

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
    }
    m_disableCompletion = true;
    qDebug() << text();
}

void DSearchBar::recomended(const QString& inputText)
{
    if(m_list->count() == 1)
    {
        m_disableCompletion = true;
        QStringList list = splitPath(m_text);
        QString modelText = m_list->item(0)->text();
        QString last = list.last();
        if(isPath())
        {
            list.removeLast();
            list.append(modelText);
            if (list.length() > 0){
                if (list.length() >=1 && list.at(0) == "/"){
                    setText(list.join("/").replace(0, 1, ""));
                }else{
                    setText(list.join("/"));
                }
            }
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
            setSelection(inputText.length(), modelText.length());
        }
//        m_text = text();
    }
}

void DSearchBar::complete(const QString &str)
{
    qDebug() << str;
    m_disableCompletion = true;
    QStringList list = splitPath(m_text);
    QString modelText = str;
    QString last = list.last();
    if(isPath())
    {
        list.removeLast();
        list.append(modelText);
        if (list.length() >=1 && list.at(0) == "/"){
            setText(list.join("/").replace(0, 1, ""));
        }else{
            setText(list.join("/"));
        }
    }
    else if(isUserShareFile()){
        return;
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
}

void DSearchBar::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    QModelIndex currentIndex = m_list->currentIndex();

    if (e->modifiers() != Qt::ShiftModifier){
        switch(key)
        {
            case Qt::Key_Escape:
                m_clearAction->trigger();
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
                            if (list.length() >=1 && list.at(0) == "/"){
                                setText(list.join("/").replace(0, 1, ""));
                            }else{
                                setText(list.join("/"));
                            }
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
//                        m_text = text();
                    }
                }
                m_list->hide();
                m_list->clear();
                if(isPath() || isLocalFile())
                {
                    if (!text().endsWith("/")){

                        setText(text() + "/");
                        setCompleter(text());
                    }else{
                        if (QFile(text()).exists() && (cursorPosition() == (text().length() - 1))){
                            if (!m_list->isVisible()){
                                setCompleter(text());
                            }
                        }
                    }
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
            case Qt::Key_Slash:
                if (e->modifiers() == Qt::NoModifier)
                    m_disableCompletion = false;

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
    else{
        QLineEdit::keyPressEvent(e);
    }
}

QAction *DSearchBar::getClearAction()
{
    return m_clearAction;
}

bool DSearchBar::hasScheme()
{
    DUrl url = DUrl::fromUserInput(text(), false);

    if( url.isBookMarkFile() ||
            url.isComputerFile() ||
            url.isLocalFile() ||
            url.isRecentFile() ||
            url.isTrashFile() ||
            url.isSearchFile() ||
            url.isNetWorkFile() ||
            url.isSMBFile() ||
            url.isUserShareFile())
        return true;
    else
        return false;
}

bool DSearchBar::isSearchFile()
{
    return DUrl::fromUserInput(text()).isSearchFile();
}

bool DSearchBar::isBookmarkFile()
{
    return DUrl::fromUserInput(text()).isSearchFile();
}

bool DSearchBar::isComputerFile()
{
    return DUrl::fromUserInput(text()).isComputerFile();
}

bool DSearchBar::isLocalFile()
{
    return DUrl::fromUserInput(text()).isLocalFile();
}

bool DSearchBar::isTrashFile()
{
    return DUrl::fromUserInput(text()).isTrashFile();
}

bool DSearchBar::isUserShareFile()
{
    return DUrl(text()).isUserShareFile();
}

bool DSearchBar::isPath()
{
    if (text().isEmpty())
        return false;

    return text().startsWith("/") || text().startsWith("./")
            || text().startsWith("~") || text().startsWith("../");
}

void DSearchBar::setCurrentPath(const DUrl &path)
{
    m_currentPath = path;
    m_disableCompletion = true;

    if (path.isLocalFile()){
        setText(path.toLocalFile());
        setSelection(0, path.toLocalFile().length());
    }else{
        setText(path.toString());
        setSelection(0, path.toString().length());
    }
}
