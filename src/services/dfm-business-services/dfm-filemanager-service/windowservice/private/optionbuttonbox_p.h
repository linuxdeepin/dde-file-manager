#ifndef OPTIONBUTTONBOX_P_H
#define OPTIONBUTTONBOX_P_H

#include "dfm_filemanager_service_global.h"

#include <QPushButton>
#include <QHBoxLayout>

class QToolButton;
class QHBoxLayout;
DSB_FM_BEGIN_NAMESPACE
class OptionButtonBox;
class OptionButtonBoxPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(OptionButtonBox)
    OptionButtonBox * const q_ptr;

    QToolButton* iconViewButton = nullptr;
    QToolButton* listViewButton = nullptr;
    QToolButton* detailButton = nullptr;
    QHBoxLayout* hBoxLayout = nullptr;

    explicit OptionButtonBoxPrivate(OptionButtonBox *parent);
};

DSB_FM_END_NAMESPACE

#endif // OPTIONBUTTONBOX_P_H
