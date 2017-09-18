/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "messagedialog.h"
#include <QIcon>

MessageDialog::MessageDialog(QWidget *parent) : DDialog(parent)
{

}

MessageDialog::MessageDialog(const QString &message, QWidget *parent):
    DDialog(parent)
{
    m_msg = message;
    initUI();
}

void MessageDialog::initUI()
{
    setIcon(QIcon(":/dialog/dialog_warning_64.png"));
    addButton(tr("OK"), true, DDialog::ButtonRecommend);
    setTitle(m_msg);
}

