#include <QDebug>
#include <QStandardPaths>

#include "dcrumbwidget.h"
#include "dcrumbbutton.h"
#include "../app/fmevent.h"
#include "windowmanager.h"
#include "dstatebutton.h"

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

void DCrumbWidget::prepareCrumbs(const QString &path)
{
    if(path.isEmpty() || path.at(0) != '/')
        return;
    m_path = path;
    QStringList list;
    if(isInHome(path))
    {
        QString tmpPath = path;
        tmpPath.replace(m_homePath, "");
        list.append(tmpPath.split("/"));
        list.insert(0, m_homePath);
        list.removeAll("");
    }
    else
    {
        list.append(path.split("/"));
        list.replace(0, "/");
        list.removeAll("");
    }
    clear();
    addCrumb(list);
    if(m_prevCheckedId != -1)
        m_group.button(m_prevCheckedId)->setChecked(true);
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

void DCrumbWidget::setCrumb(const QString &path)
{
    if(path.isEmpty() || path.at(0) != '/')
        return;
    m_path = path;
    QStringList list;
    if(isInHome(path))
    {
        QString tmpPath = path;
        tmpPath.replace(m_homePath, "");
        list.append(tmpPath.split("/"));
        list.insert(0, m_homePath);
        list.removeAll("");
    }
    else
    {
        list.append(path.split("/"));
        list.replace(0, "/");
        list.removeAll("");
    }
    m_needArrows = false;
    clear();
    addCrumb(list);
    calcCrumbPath(list.count() - 1, false);
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
        return m_path;
    m_group.buttons().last()->deleteLater();
    m_group.buttons().removeLast();
    m_group.buttons().last()->setChecked(true);

    QString tabText;
    QStringList list = m_path.split("/");
    list.replace(0, "/");
    list.removeAll("");

    tabText += list.at(0);
    for(int i = 1; i < m_group.buttons().size(); i++)
        tabText += list.at(i) + "/";

    if(tabText != "/")
        tabText.remove(tabText.length() -1 ,1);

    m_path = tabText;
    return m_path;
}

QString DCrumbWidget::path()
{
    return m_path;
}

bool DCrumbWidget::hasPath(QString path)
{
    return m_path.contains(path);
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
            m_rightArrow->setEnabled(true);
        }
        else if(rearchRightEnd())
        {
            qDebug() << "rearchRightEnd";
            m_leftArrow->setEnabled(true);
            m_rightArrow->setDisabled(true);
        }
        else
        {
            m_leftArrow->setEnabled(true);
            m_rightArrow->setEnabled(true);
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
    QString text;
    DCrumbButton * localButton = (DCrumbButton *)m_group.buttons().at(0);
    if(isRootFolder(localButton->getName()))
        text = "/";
    else
        text = localButton->getName() + "/";
    for(int i = 1; i <= index; i++)
    {
        DCrumbButton * button = (DCrumbButton *)m_group.buttons().at(i);
        text += button->getName() + "/";
    }
    if(!isRootFolder(text))
        text.remove(text.count() -1, 1);
    FMEvent event;
    event = DUrl::fromLocalFile(text);
    event = WindowManager::getWindowId(window());
    event = FMEvent::CrumbButton;
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
