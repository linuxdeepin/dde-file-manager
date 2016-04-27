#include <QDebug>
#include <QStandardPaths>
#include <QScrollBar>

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
    createArrows();
    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName("DCrumbList");
    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->addWidget(m_leftArrow);
    m_buttonLayout->addWidget(m_listWidget);
    m_buttonLayout->addWidget(m_rightArrow);
    m_buttonLayout->setContentsMargins(0,0,0,0);
    m_buttonLayout->setSpacing(0);
    setLayout(m_buttonLayout);
    setObjectName("DCrumbWidget");
    m_listWidget->setFlow(QListWidget::LeftToRight);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   
    setFixedHeight(20);
    setMinimumWidth(50);

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
    }
    else if(path.isComputerFile())
    {
        addComputerCrumb();
    }
    else if(path.isTrashFile())
    {
        addTrashCrumb();
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
    m_listWidget->clear();
    m_prevCheckedId = m_group.checkedId();
    qDeleteAll(m_group.buttons());
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

void DCrumbWidget::createCrumbs()
{
    m_crumbTotalLen = 0;
    foreach(QAbstractButton * button, m_group.buttons())
    {
        QListWidgetItem * item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(button->size().width(), 18));
        m_listWidget->setItemWidget(item, button);
        DCrumbButton * localButton = (DCrumbButton *)button;
        localButton->setItem(item);
        m_items.append(item);
        m_crumbTotalLen += button->size().width();
    }
    checkArrows();
    m_listWidget->scrollToItem(m_items.last(), QListWidget::PositionAtBottom);
    m_listWidget->setHorizontalScrollMode(QListWidget::ScrollPerPixel);
    m_listWidget->horizontalScrollBar()->setPageStep(m_listWidget->width());
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

void DCrumbWidget::checkArrows()
{
    if(m_crumbTotalLen < m_listWidget->width())
    {
        m_leftArrow->hide();
        m_rightArrow->hide();
    }
    else
    {
        m_leftArrow->show();
        m_rightArrow->show();
    }
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

    m_listWidget->scrollToItem(button->getItem());
    emit crumbSelected(event);
}

void DCrumbWidget::crumbModified()
{
    setCrumb(m_path);
}

void DCrumbWidget::crumbMoveToLeft()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    m_listWidget->scrollToItem(m_listWidget->itemAt(0,0));
}

void DCrumbWidget::crumbMoveToRight()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    m_listWidget->scrollToItem(m_listWidget->itemAt(m_listWidget->width() - 1,0));
}

void DCrumbWidget::resizeEvent(QResizeEvent *e)
{
    checkArrows();
    QFrame::resizeEvent(e);
}
