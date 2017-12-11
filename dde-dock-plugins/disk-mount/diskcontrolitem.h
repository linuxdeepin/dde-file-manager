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

#ifndef DISKCONTROLITEM_H
#define DISKCONTROLITEM_H

#include <dimagebutton.h>

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QIcon>
#include "qdiskinfo.h"

class DiskControlItem : public QFrame
{
    Q_OBJECT

public:
    explicit DiskControlItem(const QDiskInfo &info, QWidget *parent = 0);

signals:
    void requestUnmount(const QString &diskId) const;
//    void requestOpenDrive(const QString &uri) const;

private slots:
    void updateInfo(const QDiskInfo &info);
    static QString sizeString(const QString &str);
    static qreal dRound64(qreal num, int count = 1);
    const QString formatDiskSize(const quint64 num) const;

private:
    void mouseReleaseEvent(QMouseEvent *e);

private:
    QDiskInfo m_info;
    QIcon m_unknowIcon;

    QLabel *m_diskIcon;
    QLabel *m_diskName;
    QLabel *m_diskCapacity;
    QProgressBar *m_capacityValueBar;
    Dtk::Widget::DImageButton *m_unmountButton;
};

#endif // DISKCONTROLITEM_H
