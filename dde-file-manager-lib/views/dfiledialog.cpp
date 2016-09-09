#include "dfiledialog.h"

DFileDialog::DFileDialog(QWidget *parent)
    : DFileManagerWindow(parent)
{
    setWindowFlags(windowFlags() | Qt::Dialog);
}
