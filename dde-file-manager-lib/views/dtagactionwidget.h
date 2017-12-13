#ifndef DTAGACTIONWIDGET_H
#define DTAGACTIONWIDGET_H


#include <QSharedPointer>
#include <QPushButton>
#include <QMouseEvent>
#include <QPainter>
#include <QFrame>


class DTagActionWidgetPrivate;
class DTagActionWidget final : public QFrame
{
    Q_OBJECT

public:
    DTagActionWidget(QWidget* const parent = nullptr);
    virtual ~DTagActionWidget()=default;

    DTagActionWidget(const DTagActionWidget& other)=delete;
    DTagActionWidget& operator=(const DTagActionWidget &other)=delete;

    inline const QColor& selectedColor()const
    {
        return m_selectedColor;
    }

private slots:
    void onButtonClicked(QColor color)noexcept;

private:

    void setCentralLayout()noexcept;
    void initConnect();

    QColor m_selectedColor{};

    QSharedPointer<DTagActionWidgetPrivate> d_ptr{ nullptr };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DTagActionWidget)
};

#endif // DTAGACTIONWIDGET_H
