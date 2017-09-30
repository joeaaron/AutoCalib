/***************************************************************************
 *   Copyright (C) 1998-2013 by authors (see AUTHORS.txt)                  *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 *   Lux Renderer is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Lux Renderer is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This project is based on PBRT ; see http://www.pbrt.org               *
 *   Lux Renderer website : http://www.luxrender.net                       *
 ***************************************************************************/

#ifndef COLORSPACEWIDGET_H
#define COLORSPACEWIDGET_H

#include <QWidget>

#define TORGB_XWHITE_RANGE 1.0f
#define TORGB_YWHITE_RANGE 1.0f
#define TORGB_XRED_RANGE 1.0f
#define TORGB_YRED_RANGE 1.0f
#define TORGB_XGREEN_RANGE 1.0f
#define TORGB_YGREEN_RANGE 1.0f
#define TORGB_XBLUE_RANGE 1.0f
#define TORGB_YBLUE_RANGE 1.0f

#define NUM_COLORSPACE_PRESETS 9
#define NUM_WHITEPOINT_PRESETS 13

namespace Ui
{
	class ColorSpaceWidget;
}

class ColorSpaceWidget : public QWidget
{
	Q_OBJECT

public:
	ColorSpaceWidget(QWidget *parent = 0);
	~ColorSpaceWidget();

private:
	Ui::ColorSpaceWidget *ui;


};

#endif // COLORSPACEWIDGET_H
