#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QSharedPointer>

class CanvasGridView;
class BackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BackgroundWidget(QWidget *parent = nullptr);
    ~BackgroundWidget() override;
    void setPixmap(const QPixmap &pixmap);
    void paintEvent(QPaintEvent *event) override;
    void setView(const QSharedPointer<CanvasGridView> &);
    inline QPixmap pixmap() const {return m_pixmap;}
    void setAccessableInfo(const QString& info);
private:
    QPixmap m_pixmap;
    QPixmap m_noScalePixmap;
    QSharedPointer<CanvasGridView> m_view; //保有view的指针，防止背景释放时，连带一起释放view
};

typedef QSharedPointer<BackgroundWidget> BackgroundWidgetPointer;
#endif // BACKGROUNDWIDGET_H
