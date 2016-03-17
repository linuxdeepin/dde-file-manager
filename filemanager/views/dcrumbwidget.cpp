#include "dcrumbwidget.h"
#include "dcrumbbutton.h"
#include <QDebug>

DCrumbWidget::DCrumbWidget(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("DCrumbWidget");
    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setSpacing(0);
    m_buttonLayout->setContentsMargins(0,0,0,0);
    setLayout(m_buttonLayout);
    setFixedHeight(20);
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
        if(!isHomeFolder(text))
            button = new DCrumbButton(m_group.buttons().size(), text, this);
        else
            button = new DCrumbButton(m_group.buttons().size(),
                                      QIcon(":/icons/images/icons/user-home-symbolic.svg"),text, this);
        button->setFocusPolicy(Qt::NoFocus);
        m_buttonLayout->addWidget(button);
        m_group.addButton(button, button->getIndex());
        connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
    }
    m_group.buttons().last()->setChecked(true);
}

void DCrumbWidget::setCrumb(const QString &path)
{
    if(path.isEmpty() || path.at(0) != '/' || hasPath(path))
        return;
    m_path = path;
    QStringList list;
    if(isInHome(path))
    {
        list.append(path.split("/"));
        list.removeAt(0);
        list.replace(0, "/home");
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
}

void DCrumbWidget::clear()
{
    qDeleteAll(m_group.buttons());
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

bool DCrumbWidget::hasPath(QString path)
{
    return m_path.contains(path);
}

bool DCrumbWidget::isInHome(QString path)
{
    return m_path.contains("/home");
}

bool DCrumbWidget::isHomeFolder(QString path)
{
    return path == "/home";
}

bool DCrumbWidget::isRootFolder(QString path)
{
    return path == "/";
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

    emit crumbSelected(text);
}
