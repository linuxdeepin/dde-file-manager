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

#ifndef MOVEJOBCONTROLLER_H
#define MOVEJOBCONTROLLER_H


#include <QtCore>
#include <QtDBus>

class MoveJobInterface;
class MovejobWorker;

class MoveJobController : public QObject
{
    Q_OBJECT
public:
    explicit MoveJobController(QObject *parent = 0);
    ~MoveJobController();

    void initConnect();

signals:

public slots:
    void createMoveJob(QStringList files, QString destination);
    void handleConflictConfirmResponse(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);

private:
    QList<MovejobWorker*> m_works;
};

#endif // MOVEJOBCONTROLLER_H
