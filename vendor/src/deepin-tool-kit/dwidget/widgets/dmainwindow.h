#ifndef DMAINWINDOW_H
#define DMAINWINDOW_H

#include "dwidget_global.h"
#include "dobject.h"

#include <QMainWindow>

DWIDGET_BEGIN_NAMESPACE

class DMainWindowPrivate;
class DTitlebar;

class LIBDTKWIDGETSHARED_EXPORT DMainWindow : public QMainWindow, public DObject
{
    Q_OBJECT

    Q_PROPERTY(int windowRadius READ windowRadius WRITE setWindowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int shadowRadius READ shadowRadius WRITE setShadowRadius NOTIFY shadowRadiusChanged)
    Q_PROPERTY(QPoint shadowOffset READ shadowOffset WRITE setShadowOffset NOTIFY shadowOffsetChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QPainterPath clipPath READ clipPath WRITE setClipPath NOTIFY clipPathChanged)
    Q_PROPERTY(QRegion frameMask READ frameMask WRITE setFrameMask NOTIFY frameMaskChanged)
    Q_PROPERTY(QMargins frameMargins READ frameMargins NOTIFY frameMarginsChanged)

public:
    explicit DMainWindow(QWidget *parent = 0);

    bool isDXcbWindow() const;

    int windowRadius() const;

    int borderWidth() const;
    QColor borderColor() const;

    int shadowRadius() const;
    QPoint shadowOffset() const;
    QColor shadowColor() const;

    QPainterPath clipPath() const;
    QRegion frameMask() const;
    QMargins frameMargins() const;

    DTitlebar *titleBar() const;

public slots:
    void setWindowRadius(int windowRadius);

    void setBorderWidth(int borderWidth);
    void setBorderColor(const QColor &borderColor);

    void setShadowRadius(int shadowRadius);
    void setShadowOffset(const QPoint &shadowOffset);
    void setShadowColor(const QColor &shadowColor);

    void setClipPath(const QPainterPath &clipPath);
    void setFrameMask(const QRegion &frameMask);

signals:
    void windowRadiusChanged();
    void borderWidthChanged();
    void borderColorChanged();
    void shadowRadiusChanged();
    void shadowOffsetChanged();
    void shadowColorChanged();
    void clipPathChanged();
    void frameMaskChanged();
    void frameMarginsChanged();

protected:
    DMainWindow(DMainWindowPrivate &dd, QWidget *parent = 0);

private:
    D_DECLARE_PRIVATE(DMainWindow)
};

DWIDGET_END_NAMESPACE

#endif // DMAINWINDOW_H
