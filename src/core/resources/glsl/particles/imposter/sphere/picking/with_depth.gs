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
uniform vec2 imposter_texcoords[4];
uniform vec4 imposter_voffsets[4];

// Inputs from vertex shader
in vec4 particle_color_gs[1];
in float particle_radius_gs[1];

// Outputs to fragment shader
flat out vec4 particle_color_fs;
flat out float particle_radius_fs;	// The particle's radius.
flat out float ze0;					// The particle's Z coordinate in eye coordinates.
out vec2 texcoords;

void main()
{
	for(int vertex = 0; vertex < 4; vertex++) {
		particle_color_fs = particle_color_gs[0];
		texcoords = imposter_texcoords[vertex];
		particle_radius_fs = particle_radius_gs[0];	
		ze0 = gl_in[0].gl_Position.z;
		gl_Position = projection_matrix * (gl_in[0].gl_Position + imposter_voffsets[vertex] * particle_radius_gs[0]);
		EmitVertex();
	}
}
