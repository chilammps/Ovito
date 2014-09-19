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

// Inputs from calling program:
uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;
uniform vec2 imposter_texcoords[6];
uniform vec4 imposter_voffsets[6];

#if __VERSION__ >= 130

	// The particle data:
	in vec3 position;
	in vec4 color;
	in float particle_radius;

	// Output to fragment shader:
	flat out vec4 particle_color_fs;
	flat out float particle_radius_fs;	// The particle's radius.
	flat out float ze0;					// The particle's Z coordinate in eye coordinates.
	out vec2 texcoords;

#else

	attribute float particle_radius;
	attribute float vertexID;
	
	#define particle_radius_fs gl_TexCoord[1].x
	#define ze0 gl_TexCoord[1].y
	#define particle_color_fs gl_FrontColor
#endif

void main()
{
#if __VERSION__ >= 130

	// Forward color to fragment shader.
	particle_color_fs = color;

	// Transform and project particle position.
	vec4 eye_position = modelview_matrix * vec4(position, 1);

	// Assign texture coordinates. 
	texcoords = imposter_texcoords[gl_VertexID % 6];

	// Transform and project particle position.
	gl_Position = projection_matrix * (eye_position + particle_radius * imposter_voffsets[gl_VertexID % 6]);
#else

	// Pass color to fragment shader.
	particle_color_fs = gl_Color;

	// Transform and project particle position.
	vec4 eye_position = modelview_matrix * gl_Vertex;

	int cornerIndex = int(mod(vertexID+0.5, 6.0));
	
	// Assign texture coordinates. 
	gl_TexCoord[0].xy = imposter_texcoords[cornerIndex];
	
	// Transform and project particle position.
	gl_Position = projection_matrix * (eye_position + particle_radius * imposter_voffsets[cornerIndex]);
#endif

	// Forward particle radius to fragment shader.
	particle_radius_fs = particle_radius;
	
	// Pass particle position in eye coordinates to fragment shader.
	ze0 = eye_position.z;
}

