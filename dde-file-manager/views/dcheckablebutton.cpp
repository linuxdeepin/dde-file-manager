#include "dcheckablebutton.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDrag>
#include <QMimeData>

DCheckableButton::DCheckableButton(const QString &normal,
                                   const QString &hover,
                                   const QString &text,
                                   QWidget *parent):
    QPushButton(parent),
    m_text(text),
    m_normal(normal),
    m_hover(hover)
{
    setFlat(true);
    setCheckable(true);
    setObjectName("DCheckableButton");
    initUI();
    setAcceptDrops(true);
}

DCheckableButton::~DCheckableButton()
{

}

void DCheckableButton::initUI()
{
    setFixedHeight(30);
    setFocusPolicy(Qt::NoFocus);
    QIcon icon(m_normal);
    m_iconLabel = new QLabel;
    m_iconLabel->setPixmap(icon.pixmap(16,16));
    m_iconLabel->setFixedSize(16, 16);
    m_textLabel = new QLabel;
    m_textLabel->setObjectName("LeftSideBarTextLabel");
    m_textLabel->setText(m_text);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_textLabel->setFixedHeight(30);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addWidget(m_textLabel);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 0, 0, 0);
    setLayout(mainLayout);
}

void DCheckableButton::setUrl(const QString &url)
{
    m_url = url;
}

QString DCheckableButton::getUrl()
{
    return m_url;
}

void DCheckableButton::nextCheckState()
{
    QPushButton::nextCheckState();
    if(isChecked())
    {
        QIcon icon(m_hover);
        m_iconLabel->setPixmap(icon.pixmap(16,16));
    }
    else
    {
        QIcon icon(m_normal);
        m_iconLabel->setPixmap(icon.pixmap(16,16));
    }
}

void DCheckableButton::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
    qDebug() << "button drag";
}

void DCheckableButton::dropEvent(QDropEvent *e)
{
    Q_UNUSED(e)
}

void DCheckableButton::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
    QMimeData *data = new QMimeData;

    QDrag *drag = new QDrag(this);
    drag->setMimeData(data);
    drag->start();

    qDebug() << "hahah";
}

void DCheckableButton::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}


