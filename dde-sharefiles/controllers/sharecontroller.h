#ifndef SHARECONTROLLER_H
#define SHARECONTROLLER_H

#include "applicationcontroller.h"


class T_CONTROLLER_EXPORT ShareController : public ApplicationController
{
    Q_OBJECT
public:
    Q_INVOKABLE
    ShareController() { }
    ShareController(const ShareController &other);

public slots:
    void share();
    void shareDir();
    void shareFiles();
    void download();
};

T_DECLARE_CONTROLLER(ShareController, sharecontroller)

#endif // SHARECONTROLLER_H
