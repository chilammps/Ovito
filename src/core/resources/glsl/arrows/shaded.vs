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
uniform mat3 normal_matrix;

#if __VERSION__ >= 130

	in vec3 position;
	in vec3 normal;
	in vec4 color;
	
	flat out vec4 vertex_color_fs;
	out vec3 vertex_normal_fs;

#else

	varying vec3 vertex_normal_fs;

#endif

void main()
{
#if __VERSION__ >= 130
	
	vertex_color_fs = color;
	gl_Position = modelview_projection_matrix * vec4(position, 1.0);
	vertex_normal_fs = normalize(normal_matrix * normal);
	
#else
	
	gl_FrontColor = gl_Color;
	gl_Position = modelview_projection_matrix * gl_Vertex;
	vertex_normal_fs = normalize(normal_matrix * gl_Normal);
	
#endif
}
