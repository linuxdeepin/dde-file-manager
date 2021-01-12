

#include <QDebug>

#include "droundbutton.h"


static constexpr QMarginsF MARGINS{ 1.0, 1.0, 1.0, 1.0};


DRoundButton::DRoundButton(const std::initializer_list<QPair<QColor, QColor>>& colors, QWidget* const parent)
                 :QFrame{ parent },
                  m_allStatusColors{ colors }
{
}

void DRoundButton::setRadiusF(const double& radius)noexcept
{
    this->m_radius.first = radius;
    this->setFixedSize(static_cast<int>(radius), static_cast<int>(radius));
}

void DRoundButton::setRadius(const std::size_t& radius)noexcept
{
    this->m_radius.second = radius;
    this->setFixedSize(static_cast<int>(radius), static_cast<int>(radius));
}

void DRoundButton::setCheckable(bool checkable)noexcept
{
    m_checkable = checkable;
}

void DRoundButton::setChecked(bool checked)
{
    if (!m_checkable)
        return;

    if (checked) {
        if (m_paintStatus == PaintStatus::checked)
            return;

        setPaintStatus(PaintStatus::checked);
    } else if (m_paintStatus == PaintStatus::checked) {
        setPaintStatus(PaintStatus::normal);
    } else {
        return;
    }

    emit checkedChanged();
}

bool DRoundButton::isChecked() const
{
    return m_paintStatus == PaintStatus::checked;
}

bool DRoundButton::isHovered() const
{
    return testAttribute(Qt::WA_UnderMouse);
}

QColor DRoundButton::color() const
{
    return m_allStatusColors.first().second;
}

void DRoundButton::enterEvent(QEvent* event)
{
    if (!isChecked()) {
        setPaintStatus(PaintStatus::hover);
    }

    event->accept();

    emit enter();
}

void DRoundButton::leaveEvent(QEvent* event)
{
    if (!isChecked()) {
        setPaintStatus(PaintStatus::normal);
    }

    event->accept();

    emit leave();
}

void DRoundButton::mousePressEvent(QMouseEvent* event)
{
    if (!isChecked()) {
        setPaintStatus(PaintStatus::pressed);
    }

    QFrame::mousePressEvent(event);
}

void DRoundButton::mouseReleaseEvent(QMouseEvent* event)
{
    setChecked(!isChecked());

    emit click(m_allStatusColors[0].second);
    QFrame::mouseReleaseEvent(event);
}



void DRoundButton::paintEvent(QPaintEvent* paintEvent)
{
    (void)paintEvent;

    QPainter painter{ this };
    double theRadius{ static_cast<double>( m_radius.first > m_radius.second ?
                                               m_radius.first : m_radius.second ) };

    QPen pen{ Qt::SolidLine };

    pen.setWidthF(1.0);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setRenderHint(QPainter::Antialiasing, true);

    switch(m_paintStatus)
    {
    case PaintStatus::normal:
    {
        QRectF rectF{ 4, 4, theRadius-8, theRadius-8 };
        QBrush brush{ m_allStatusColors[0].second };

        pen.setColor( m_allStatusColors[0].first );
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawEllipse(rectF.marginsRemoved(MARGINS));

        break;
    }
    case PaintStatus::hover:
    {
        QRectF rectF{ 0, 0, theRadius, theRadius };

        pen.setColor( m_allStatusColors[1].first );
        painter.setPen(pen);
        painter.drawEllipse(rectF.marginsRemoved(MARGINS));

        QBrush brush{ m_allStatusColors[0].second };
        rectF = QRectF{ 4, 4, theRadius-8, theRadius-8 };
        pen.setColor( m_allStatusColors[0].first );
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawEllipse(rectF.marginsRemoved(MARGINS));

        break;
    }
    case PaintStatus::pressed:
    {
        QRectF rectF{ 0, 0, theRadius, theRadius };
        QBrush brush{ m_allStatusColors[1].second };

        pen.setColor( m_allStatusColors[1].first );
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawEllipse(rectF.marginsRemoved(MARGINS));


        rectF = QRectF{ 4, 4, theRadius-8, theRadius-8 };
        pen.setColor( m_allStatusColors[0].first );
        brush.setColor( m_allStatusColors[0].second );
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawEllipse(rectF.marginsRemoved(MARGINS));

        break;
    }
    case PaintStatus::checked:
    {
        QRectF rectF{ 0, 0, theRadius, theRadius };
        QBrush brush{ m_allStatusColors[1].second };

        pen.setColor( m_allStatusColors[1].first );
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawEllipse(rectF.marginsRemoved(MARGINS));


        rectF = QRectF{ 4, 4, theRadius-8, theRadius-8 };
        pen.setColor( m_allStatusColors[0].first );
        brush.setColor( m_allStatusColors[0].second );
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawEllipse(rectF.marginsRemoved(MARGINS));

        break;
    }
    default:
    {
        break;
    }
    }

}

void DRoundButton::setPaintStatus(DRoundButton::PaintStatus status)
{
    if (m_paintStatus == status)
        return;

    m_paintStatus = status;

    update();
}

