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
layout(triangle_strip, max_vertices=14) out;

// Inputs from calling program:
uniform mat4 projection_matrix;

// Inputs from vertex shader
in vec4 cylinder_color_gs[1];		// The base color of the cylinder.
in float cylinder_radius_gs[1];		// The radius of the cylinder
in vec3 cylinder_view_base_gs[1];	// Transformed cylinder position in view coordinates
in vec4 cylinder_view_axis_gs[1];	// Transformed cylinder axis in view coordinates

// Outputs to fragment shader
flat out vec4 cylinder_color_fs;		// The base color of the cylinder.
flat out vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
flat out vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
flat out float cylinder_radius_sq_fs;	// The squared radius of the cylinder
flat out float cylinder_length;			// The length of the cylinder

void main()
{
	float clen = length(cylinder_view_axis_gs[0]);

	vec4 u;
	if(cylinder_view_axis_gs[0].y != 0.0 || cylinder_view_axis_gs[0].x != 0.0)
		u = normalize(vec4(cylinder_view_axis_gs[0].y, -cylinder_view_axis_gs[0].x, 0.0, 0.0)) * cylinder_radius_gs[0];
	else if(cylinder_view_axis_gs[0].z != 0.0)
		u = normalize(vec4(-cylinder_view_axis_gs[0].z, 0.0, cylinder_view_axis_gs[0].x, 0.0)) * cylinder_radius_gs[0];
	else
	    u = vec4(0.0);
	vec4 v = vec4(normalize(cross(cylinder_view_axis_gs[0].xyz, u.xyz)), 0.0) * cylinder_radius_gs[0];

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position + u + v + cylinder_view_axis_gs[0]);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position + u - v + cylinder_view_axis_gs[0]);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position + u + v);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position + u - v);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position - u - v);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position + u - v + cylinder_view_axis_gs[0]);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position -u - v + cylinder_view_axis_gs[0]);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position + u + v + cylinder_view_axis_gs[0]);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position - u + v + cylinder_view_axis_gs[0]);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position + u + v);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position - u + v);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position - u - v);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position - u + v + cylinder_view_axis_gs[0]);
	EmitVertex();

	cylinder_color_fs = cylinder_color_gs[0];
	cylinder_radius_sq_fs = cylinder_radius_gs[0] * cylinder_radius_gs[0];
	cylinder_view_base = cylinder_view_base_gs[0];
	cylinder_view_axis = cylinder_view_axis_gs[0].xyz;
	cylinder_length = clen;
	gl_Position = projection_matrix * (gl_in[0].gl_Position - u - v + cylinder_view_axis_gs[0]);
	EmitVertex();
}
