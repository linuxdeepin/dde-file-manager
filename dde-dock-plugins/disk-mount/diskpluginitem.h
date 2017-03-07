#ifndef DISKPLUGINITEM_H
#define DISKPLUGINITEM_H

#include "constants.h"

#include <QWidget>
#include <QPixmap>

class DiskPluginItem : public QWidget
{
    Q_OBJECT

public:
    explicit DiskPluginItem(QWidget *parent = 0);

signals:
    void requestContextMenu() const;

public slots:
    void setDockDisplayMode(const Dock::DisplayMode mode);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void mousePressEvent(QMouseEvent *e);
    QSize sizeHint() const;

private:
    void updateIcon();

private:
    Dock::DisplayMode m_displayMode;

    QPixmap m_icon;
};

#endif // DISKPLUGINITEM_H
