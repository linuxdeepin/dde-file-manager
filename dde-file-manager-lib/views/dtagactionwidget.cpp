
#include <array>

#include "dfileview.h"
#include "darrowrectangle.h"
#include "dtagactionwidget.h"
#include "views/droundbutton.h"


#include <QColor>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>



using namespace Dtk::Widget;


class DTagActionWidgetPrivate
{
public:
    DTagActionWidgetPrivate(DTagActionWidget* const qPtr);


    void setParameters()noexcept;
    void initUiElement();


    std::array<DRoundButton*, 8> m_roundButtons{ nullptr };
    QHBoxLayout* m_HBoxLayout{ nullptr };
    DTagActionWidget* q_ptr{ nullptr };
};



DTagActionWidgetPrivate::DTagActionWidgetPrivate(DTagActionWidget * const qPtr)
                        :q_ptr{ qPtr }
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
    for(std::size_t index = 0; index != 8; ++index){
        m_roundButtons[index]->setContentsMargins(0, 0, 0, 0);
        m_roundButtons[index]->setRadius(20);
        m_HBoxLayout->addWidget(m_roundButtons[index], Qt::AlignCenter);
    }
    m_HBoxLayout->addSpacing(21);
    m_HBoxLayout->setMargin(0);
    m_HBoxLayout->setSpacing(0);

}



DTagActionWidget::DTagActionWidget(QWidget * const parent)
                 :QFrame{ parent },
                  d_ptr{ new DTagActionWidgetPrivate{ this } }
{
    this->setCentralLayout();
    this->initConnect();
}

void DTagActionWidget::onButtonClicked(QColor color) noexcept
{
    m_selectedColor = color;
}


void DTagActionWidget::setCentralLayout()noexcept
{
    DTagActionWidgetPrivate* const d{ d_func() };
    this->setLayout(d->m_HBoxLayout);
}

void DTagActionWidget::initConnect()
{
    DTagActionWidgetPrivate* const d{ d_func() };

    for(const DRoundButton* const button : d->m_roundButtons){
        QObject::connect(button, &DRoundButton::click, this, &DTagActionWidget::onButtonClicked);
    }
}
