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

    Q_PROPERTY(QList<QColor> checkedColorList READ checkedColorList WRITE setCheckedColorList)
    Q_PROPERTY(bool exclusive READ exclusive WRITE setExclusive)

public:
    explicit DTagActionWidget(QWidget* const parent = nullptr);
    virtual ~DTagActionWidget()=default;

    DTagActionWidget(const DTagActionWidget& other)=delete;
    DTagActionWidget& operator=(const DTagActionWidget &other)=delete;

    QList<QColor> checkedColorList() const;
    void setCheckedColorList(const QList<QColor> &colorNames);

    bool exclusive() const;
    void setExclusive(bool exclusive);

    void setToolTipVisible(bool visible);
    void setToolTipText(const QString &text);
    void clearToolTipText();

signals:
    void hoverColorChanged(const QColor &color);
    void checkedColorChanged(const QColor &color);

private:
    void setCentralLayout()noexcept;
    void initConnect();

    QSharedPointer<DTagActionWidgetPrivate> d_ptr{ nullptr };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DTagActionWidget)
};

#endif // DTAGACTIONWIDGET_H
