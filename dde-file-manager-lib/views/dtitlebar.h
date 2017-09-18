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

#ifndef DTITLEBAR_H
#define DTITLEBAR_H

#include <QFrame>

class QIcon;
class QPushButton;

class DTitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit DTitleBar(QWidget *parent = 0);
    ~DTitleBar();

    static const int ButtonHeight;

    void initData();
    void initUI();
    void initConnect();

signals:
    void maximumed();
    void minimuned();
    void normaled();
    void closed();
    void switchMaxNormal();

public slots:
    void setNormalIcon();
    void setMaxIcon();

protected:
    void  mouseDoubleClickEvent(QMouseEvent* event);

private:
    QIcon* m_settingsIcon;
    QIcon* m_minIcon;
    QIcon* m_maxIcon;
    QIcon* m_normalIcon;
    QIcon* m_closeIcon;

    QPushButton* m_settingButton;
    QPushButton* m_minButton;
    QPushButton* m_maxNormalButton;
    QPushButton* m_closeButton;
};

#endif // DTITLEBAR_H
