#ifndef WALLPAPERITEM_H
#define WALLPAPERITEM_H

#include <QFrame>
#include <QFutureWatcher>
#include <QPropertyAnimation>

class QLabel;
class Button;
class AppearanceDaemonInterface;
class WrapperWidget;
class WallpaperItem : public QFrame
{
    Q_OBJECT
public:
    WallpaperItem(QFrame *parent = 0, const QString &path = "");
    ~WallpaperItem();

    void slideUp();
    void slideDown();

    QString getPath() const;
    void setPath(const QString &path);

    bool getDeletable() const;
    void setDeletable(bool deletable);

    void setOpacity(qreal opacity);

    QRect conentImageGeometry() const;

signals:
    void pressed();
    void hoverIn();
    void hoverOut();
    void desktopButtonClicked();
    void lockButtonClicked();

public slots:
    void thumbnailFinished();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QString m_path;
    bool m_deletable;

    WrapperWidget * m_wrapper = NULL;
    Button * m_desktopButton = NULL;
    Button * m_lockButton = NULL;
    QPropertyAnimation * m_upAnim = NULL;
    QPropertyAnimation * m_downAnim = NULL;

    QFutureWatcher<QPixmap> * m_thumbnailerWatcher = NULL;

    void initUI();
    void initAnimation();
    void initPixmap();
};

#endif // WALLPAPERITEM_H
