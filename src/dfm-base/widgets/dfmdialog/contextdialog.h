#ifndef CONTEXTDIALOG_H
#define CONTEXTDIALOG_H

#include "dfm-base/dfm_base_global.h"

#include <DDialog>

DFMBASE_BEGIN_NAMESPACE

class ContextDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
public:
    explicit ContextDialog(QWidget *parent = nullptr);
    ContextDialog(const QString &title, const QString& message, QWidget *parent = nullptr);
};

DFMBASE_END_NAMESPACE

#endif // CONTEXTDIALOG_H
