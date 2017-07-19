#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QLabel>

class ImageView : public QLabel
{
    Q_OBJECT
public:
    explicit ImageView(const QString &fileName, QWidget *parent = 0);

    void setFile(const QString &fileName);
    QSize sourceSize() const;

private:
    QSize m_sourceSize;
};

#endif // IMAGEVIEW_H
