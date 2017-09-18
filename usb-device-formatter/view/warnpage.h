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

#ifndef WARNPAGE_H
#define WARNPAGE_H

#include <QFrame>

class WarnPage : public QFrame
{
    Q_OBJECT
public:
    explicit WarnPage(QWidget *parent = 0);
    void initUI();

signals:

public slots:
};

#endif // WARNPAGE_H
