#ifndef DROUNDRECTBUTTON_H
#define DROUNDRECTBUTTON_H


#include <tuple>
#include <initializer_list>

#include <QFrame>
#include <QPainter>
#include <QMarginsF>
#include <QMouseEvent>
#include <QPaintEvent>

class DRoundButton final : public QFrame
{
    Q_OBJECT

private:
    enum class PaintStatus{ normal, hover, pressed, checked };

public:

    DRoundButton(const std::initializer_list<QPair<QColor, QColor>>& colors,
                                            QWidget* const parent = nullptr);



    DRoundButton(const DRoundButton& other)=delete;
    DRoundButton& operator=(const DRoundButton& other)=delete;

    void setRadiusF(const double& radius)noexcept;
    void setRadiusF(const double &radius)const noexcept= delete;
    void setRadius(const std::size_t& radius) noexcept;
    void setRadius(const std::size_t& radius)const noexcept = delete;

    void setBackGroundColor(const QColor& color)noexcept;
    void setBackGroundColor(const QColor &color)const noexcept = delete;


    void setCheckable(bool checkable)noexcept;
    void setChecked(bool checked);
    bool isChecked() const;
    bool isHovered() const;

    QColor color() const;

signals:
    void click(QColor color);
    void enter();
    void leave();
    void checkedChanged();

protected:
    virtual void enterEvent(QEvent* event)override;
    virtual void leaveEvent(QEvent* event)override;
    virtual void mousePressEvent(QMouseEvent* event)override;
    virtual void mouseReleaseEvent(QMouseEvent* event)override;

    virtual void paintEvent(QPaintEvent* paintEvent)override;



private:
    using QFrame::setFixedHeight;
    using QFrame::setFixedWidth;
    using QFrame::setFixedSize;
    using QFrame::setBaseSize;


    void setParameters();
    void setPaintStatus(PaintStatus status);

    bool m_checkable{ true };

    QColor m_selectedColor{};
    QPair<double, std::size_t> m_radius{0.0, 0};
    QList<QPair<QColor, QColor>> m_allStatusColors{};
    PaintStatus m_paintStatus{ PaintStatus::normal };
};


#endif // DROUNDRECTBUTTON_H
