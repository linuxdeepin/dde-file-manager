#include "contextdialog.h"

DFMBASE_BEGIN_NAMESPACE

/*!
 * \class The ContextDialog class
 * \brief 该类融合用户复杂的提示框弹框操作。
 * 例如在动态链接的需求中，如果源文件被删除。
 * 此时用户点击共享文件。则由用户交互进行【确认】/【取消】
 * 最终回调不同的执行逻辑进行删除动态链接文件。
 *
 * ---------------------------------------
 * |您所访问的动态链接文件关联的真实文件不存在， |
 * |是否删除该链接文件？                     |
 * |--------------------------------------
 * |        【确 认】        【取 消】      |
 * ---------------------------------------
 *
 * 由用户进行二次确认对话框ContextDialog
 */

ContextDialog::ContextDialog(QWidget *parent)
    : DDialog(parent)
{

}

ContextDialog::ContextDialog(const QString &title, const QString &message, QWidget *parent)
    : DDialog(title, message, parent)
{

}

DFMBASE_END_NAMESPACE
