#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>
#include <QPixmap>

class BackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BackgroundWidget(QWidget *parent = nullptr);
    void setPixmap(const QPixmap &pixmap);
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void setVisible(bool visible) Q_DECL_OVERRIDE;

private:
    QPixmap m_pixmap;
    QPixmap m_noScalePixmap;
};

typedef QSharedPointer<BackgroundWidget> BackgroundWidgetPointer;
#endif // BACKGROUNDWIDGET_H
