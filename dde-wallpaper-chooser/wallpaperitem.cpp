#include "wallpaperitem.h"
#include "constants.h"
#include "thumbnailmanager.h"
#include "button.h"

#include <QEvent>
#include <QPixmap>
#include <QLabel>
#include <QUrl>
#include <QDebug>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

static QPixmap ThumbnailImage(const QString &path)
{
    QUrl url = QUrl::fromPercentEncoding(path.toUtf8());
    QString realPath = url.toLocalFile();

    ThumbnailManager * tnm = ThumbnailManager::instance();
    QPixmap pix = QPixmap(realPath).scaled(QSize(ItemWidth, ItemHeight), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    if (pix.width() > ItemWidth || pix.height() > ItemHeight) {
        pix = pix.copy((pix.width() - ItemWidth) / 2, (pix.height() - ItemHeight) / 2, ItemWidth, ItemHeight);
    }

    tnm->replace(QUrl::toPercentEncoding(path), pix);

    return pix;
}

class WrapperWidget : public QWidget
{
public:
    explicit WrapperWidget(QWidget *parent = 0) : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        QPainter pa(this);

        pa.setOpacity(m_opacity);
        pa.drawPixmap(m_pixmapBoxGeometry.topLeft(), m_pixmap, m_pixmapBoxGeometry.united(event->rect()));
    }

private:
    QPixmap m_pixmap;
    qreal m_opacity = 1;
    QRect m_pixmapBoxGeometry;

    friend class WallpaperItem;
};

WallpaperItem::WallpaperItem(QFrame *parent, const QString &path) :
    QFrame(parent),
    m_path(path),
    m_thumbnailerWatcher(new QFutureWatcher<QPixmap>(this))
{
    initUI();
    initPixmap();
    initAnimation();

    connect(m_thumbnailerWatcher, &QFutureWatcher<QPixmap>::finished, this, &WallpaperItem::thumbnailFinished);
}

WallpaperItem::~WallpaperItem()
{

}

void WallpaperItem::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameShape(QFrame::NoFrame);

    m_wrapper = new WrapperWidget(this);
    m_wrapper->setFixedSize(ItemWidth, ItemHeight * 2);
    m_wrapper->setAttribute(Qt::WA_TranslucentBackground);

    QFrame * buttonArea = new QFrame(m_wrapper);
    buttonArea->setFixedSize(ItemWidth, ItemHeight);
    buttonArea->move(0, ItemHeight);

    QVBoxLayout * buttonLayout = new QVBoxLayout(buttonArea);
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);

    m_desktopButton = new Button;
    m_desktopButton->setText(tr("Only desktop"));
    m_desktopButton->setAttract(false);
    m_lockButton = new Button;
    m_lockButton->setText(tr("Only lock screen"));
    m_lockButton->setAttract(false);

    buttonLayout->addWidget(m_desktopButton);
    buttonLayout->addWidget(m_lockButton);

    connect(m_desktopButton, &Button::clicked, this, &WallpaperItem::desktopButtonClicked);
    connect(m_lockButton, &Button::clicked, this, &WallpaperItem::lockButtonClicked);
}

void WallpaperItem::initAnimation()
{
    m_upAnim = new QPropertyAnimation(m_wrapper, "pos");
    m_upAnim->setDuration(300);
    m_upAnim->setStartValue(QPoint(0, 0));
    m_upAnim->setEndValue(QPoint(0, -ItemHeight));

    m_downAnim = new QPropertyAnimation(m_wrapper, "pos");
    m_downAnim->setDuration(300);
    m_downAnim->setStartValue(QPoint(0, -ItemHeight));
    m_downAnim->setEndValue(QPoint(0, 0));
}

void WallpaperItem::initPixmap()
{
    ThumbnailManager * tnm = ThumbnailManager::instance();

    if (!tnm->find(QUrl::toPercentEncoding(m_path), &m_wrapper->m_pixmap)
            || m_wrapper->m_pixmap.size() != QSize(ItemWidth, ItemHeight)) {
        QFuture<QPixmap> f = QtConcurrent::run(ThumbnailImage, m_path);
        m_thumbnailerWatcher->setFuture(f);
    }
}

void WallpaperItem::slideUp()
{
    if (m_upAnim->endValue().toPoint() != m_wrapper->pos()) {
        m_upAnim->start();
    }
}

void WallpaperItem::slideDown()
{
    if (m_downAnim->endValue().toPoint() != m_wrapper->pos()) {
        m_downAnim->start();
    }
}

QString WallpaperItem::getPath() const
{
    QUrl url = QUrl::fromPercentEncoding(m_path.toUtf8());
    return url.toLocalFile();
}

void WallpaperItem::thumbnailFinished()
{
    QFuture<QPixmap> f = m_thumbnailerWatcher->future();
    m_wrapper->m_pixmap = f.result();
    m_wrapper->update();
}

void WallpaperItem::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
        emit pressed();
}

void WallpaperItem::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    emit hoverIn();
}

void WallpaperItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    emit hoverOut();
}

void WallpaperItem::resizeEvent(QResizeEvent *event)
{
    const QPoint &offset = QPoint((event->size().width() - ItemWidth) / 2, (event->size().height() - ItemHeight) / 2);

    m_wrapper->setFixedWidth(width());
    m_wrapper->m_pixmapBoxGeometry = QRect(offset, QSize(ItemWidth, ItemHeight));

    QFrame::resizeEvent(event);
}

bool WallpaperItem::getDeletable() const
{
    return m_deletable;
}

void WallpaperItem::setDeletable(bool deletable)
{
    m_deletable = deletable;
}

void WallpaperItem::setOpacity(qreal opacity)
{
    if (m_wrapper->m_opacity == opacity)
        return;

    m_wrapper->m_opacity = opacity;
    m_wrapper->update();
}

QRect WallpaperItem::conentImageGeometry() const
{
    return m_wrapper->m_pixmapBoxGeometry;
}

void WallpaperItem::setPath(const QString &path)
{
    m_path = path;
}
