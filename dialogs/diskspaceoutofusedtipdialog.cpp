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

#include "diskspaceoutofusedtipdialog.h"
#include <DTitlebar>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QFrame>

DiskSpaceOutOfUsedTipDialog::DiskSpaceOutOfUsedTipDialog(QWidget *parent) : DDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint|
                   Qt::WindowCloseButtonHint|
                   Qt::Dialog);
    setIcon(QIcon(":/images/dialogs/images/dialog_warning_64.png"));

    setTitle(tr("Target disk doesn't have enough space, unable to copy!"));
    setFixedHeight(125);
    addButton(tr("OK"));
}
