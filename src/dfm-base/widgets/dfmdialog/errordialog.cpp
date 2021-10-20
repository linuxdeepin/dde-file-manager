#include "errordialog.h"

DFMBASE_BEGIN_NAMESPACE

/*!
 * \class The ErrorDialog class
 * \brief 该类主要显示固定简单逻辑的错误处理，
 * 也可只显示错误的提示信息
 * 无需用户进行复杂的确认或者取消按钮。
 * 提供以单个按钮“确认”触发的错误处理的后续流程。
 * ---------------------------------
 * | \/ smb无法访问，请检查网络是否正常 |
 * | /\                            |
 * |--------------------------------
 * |                      【确 认】  |
 * ---------------------------------
 */

ErrorDialog::ErrorDialog(QWidget *parent)
    : DDialog(parent)
{

}

ErrorDialog::ErrorDialog(const QString &title, const QString &message, QWidget *parent)
    : DDialog(title, message, parent)
{

}

DFMBASE_END_NAMESPACE
