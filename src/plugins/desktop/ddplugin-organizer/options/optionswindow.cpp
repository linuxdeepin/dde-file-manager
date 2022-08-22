#include "optionswindow.h"
#include "optionswindow_p.h"

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE

OptionsWindowPrivate::OptionsWindowPrivate(OptionsWindow *qq) : q(qq)
{

}


OptionsWindow::OptionsWindow(QWidget *parent)
    : DMainWindow(parent)
    , d(new OptionsWindowPrivate(this))
{

}

OptionsWindow::~OptionsWindow()
{
    delete d;
    d = nullptr;
}

bool OptionsWindow::initialize()
{
    return true;
}
