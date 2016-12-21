#include "dcrumbbutton.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

DCrumbButton::DCrumbButton(int index, const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    m_index = index;
    m_name = text;
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("DCrumbButton");
}

DCrumbButton::DCrumbButton(int index, const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton(icon, "", parent)
{
    m_index = index;
    m_name = text;
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("DCrumbButton");
}

int DCrumbButton::getIndex()
{
    return m_index;
}

QString DCrumbButton::getName()
{
    return m_name;
}

void DCrumbButton::setItem(QListWidgetItem *item)
{
    m_item = item;
}

void DCrumbButton::setListWidget(QListWidget *widget)
{
    m_listWidget = widget;
}

QListWidgetItem *DCrumbButton::getItem()
{
    return m_item;
}

DUrl DCrumbButton::url() const
{
    return m_url;
}

void DCrumbButton::setUrl(const DUrl &url)
{
    m_url = url;
}

void DCrumbButton::paintEvent(QPaintEvent *e)
{

    QPushButton *button = qobject_cast<QPushButton*>(m_listWidget->itemWidget(m_listWidget->item(m_index + 1)));

    if (!isChecked() && (!button || (button &&  !button->isChecked()))) {
        QPainter painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);

        double w = width();
        double h = height();
        QPolygon polygon;
        polygon << QPoint(w, 2);
        polygon << QPoint(w, h - 2);
        QPen pen(QColor(0, 0, 0, 24));
        pen.setWidthF(1);
        painter.setPen(pen);
        painter.drawPolygon(polygon);
        painter.end();
    }


    QPushButton::paintEvent(e);
}

void DCrumbButton::mouseMoveEvent(QMouseEvent *e)
{
    return QWidget::mouseMoveEvent(e);
}

void DCrumbButton::mousePressEvent(QMouseEvent *e)
{
    oldGlobalPos = e->globalPos();
    if(e->button() == Qt::LeftButton)
        return QPushButton::mousePressEvent(e);

}

void DCrumbButton::mouseReleaseEvent(QMouseEvent *e)
{
    const QPoint &difference = e->globalPos() - oldGlobalPos;

    if (qAbs(difference.x()) < 10 && qAbs(difference.y()) < 10 && e->button() == Qt::LeftButton) {
        QPushButton::mouseReleaseEvent(e);
    }
}

DCrumbIconButton::DCrumbIconButton(int index, const QIcon &normalIcon, const QIcon &hoverIcon, const QIcon &checkedIcon, const QString &text, QWidget *parent)
    : DCrumbButton(index, normalIcon, text, parent)
{
    m_normalIcon = normalIcon;
    m_hoverIcon = hoverIcon;
    m_checkedIcon = checkedIcon;
    setObjectName("DCrumbIconButton");
}

void DCrumbIconButton::checkStateSet()
{
    if(isChecked()){
        setIcon(m_checkedIcon);
    }
    else{
        setIcon(m_normalIcon);
    }
    qDebug() << "Check state changed: " << isChecked();
}

void DCrumbIconButton::nextCheckState()
{
    setChecked(!isChecked());
    if(isChecked()){
        setIcon(m_checkedIcon);
    }
    else{
        setIcon(m_normalIcon);
    }
    qDebug() << "Next check state changed: " << isChecked();
}
