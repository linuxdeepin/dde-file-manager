#include "progressbox.h"
#include <QPen>
#include <QPainter>
#include <QLine>
#include <QtMath>
#include <QDebug>
#include <QLabel>
#include <QImage>
#include <QVBoxLayout>

ProgressBox::ProgressBox(QWidget *parent) :
    ProgressLine(parent),
    m_taskTimer(new QTimer(this)),
    m_updateTimer(new QTimer(this)),
    m_taskAni(new QVariantAnimation(this)),
    m_textLabel(new QLabel(this)),
    m_BackImage(new QImage(":/icoms/icons/water_back.png")),
    m_FrontImage(new QImage(":/icoms/icons/water_front.png")),
    m_shadow(new QImage(":/icoms/icons/water_shadow.png"))
{
    m_taskTimer->setInterval(100);
    m_taskTimer->setSingleShot(true);

    m_updateTimer->setInterval(66);
    m_updateTimer->start();

    initUI();
    initConnections();
}

void ProgressBox::initConnections()
{
    connect(m_taskTimer, &QTimer::timeout, this, &ProgressBox::taskTimeOut);
    connect(m_updateTimer, &QTimer::timeout, this, &ProgressBox::updateAnimation);
}

void ProgressBox::initUI()
{
    m_textLabel->setText("0");
    m_textLabel->setObjectName("Progress");
    m_textLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_textLabel, 0, Qt::AlignCenter);
    setLayout(mainLayout);

    setFixedSize(128, 128);
    setMask(QRegion(0, 0, this->width(), this->height(), QRegion::Ellipse));

    m_FrontWidth = m_FrontImage->width() - 1;
    m_BackWidth = m_BackImage->width() - 1;
    m_ForntXOffset1 = 0;
    m_ForntXOffset2 = -m_FrontWidth;
    m_BackXOffset1 = m_BackWidth / 2;
    m_BackXOffset2 = -m_BackWidth / 2;
    m_Pop7YOffset = 100;
    m_Pop8YOffset = 100;
    m_Pop11YOffset = 100;
}

void ProgressBox::updateAnimation()
{
    //bleds generation
    int zeroY = 104;
    int fullY = 8;
    m_YOffset = zeroY - value() * (zeroY - fullY) / 100 ;

    auto wmax = 114;
    auto wmin = 12;

    double speed = 1.0 * value() / 100;

    m_ForntXOffset1 += 2;
    m_ForntXOffset2 += 2;
    if (m_ForntXOffset1 >= wmax) {
        m_ForntXOffset1 = -m_FrontWidth;
    }
    if (m_ForntXOffset2 >= wmax) {
        m_ForntXOffset2 = -m_FrontWidth;
    }

    m_BackXOffset1 -= 3;
    m_BackXOffset2 -= 3;
    if (m_BackXOffset1 < -m_BackWidth) {
        m_BackXOffset1 = wmax;
    }
    if (m_BackXOffset2 < -m_BackWidth) {
        m_BackXOffset2 = wmax;
    }

    if (m_YOffset < -wmin) {
        m_YOffset = 88;
    }

    m_Pop7YOffset -= 0.8 * speed;
    if (m_Pop7YOffset < m_YOffset + 10) {
        m_Pop7YOffset = 100;
    }
    m_Pop7XOffset = 35 + 14 + qSin((100 - m_Pop7YOffset) * 2 * 3.14 / 40) * 8;

    m_Pop8YOffset -= 1.2 * speed;
    if (m_Pop8YOffset < m_YOffset + 10) {
        m_Pop8YOffset = 100;
    }
    m_Pop8XOffset = 45 + 14 + qCos((100 - m_Pop8YOffset) * 2 * 3.14 / 40) * 9
                    * (100 - m_Pop8XOffset) / 60;

    m_Pop11YOffset -= 1.6 * speed;
    if (m_Pop11YOffset < m_YOffset + 10) {
        m_Pop11YOffset = 100;
    }
    m_Pop11XOffset = 55 + 14 + qSin((100 - m_Pop11YOffset) * 2 * 3.14 / 50)
                     * 11 * (100 - m_Pop11YOffset) / 60;

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
            m_textLabel->setText(QString::number(val.toInt()));
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
    ani->setDuration(300);
    ani->setStartValue(value());
    ani->setEndValue(qreal(100));
    connect(ani, &QVariantAnimation::valueChanged, this, [=] (const QVariant& val){
            setValue(val.toReal());
            m_textLabel->setText(QString::number(val.toInt()));
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
    auto rect = this->rect();

    auto w = 100;
    auto h = 100;

    // draw backgroud
    QPainterPath outerPath;
    outerPath.addEllipse(rect.center(), w / 2, h / 2);
    p.fillPath(outerPath, QBrush(qRgb(0xcb, 0xe0, 0xff)));

    // draw water
    p.drawImage(m_BackXOffset1, m_YOffset, *m_BackImage);
    p.drawImage(m_BackXOffset2, m_YOffset, *m_BackImage);
    p.drawImage(m_ForntXOffset1, m_YOffset, *m_FrontImage);
    p.drawImage(m_ForntXOffset2, m_YOffset, *m_FrontImage);

    //drwa pop

    if (value() > 25) {
        QPainterPath pop7;
        pop7.addEllipse(m_Pop7XOffset, m_Pop7YOffset, 7, 7);
        p.fillPath(pop7, QColor(77, 208, 255));

        QPainterPath pop8;
        pop8.addEllipse(m_Pop8XOffset, m_Pop8YOffset, 8, 8);
        p.fillPath(pop8, QColor(77, 208, 255));

        QPainterPath pop11;
        pop11.addEllipse(m_Pop11XOffset, m_Pop11YOffset, 11, 11);
        p.fillPath(pop11, QColor(77, 208, 255));
    }
    auto offset = 14;
    // draw boder
    QPen borderPenBk(QColor(0xff, 0xff, 0xff, 0xff), 16);
    p.setPen(borderPenBk);
    p.drawEllipse(offset - 8,  offset - 8, w + 16, h + 16);

    // draw boder
    QPen borderPen1(QColor(0xff, 0xff, 0xff, 0x80), 2);
    p.setPen(borderPen1);
    p.drawEllipse(5 + offset, 5 + offset, w - 10, h - 10);

    QPen borderPen(QColor(qRgb(43, 146, 255)), 2);
    p.setPen(borderPen);
    p.drawEllipse(3 + offset, 3 + offset, w - 6, h - 6);

    QPen smothBorderPen(Qt::white, 4);
    p.setPen(smothBorderPen);
    p.drawEllipse(offset, offset, w, h);

    p.drawImage(0, 0, *m_shadow);

    QLabel::paintEvent(event);
}
