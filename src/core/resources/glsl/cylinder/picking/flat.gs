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

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

// Inputs from calling program: 
uniform mat4 modelview_projection_matrix;
uniform bool is_perspective;
uniform vec3 parallel_view_dir;
uniform vec3 eye_pos;

// Inputs from vertex shader
in vec4 color_gs[1];
in vec3 cylinder_axis_gs[1];
in float cylinder_radius_gs[1];

// Outputs to fragment shader
flat out vec4 vertex_color_out;	

void main()
{
	// Get view direction.
	vec3 view_dir;
	if(!is_perspective)
		view_dir = parallel_view_dir;
	else
		view_dir = eye_pos - gl_in[0].gl_Position.xyz;

	// Build local coordinate system.
	vec3 u = normalize(cross(view_dir, cylinder_axis_gs[0])) * cylinder_radius_gs[0];
	vec3 v = cylinder_axis_gs[0];
	
	gl_Position = modelview_projection_matrix * vec4(v + u + gl_in[0].gl_Position.xyz, 1.0);
	vertex_color_out = color_gs[0];
	EmitVertex();

	gl_Position = modelview_projection_matrix * vec4(u + gl_in[0].gl_Position.xyz, 1.0);
	vertex_color_out = color_gs[0];
	EmitVertex();

	gl_Position = modelview_projection_matrix * vec4(v - u + gl_in[0].gl_Position.xyz, 1.0);
	vertex_color_out = color_gs[0];
	EmitVertex();

	gl_Position = modelview_projection_matrix * vec4(-u + gl_in[0].gl_Position.xyz, 1.0);
	vertex_color_out = color_gs[0];
	EmitVertex();
}
