#include "ddragwidget.h"

DDragWidget::DDragWidget(QObject *parent)
    : QDrag(parent)
{
    m_widget = new PixmapWidget();
    m_widget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_widget->setAttribute(Qt::WA_TranslucentBackground);
    m_widget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint );
    m_timer = new QTimer(this);
    m_timer->setInterval(20);
    connect(m_timer, &QTimer::timeout, this, &DDragWidget::timerEvent, Qt::DirectConnection);
}

DDragWidget::~DDragWidget()
{
    delete m_widget;
}

void DDragWidget::startDrag()
{
    qApp->installEventFilter(this);
    m_widget->show();
    m_widget->move(QCursor::pos() - m_hotspot);
    m_timer->start(20);
    exec();
    m_timer->stop();
    delete this;
}

void DDragWidget::setPixmap(const QPixmap &pixmap)
{
    m_widget->setPixmap(pixmap);
}

void DDragWidget::setHotSpot(const QPoint &hotspot)
{
    m_hotspot = hotspot;
}

bool DDragWidget::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj);
    if(e->type() == QEvent::MouseButtonRelease)
    {
        m_timer->stop();
        m_widget->hide();
        qApp->removeEventFilter(this);
        qDebug() << "mouse release";
    }
    else if(e->type() == QEvent::Drop)
    {
        m_timer->stop();
        m_widget->hide();
        qApp->removeEventFilter(this);
        qDebug() << "mouse drop";
    }
    return false;
}

void DDragWidget::timerEvent()
{
    m_widget->move(QCursor::pos() - m_hotspot);
}

PixmapWidget::PixmapWidget()
{
    setAcceptDrops(true);
}

void PixmapWidget::setPixmap(const QPixmap &pixmap)
{
    if(pixmap.isNull())
        return;
    m_pixmap = pixmap;
    resize(m_pixmap.width(), m_pixmap.height());
}

void PixmapWidget::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept();
}

void PixmapWidget::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void PixmapWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(0,0, m_pixmap);
    painter.end();
}
