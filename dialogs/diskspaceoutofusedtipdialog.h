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

#ifndef DISKSPACEOUTOFUSEDTIPDIALOG_H
#define DISKSPACEOUTOFUSEDTIPDIALOG_H

#include <QObject>
#include "ddialog.h"

DWIDGET_USE_NAMESPACE

class DiskSpaceOutOfUsedTipDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DiskSpaceOutOfUsedTipDialog(QWidget *parent = 0);

signals:

public slots:
};

#endif // DISKSPACEOUTOFUSEDTIPDIALOG_H
