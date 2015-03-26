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
uniform bool is_perspective;
uniform vec3 parallel_view_dir;
uniform vec3 eye_pos;

#if __VERSION__ >= 130
	in vec3 position;
	in vec4 color;
#else
	#define in attribute
	#define out varying
	#define flat
	#define color gl_Color
	#define position gl_Vertex
#endif

in vec3 cylinder_base;
in vec3 cylinder_axis;

flat out vec4 vertex_color_out;

void main()
{
	vertex_color_out = color;
	
	if(cylinder_axis != vec3(0)) {
	
		// Get view direction.
		vec3 view_dir;
		if(!is_perspective)
			view_dir = parallel_view_dir;
		else
			view_dir = eye_pos - cylinder_base;
	
		// Build local coordinate system.
		vec3 u = normalize(cross(view_dir, cylinder_axis));
		vec3 rotated_pos = cylinder_axis * position.x + u * position.y + cylinder_base;
		gl_Position = modelview_projection_matrix * vec4(rotated_pos, 1.0);
	}
	else {
		gl_Position = vec4(0);
	}	
}
