#include "progressbox.h"
#include <QPen>
#include <QPainter>
#include <QLine>
#include <QtMath>
#include <QDebug>

ProgressBox::ProgressBox(QWidget *parent) : ProgressLine(parent)
{
    m_taskTimer = new QTimer(this);
    m_updateTimer = new QTimer(this);
    m_taskAni = new QVariantAnimation(this);
    initConnections();
    m_taskTimer->setInterval(100);
    m_taskTimer->setSingleShot(true);

    m_updateTimer->setInterval(10);
    m_updateTimer->start();
}

void ProgressBox::initConnections()
{
    connect(m_taskTimer, &QTimer::timeout, this, &ProgressBox::taskTimeOut);
    connect(m_updateTimer, &QTimer::timeout, this, &ProgressBox::updateAnimation);
}

void ProgressBox::updateAnimation()
{
    //bleds generation
    if(m_bleds.count() < 3){
        Bled b;
        b.pos.setX(random()%width());
        b.pos.setY(height());
        m_bleds << b;
    }

    for(int i = 0; i < m_bleds.size(); i++){
        Bled b = m_bleds.at(i);
        b.pos.setY(b.pos.y() - 1);
        if(b.pos.y() < (height() - value()*height()/100)){
            m_bleds.removeAt(i);
            i--;
            continue;
        }
        m_bleds.replace(i, b);
    }
    update();
}

void ProgressBox::taskTimeOut()
{
    m_taskAni->stop();
    m_taskAni->setDuration(10000); //10 seconds
    m_taskAni->setStartValue(qreal(0));
    m_taskAni->setEndValue(qreal(99));
    m_taskAni->setEasingCurve(QEasingCurve::OutCirc);
    connect(m_taskAni, &QVariantAnimation::valueChanged, this, [=] (const QVariant& val){
            setValue(val.toReal());
    });
    m_taskAni->start();
}

void ProgressBox::startTask()
{
    m_taskTimer->start();
}

void ProgressBox::finishedTask(const bool result)
{
    m_taskAni->stop();
    QVariantAnimation* ani = new QVariantAnimation(this);
    ani->setDuration(500);
    ani->setStartValue(value());
    ani->setEndValue(qreal(100));
    ani->setEasingCurve(QEasingCurve::InQuint);
    connect(ani, &QVariantAnimation::valueChanged, this, [=] (const QVariant& val){
            setValue(val.toReal());
    });
    connect(ani, &QVariantAnimation::finished, [=]{
        ani->deleteLater();
        setValue(0);
        emit finished(result);
    });
    ani->start();
}

void ProgressBox::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    QFont f;
    f.setPixelSize(25);
    p.setFont(f);

    //draw bg
    QPainterPath bgPath;
    bgPath.addRoundedRect(0, 0, width(), height(), width()/2, height()/2);
    p.fillPath(bgPath, QColor(0, 0, 0, 25));
    p.setClipPath(bgPath);

    //draw imgs
    static int cox = 0;
    cox++;
    int baseY = height() - value()*height()/100;
    int maxY = 5;
    QPolygonF pl1;
    pl1 << QPoint(0, height());
    for(int i = 0; i<width(); i++){
        float py = (float)maxY*qSin(qDegreesToRadians((float)(i - cox*1.7)*(400)/width()));
        float realy = baseY - py;
        pl1 << QPointF(i, realy);
    }
    pl1 << QPoint(width(), height());

    QPolygonF pl2;
    pl2 << QPoint(0, height());
    for(int i = 0; i<width(); i++){
        float py = (float)maxY*qSin(qDegreesToRadians((float)(i - cox + 15)*(400)/width()));
        float realy = baseY - py - 3;
        pl2 << QPointF(i, realy);
    }
    pl2 << QPoint(width(), height());

    QPainterPath path2;
    path2.addPolygon(pl2);
    p.fillPath(path2, QColor(30, 160, 233));

    QPainterPath path1;
    path1.addPolygon(pl1);
    p.fillPath(path1, QColor(50, 199, 255));

    QPainterPath bledPath;
    foreach (const Bled& b, m_bleds) {

        qreal s = 15 - 10*b.pos.y()/height();
        bledPath.addRoundedRect(b.pos.x(), b.pos.y(), s, s, s/2, s/2);
    }
    p.fillPath(bledPath, QColor(255, 255, 255, 100));

    //draw text
    QString progressStr = QString::number((int)value()) + "%";
    QFontMetrics fm(f);
    QRect rect;
    rect.setWidth(fm.width(progressStr));
    rect.setHeight(25);
    rect.moveCenter(QPoint(width()/2, height()/2));
    pen.setColor(Qt::white);
    p.setPen(pen);
    p.drawText(rect, progressStr);

    QLabel::paintEvent(event);
}
