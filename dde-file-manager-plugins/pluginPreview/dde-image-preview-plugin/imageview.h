#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QFrame>
#include <QImage>

#define MIN_SIZE QSize(400, 300)

class ImageView : public QFrame
{
    Q_OBJECT
public:
    explicit ImageView(const QString& file, QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    QImage m_img;
};

#endif // IMAGEVIEW_H
