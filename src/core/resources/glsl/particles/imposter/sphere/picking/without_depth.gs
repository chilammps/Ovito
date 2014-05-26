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
uniform mat4 projection_matrix;

// Inputs from vertex shader
in vec4 particle_color_gs[1];
in float particle_radius_gs[1];

// Outputs to fragment shader
flat out vec4 particle_color_fs;
out vec2 texcoords;

void main()
{
	float psizeX = particle_radius_gs[0] * projection_matrix[0][0];
	float psizeY = particle_radius_gs[0] * projection_matrix[1][1];

	particle_color_fs = particle_color_gs[0];
	texcoords = vec2(1,1);
	gl_Position = gl_in[0].gl_Position + vec4(psizeX, -psizeY, 0.0, 0.0);
	EmitVertex();
	
	particle_color_fs = particle_color_gs[0];
	texcoords = vec2(1,0);
	gl_Position = gl_in[0].gl_Position + vec4(psizeX, psizeY, 0.0, 0.0);
	EmitVertex();
	
	particle_color_fs = particle_color_gs[0];
	texcoords = vec2(0,1);
	gl_Position = gl_in[0].gl_Position + vec4(-psizeX, -psizeY, 0.0, 0.0);
	EmitVertex();
	
	particle_color_fs = particle_color_gs[0];
	texcoords = vec2(0,0);
	gl_Position = gl_in[0].gl_Position + vec4(-psizeX, psizeY, 0.0, 0.0);
	EmitVertex();
}
