#ifndef BACKGROUNDDEFAULT_H
#define BACKGROUNDDEFAULT_H

#include "dfm-base/widgets/abstractbackground.h"

class BackgroundDefault: public dfmbase::AbstractBackground
{
public:
    explicit BackgroundDefault(QWidget *parent = nullptr);
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // BACKGROUNDDEFAULT_H
