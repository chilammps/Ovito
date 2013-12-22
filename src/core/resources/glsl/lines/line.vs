///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2013) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

uniform mat4 modelview_projection_matrix;

#if __VERSION__ >= 130

	in vec3 position;
	in vec4 color;
	out vec4 vertex_color_fs;

#endif

void main()
{
#if __VERSION__ >= 130
	vertex_color_fs = color;
	gl_Position = modelview_projection_matrix * vec4(position, 1.0);
#else
	gl_FrontColor = gl_Color;
	gl_Position = modelview_projection_matrix * gl_Vertex;
#endif
}
