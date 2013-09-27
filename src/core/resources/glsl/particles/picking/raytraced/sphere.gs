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
layout(triangle_strip, max_vertices=24) out;

// Inputs from calling program:
uniform mat4 projection_matrix;

// Inputs from vertex shader
in vec4 particle_color_in[1];
in float particle_radius_in[1];

// Outputs to fragment shader
flat out vec4 particle_color;
flat out float particle_radius_squared;
flat out vec3 particle_view_pos;

uniform vec3 cubeVerts[8];
uniform int stripIndices[14];

void main()
{
	particle_view_pos = vec3(gl_in[0].gl_Position); 
	particle_color = particle_color_in[0];
	particle_radius_squared = particle_radius_in[0] * particle_radius_in[0];
	
	for(int vertex = 0; vertex < 14; vertex++) {
		gl_Position = projection_matrix * vec4(particle_view_pos + cubeVerts[stripIndices[vertex]] * particle_radius_in[0], 1);
		EmitVertex();
	}
}
