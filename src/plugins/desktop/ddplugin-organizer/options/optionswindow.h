#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <DMainWindow>

namespace ddplugin_organizer {

class OptionsWindowPrivate;
class OptionsWindow : public DTK_WIDGET_NAMESPACE::DMainWindow
{
    Q_OBJECT
    friend class OptionsWindowPrivate;
public:
    explicit OptionsWindow(QWidget *parent = 0);
    ~OptionsWindow() override;
    bool initialize();
private:
    OptionsWindowPrivate *d;
};

}

#endif // OPTIONSWINDOW_H
