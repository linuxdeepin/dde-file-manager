#include <QDebug>
#include <QStandardPaths>

#include "dcrumbwidget.h"
#include "dcrumbbutton.h"
#include "../app/fmevent.h"
#include "windowmanager.h"
#include "dstatebutton.h"
#include "../app/global.h"

DCrumbWidget::DCrumbWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

void DCrumbWidget::initUI()
{
    m_homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last();
    setObjectName("DCrumbWidget");
    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setSpacing(0);
    m_buttonLayout->setContentsMargins(0,0,0,0);
    m_buttonLayout->setDirection(QHBoxLayout::RightToLeft);
    setLayout(m_buttonLayout);
    setFixedHeight(20);
}

void DCrumbWidget::prepareCrumbs(const DUrl &path)
{
    m_path = path;
    clear();
    if(path.isRecentFile())
    {
        addRecentCrumb();
        calcCrumbPath(0, false);
    }
    else if(path.isComputerFile())
    {
        addComputerCrumb();
        calcCrumbPath(0, false);
    }
    else if(path.isTrashFile())
    {
        addTrashCrumb();
        calcCrumbPath(0, false);
    }
    else
    {
        QStringList list;
        if(isInHome(path.toLocalFile()))
        {
            QString tmpPath = path.toLocalFile();
            tmpPath.replace(m_homePath, "");
            list.append(tmpPath.split("/"));
            list.insert(0, m_homePath);
            list.removeAll("");
        }
        else
        {
            list.append(path.toLocalFile().split("/"));
            if(path.isLocalFile())
                list.replace(0, "/");
            list.removeAll("");
        }
        addCrumb(list);
        if(m_prevCheckedId != -1)
            m_group.button(m_prevCheckedId)->setChecked(true);
    }
}

void DCrumbWidget::addCrumb(const QString &text)
{
    DCrumbButton * button = new DCrumbButton(m_group.buttons().size(), text, this);
    m_buttonLayout->addWidget(button);
    m_group.addButton(button, button->getIndex());
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addCrumb(const QStringList &list)
{
    for(int i = 0; i < list.size(); i++)
    {
        QString text = list.at(i);
        DCrumbButton * button;
        if(!isHomeFolder(text)){
            button = new DCrumbButton(m_group.buttons().size(), text, this);
        }else{
            button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/home_normal_16px.svg"),
                    QIcon(":/icons/images/icons/home_hover_16px.svg"),
                    QIcon(":/icons/images/icons/home_checked_16px.svg"),
                    text, this);
        }
        button->setFocusPolicy(Qt::NoFocus);
        button->adjustSize();
        m_group.addButton(button, button->getIndex());
        connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
    }
    m_group.buttons().last()->setChecked(true);
}

void DCrumbWidget::setCrumb(const DUrl &path)
{
    m_path = path;
    m_needArrows = false;
    clear();
    if(path.isRecentFile())
    {
        addRecentCrumb();
        calcCrumbPath(0, false);
    }
    else if(path.isComputerFile())
    {
        addComputerCrumb();
        calcCrumbPath(0, false);
    }
    else if(path.isTrashFile())
    {
        addTrashCrumb();
        addLocalCrumbs(path);
    }
    else
    {
        addLocalCrumbs(path);
    }
    createCrumbs();
    repaint();
}

void DCrumbWidget::clear()
{
    m_prevCheckedId = m_group.checkedId();
    qDeleteAll(m_group.buttons());
    if(m_rightArrow)
    {
        delete m_rightArrow;
        m_rightArrow = NULL;
    }

    if(m_leftArrow)
    {
        delete m_leftArrow;
        m_leftArrow = NULL;
    }
}

QString DCrumbWidget::back()
{
    if(m_group.buttons().size() <= 1)
        return m_path.toLocalFile();
    m_group.buttons().last()->deleteLater();
    m_group.buttons().removeLast();
    m_group.buttons().last()->setChecked(true);

    QString tabText;
    QStringList list = m_path.toLocalFile().split("/");
    list.replace(0, "/");
    list.removeAll("");

    tabText += list.at(0);
    for(int i = 1; i < m_group.buttons().size(); i++)
        tabText += list.at(i) + "/";

    if(tabText != "/")
        tabText.remove(tabText.length() -1 ,1);

    m_path.setUrl(tabText);
    return m_path.toLocalFile();
}

QString DCrumbWidget::path()
{
    return m_path.toLocalFile();
}

void DCrumbWidget::addRecentCrumb()
{
    QString text = RECENT_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/recent_normal_16px.svg"),
                QIcon(":/icons/images/icons/recent_hover_16px.svg"),
                QIcon(":/icons/images/icons/recent_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addComputerCrumb()
{
    QString text = COMPUTER_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/disk_normal_16px.svg"),
                QIcon(":/icons/images/icons/disk_hover_16px.svg"),
                QIcon(":/icons/images/icons/disk_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addTrashCrumb()
{
    QString text = TRASH_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/trash_normal_16px.svg"),
                QIcon(":/icons/images/icons/trash_hover_16px.svg"),
                QIcon(":/icons/images/icons/trash_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addHomeCrumb()
{
    QString text = m_homePath;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/home_normal_16px.svg"),
                QIcon(":/icons/images/icons/home_hover_16px.svg"),
                QIcon(":/icons/images/icons/home_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addLocalCrumbs(const DUrl & path)
{
    QStringList list;
    qDebug() << path.path();
    if(isInHome(path.path()))
    {
        QString tmpPath = path.toLocalFile();
        tmpPath.replace(m_homePath, "");
        list.append(tmpPath.split("/"));
        list.insert(0, m_homePath);
        list.removeAll("");
    }
    else
    {
        list.append(path.path().split("/"));
        if(path.isLocalFile())
            list.replace(0, "/");
        list.removeAll("");
    }
    addCrumb(list);
    calcCrumbPath(m_group.buttons().count() - 1, false);
}

bool DCrumbWidget::hasPath(QString path)
{
    return m_path.toLocalFile().contains(path);
}

bool DCrumbWidget::isInHome(QString path)
{
    return path.startsWith(m_homePath);
}

bool DCrumbWidget::isHomeFolder(QString path)
{
    return path == m_homePath;
}

bool DCrumbWidget::isRootFolder(QString path)
{
    return path == "/";
}

void DCrumbWidget::calcCrumbPath(int index, bool fixed)
{
    QRect rect = parentWidget()->rect();
    int spaceLeft = rect.width() - 48;
    int accum = 0;
    m_buttons.clear();
    int len = index;
    if(len >= m_group.buttons().count())
        len = m_group.buttons().count() - 1;

    int count = preCalcCrumb();
    qDebug() << "calc" << count;
    for(int i = len; i >= 0; i--)
    {
        DCrumbButton * button = (DCrumbButton*)m_group.buttons().at(i);
        accum += button->rect().width();

        if(accum < spaceLeft || m_buttons.count() < 3)
        {
            m_buttons.append(button);
            if(i == 0 &&
               m_group.buttons().count() > 3 &&
               len < m_group.buttons().count() - 1 && !fixed)
            {

                calcCrumbPath(count - 1, true);
                m_needArrows = true;
                break;
            }
        }
        else
        {
            m_needArrows = true;
            break;
        }
    }
}

int DCrumbWidget::preCalcCrumb()
{
    QRect rect = parentWidget()->rect();
    int spaceLeft;
    if(m_needArrows)
        spaceLeft = rect.width() - 48 - 50;
    else
        spaceLeft = rect.width() - 48;
    int accum = 0;
    int count = 0;
    for(int i = 0; i < m_group.buttons().count(); i++)
    {
        DCrumbButton * button = (DCrumbButton*)m_group.buttons().at(i);
        accum += button->rect().width();
        if(accum > spaceLeft)
            return count;
        count++;
    }
    return count;
}

void DCrumbWidget::createCrumbs()
{
    if(m_needArrows)
    {
        createArrows();
        if(rearchLeftEnd())
        {
            qDebug() << "rearchLeftEnd";
            m_leftArrow->setDisabled(true);
            m_leftArrow->hide();
            m_rightArrow->setEnabled(true);
            m_rightArrow->show();
        }
        else if(rearchRightEnd())
        {
            qDebug() << "rearchRightEnd";
            m_leftArrow->setEnabled(true);
            m_leftArrow->show();
            m_rightArrow->setDisabled(true);
            m_rightArrow->hide();
        }
        else
        {
            m_leftArrow->setEnabled(true);
            m_leftArrow->show();
            m_rightArrow->setEnabled(true);
            m_rightArrow->show();
        }
        m_buttons.push_front(m_rightArrow);
        m_buttons.push_back(m_leftArrow);
    }
    foreach(QPushButton * button, m_buttons)
    {
        m_buttonLayout->addWidget(button);
    }
}

void DCrumbWidget::createArrows()
{
    m_leftArrow = new DStateButton(":/icons/images/icons/backward_normal.png");
    m_leftArrow->setObjectName("leftArrow");
    m_leftArrow->setFixedWidth(25);
    m_leftArrow->setFixedHeight(20);
    m_leftArrow->setFocusPolicy(Qt::NoFocus);

    m_rightArrow = new DStateButton(":/icons/images/icons/forward_normal.png");
    m_rightArrow->setObjectName("rightArrow");
    m_rightArrow->setFixedWidth(25);
    m_rightArrow->setFixedHeight(20);
    m_rightArrow->setFocusPolicy(Qt::NoFocus);
    connect(m_leftArrow, &DStateButton::clicked, this, &DCrumbWidget::crumbMoveToLeft);
    connect(m_rightArrow, &DStateButton::clicked, this, &DCrumbWidget::crumbMoveToRight);
}

int DCrumbWidget::nextRightCrumbs()
{
    m_buttons.takeFirst();
    m_buttons.takeLast();
    return m_group.buttons().indexOf(m_buttons.first());
}

bool DCrumbWidget::rearchLeftEnd()
{
    int index = m_group.buttons().indexOf(m_buttons.last());
    if(index == 0)
        return true;
    else
        return false;
}

bool DCrumbWidget::rearchRightEnd()
{
    int index = m_group.buttons().indexOf(m_buttons.first());
    if(index == m_group.buttons().count() - 1)
        return true;
    else
        return false;
}

int DCrumbWidget::nextLeftCrumbs()
{
    m_buttons.takeFirst();
    m_buttons.takeLast();
    return m_group.buttons().indexOf(m_buttons.last());
}


void DCrumbWidget::buttonPressed()
{
    DCrumbButton * button = static_cast<DCrumbButton*>(sender());
    int index = button->getIndex();
    FMEvent event;
    event = WindowManager::getWindowId(window());
    event = FMEvent::CrumbButton;
    QString text;
    DCrumbButton * localButton = (DCrumbButton *)m_group.buttons().at(0);
    qDebug() << localButton->getName();
    for(int i = 1; i <= index; i++)
    {
        DCrumbButton * button = (DCrumbButton *)m_group.buttons().at(i);
        text += "/" + button->getName();
    }
    if(localButton->getName() == RECENT_ROOT)
    {
        event = DUrl::fromRecentFile(text.isEmpty() ? "/":text);
    }
    else if(localButton->getName() == COMPUTER_ROOT)
    {
        event = DUrl::fromComputerFile(text.isEmpty() ? "/":text);
    }
    else if(localButton->getName() == TRASH_ROOT)
    {
        event = DUrl::fromTrashFile(text.isEmpty() ? "/":text);
    }
    else if(localButton->getName() == m_homePath)
    {
        event = DUrl::fromLocalFile(m_homePath + text);
    }
    else
    {
        event = DUrl::fromLocalFile(text.isEmpty() ? "/":text);
    }

    emit crumbSelected(event);
}

void DCrumbWidget::crumbModified()
{
    setCrumb(m_path);
}

void DCrumbWidget::crumbMoveToLeft()
{
    int index = nextLeftCrumbs();
    prepareCrumbs(m_path);
    qDebug() << index;
    calcCrumbPath(index - 1, false);
    createCrumbs();
}

void DCrumbWidget::crumbMoveToRight()
{
    int index = nextRightCrumbs();
    qDebug() << index;
    prepareCrumbs(m_path);
    calcCrumbPath(index + 3, false);
    createCrumbs();
}
