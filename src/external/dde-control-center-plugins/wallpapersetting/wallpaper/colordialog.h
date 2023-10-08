// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <DAbstractDialog>
#include <DIconButton>

#include <QVBoxLayout>
#include <QLabel>
#include <QImage>
#include <QSlider>

class QDBusObjectPath;

namespace dfm_wallpapersetting {

class ColorLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ColorLabel(QWidget *parent = nullptr);
    bool pickColor(QPoint pos, QColor &c) const;
    void setCurrent(const QColor &c);
    QColor current() const;
    QRect insideGeometry() const;
signals:
    void colorPicked(const QColor &c);
public slots:
    void setHue(qreal hue);
protected:
    void updateImage();
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    QImage image;
    qreal hue = 0;
    bool pressed = false;
    QPoint currentPos;
};

class ColorSlider : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSlider(QWidget *parent = nullptr);
    bool pickColor(int x, qreal &h) const;
    void updateImage();
    void setValue(qreal h);
protected:
    QRect handleRect() const;
    QRect imageRect() const;
signals:
    void valueChanged(qreal h);
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QImage image;
    int currentX = 0;
    bool pressed = false;
    int clickOffset = 0;
    bool onHandle = false;
};

class ColorEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ColorEditor(QWidget *parent = nullptr);
public:
    inline QRect prevewRect() const {
        return QRect(0, 0, 24, 24);
    }
    void setCurrent(const QColor &c);
    inline QColor getCurrent() const {
        return current;
    }
signals:
    void colorEdited(const QColor &);
protected slots:
    void onNameChanged();
    void onNodeChanged();
protected:
    QLineEdit *create(const QString &text, const QSize &sz, QBoxLayout *lay);
    void paintEvent(QPaintEvent *e) override;
private:
    QColor current;
    QLineEdit *hexEditor = nullptr;
    QLineEdit *rEditor = nullptr;
    QLineEdit *gEditor = nullptr;
    QLineEdit *bEditor = nullptr;
};

class ColorDialog : public Dtk::Widget::DAbstractDialog
{
    Q_OBJECT
public:
    explicit ColorDialog(QWidget *parent = nullptr);
    void setColor(const QColor &);
    QColor color() const;
signals:
protected slots:
    void onSliderChanged(qreal h);
    void onColorPicked(const QColor &c);
    void onColorEdited(const QColor &c);
    void onPicked(const QString &uuid, const QString &colorName);
    void startPick();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
private:
    QVBoxLayout *mainLayout = nullptr;
    ColorLabel *label = nullptr;
    ColorSlider *slider = nullptr;
    ColorEditor *editor = nullptr;
    Dtk::Widget::DIconButton *pickTool = nullptr;
};

}

#endif // COLORDIALOG_H
