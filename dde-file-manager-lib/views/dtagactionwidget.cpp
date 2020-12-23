
#include <array>

#include "dfileview.h"
#include "darrowrectangle.h"
#include "dtagactionwidget.h"
#include "views/droundbutton.h"
#include "accessibility/ac-lib-file-manager.h"

#include <QColor>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class DTagActionWidgetPrivate
{
public:
    explicit DTagActionWidgetPrivate(DTagActionWidget *const qPtr);


    void setParameters()noexcept;
    void initUiElement();


    std::array<DRoundButton *, 8> m_roundButtons{ {nullptr} };
    QVBoxLayout *mainLayout{ nullptr };
    QHBoxLayout *m_HBoxLayout{ nullptr };
    DTagActionWidget *q_ptr{ nullptr };
    QStringList checkedColorList;
    QLabel *toolTip{ nullptr };
    bool exclusive = false;
};



DTagActionWidgetPrivate::DTagActionWidgetPrivate(DTagActionWidget *const qPtr)
    : q_ptr{ qPtr }
{
    this->initUiElement();
}

void DTagActionWidgetPrivate::setParameters()noexcept
{
    q_ptr->setFocusPolicy(Qt::StrongFocus);
}

void DTagActionWidgetPrivate::initUiElement()
{
    m_roundButtons[0] = new DRoundButton{
        { QColor{"#ff9311"}, QColor{"#ffa503"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };

    m_roundButtons[1] = new DRoundButton{
        { QColor{"#ec1640"}, QColor{"#ff1c49"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };

    m_roundButtons[2] = new DRoundButton{
        { QColor{"#8e08de"}, QColor{"#9023fc"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };

    m_roundButtons[3] = new DRoundButton{
        { QColor{"#0956e7"}, QColor{"#3468ff"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };

    m_roundButtons[4] = new DRoundButton{
        { QColor{"#00c7e1"}, QColor{"#00b5ff"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };

    m_roundButtons[5] = new DRoundButton{
        { QColor{"#60cb00"}, QColor{"#58df0a"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };

    m_roundButtons[6] = new DRoundButton{
        { QColor{"#f4e710"}, QColor{"#fef144"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };

    m_roundButtons[7] = new DRoundButton{
        { QColor{"#ababab"}, QColor{"#cccccc"} },
        { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    };


    m_HBoxLayout = new QHBoxLayout;

    m_HBoxLayout->addSpacing(21);
    for (std::size_t index = 0; index != 8; ++index) {
        m_roundButtons[index]->setContentsMargins(0, 0, 0, 0);
        m_roundButtons[index]->setRadius(20);

        QString objMark = QString("Color%1").arg(index + 1);
        m_roundButtons[index]->setObjectName(objMark);
        AC_SET_ACCESSIBLE_NAME( m_roundButtons[index], objMark);

        m_HBoxLayout->addWidget(m_roundButtons[index], Qt::AlignCenter);
    }

    m_HBoxLayout->addSpacing(21);
    m_HBoxLayout->setMargin(0);
    m_HBoxLayout->setSpacing(0);

    mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(m_HBoxLayout);

    toolTip = new QLabel();
    toolTip->setText(QStringLiteral(" "));
    toolTip->setStyleSheet("color: #707070; font-size: 10px");
    toolTip->setObjectName("tool_tip");
    AC_SET_ACCESSIBLE_NAME( toolTip, AC_TAG_ACTION_WIDGET_TOOL_TIP);

    mainLayout->addWidget(toolTip, 0, Qt::AlignHCenter);
}



DTagActionWidget::DTagActionWidget(QWidget *const parent)
    : QFrame{ parent },
      d_ptr{ new DTagActionWidgetPrivate{ this } }
{
    this->setObjectName("tagActionWidget");
    AC_SET_ACCESSIBLE_NAME( this, AC_TAG_ACTION_WIDGET);
    this->setCentralLayout();
    this->initConnect();
}

QList<QColor> DTagActionWidget::checkedColorList() const
{
    Q_D(const DTagActionWidget);

    QList<QColor> list;

    for (const DRoundButton *button : d->m_roundButtons) {
        if (button->isChecked())
            list << button->color();
    }

    return list;
}

void DTagActionWidget::setCheckedColorList(const QList<QColor> &colorNames)
{
    Q_D(DTagActionWidget);

    for (DRoundButton *button : d->m_roundButtons) {
        button->setChecked(colorNames.contains(button->color()));
    }
}

bool DTagActionWidget::exclusive() const
{
    Q_D(const DTagActionWidget);

    return d->exclusive;
}

void DTagActionWidget::setExclusive(bool exclusive)
{
    Q_D(DTagActionWidget);

    d->exclusive = exclusive;
}

void DTagActionWidget::setToolTipVisible(bool visible)
{
    Q_D(DTagActionWidget);

    d->toolTip->setVisible(visible);
}

void DTagActionWidget::setToolTipText(const QString &text)
{
    Q_D(DTagActionWidget);

    d->toolTip->setText(text);
}

void DTagActionWidget::clearToolTipText()
{
    setToolTipText(QStringLiteral(" "));
}

void DTagActionWidget::setCentralLayout()noexcept
{
    DTagActionWidgetPrivate *const d{ d_func() };
    this->setLayout(d->mainLayout);
}

void DTagActionWidget::initConnect()
{
    Q_D(const DTagActionWidget);

    for (DRoundButton *button : d->m_roundButtons) {
        connect(button, &DRoundButton::enter, this, [this, button] {
            emit hoverColorChanged(button->color());
        });
        connect(button, &DRoundButton::leave, this, [this] {
            emit hoverColorChanged(QColor());
        });
        connect(button, &DRoundButton::checkedChanged, this, [this, button, d] {
            if (button->isChecked() && exclusive())
            {
                for (DRoundButton *b : d->m_roundButtons) {
                    if (b != button)
                        b->setChecked(false);
                }
            }
        });
        connect(button, &DRoundButton::click, this, [this](QColor color) {
            emit checkedColorChanged(color);
        });

    }
}
