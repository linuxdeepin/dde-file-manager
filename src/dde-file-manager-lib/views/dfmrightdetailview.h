/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
 *
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
#ifndef DFMRIGHTDETAILVIEW_H
#define DFMRIGHTDETAILVIEW_H

#include<QFrame>
#include <durl.h>

class DFMRightDetailViewPrivate;
class DFMRightDetailView : public QFrame{
    Q_OBJECT
public:
    explicit DFMRightDetailView(const DUrl &fileUrl, QWidget *parent = nullptr);
    virtual ~DFMRightDetailView();

    void setUrl(const DUrl &url);
    void setTagWidgetVisible(bool visible);
protected:
    void initUI();
    void initTagWidget();
    void LoadFileTags();

private:
    QScopedPointer<DFMRightDetailViewPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMRightDetailView)

};


#endif // DFMRIGHTDETAILVIEW_H
