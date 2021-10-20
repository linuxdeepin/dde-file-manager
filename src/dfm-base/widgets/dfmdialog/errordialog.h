#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include "dfm-base/dfm_base_global.h"

#include <DDialog>

DFMBASE_BEGIN_NAMESPACE

class ErrorDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    ErrorDialog(const QString &title, const QString& message, QWidget *parent = nullptr);
};

DFMBASE_END_NAMESPACE

#endif // ERRORDIALOG_H
