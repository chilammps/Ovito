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

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;
uniform bool is_perspective;
uniform float line_width;

#if __VERSION__ >= 130

	in vec3 position;
	in vec4 color;
	in vec3 vector;
	out vec4 vertex_color_fs;
	
#else

	attribute vec3 vector;

#endif

void main()
{
#if __VERSION__ >= 130
	vertex_color_fs = color;
#else
	gl_FrontColor = gl_Color;
#endif
	
#if __VERSION__ >= 130
	vec4 view_position = modelview_matrix * vec4(position, 1.0);
#else
	vec4 view_position = modelview_matrix * gl_Vertex;
#endif
	vec3 view_dir;
	if(is_perspective)
		view_dir = view_position.xyz;
	else
		view_dir = vec3(0,0,-1);
	vec3 u = cross(view_dir, (modelview_matrix * vec4(vector,0.0)).xyz);
	if(u != vec3(0)) {
		float w = projection_matrix[0][3] * view_position.x + projection_matrix[1][3] * view_position.y
			+ projection_matrix[2][3] * view_position.z + projection_matrix[3][3];
		gl_Position = projection_matrix * (view_position - vec4((w * line_width / length(u)) * u, 0.0));
	}
	else {
		gl_Position = vec4(0);
	}
	
}
