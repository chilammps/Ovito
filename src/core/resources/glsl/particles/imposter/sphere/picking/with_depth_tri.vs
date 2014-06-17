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
uniform int pickingBaseID;

#if __VERSION__ >= 130

	// The particle data:
	in vec3 position;
	in float particle_radius;
	
	// Output to fragment shader:
	flat out vec4 particle_color_fs;
	flat out float particle_radius_fs;		// The particle's radius.
	flat out float ze0;					// The particle's Z coordinate in eye coordinates.
	out vec2 texcoords;

#else

	// The particle data:
	attribute float particle_radius;
	attribute float vertexID;

	// Output to fragment shader:
	#define particle_radius_fs gl_TexCoord[1].x
	#define ze0 gl_TexCoord[1].y
	
#endif

void main()
{
#if __VERSION__ >= 130
	// Compute color from object ID.
	int objectID = pickingBaseID + (gl_VertexID / 6);
		
	particle_color_fs = vec4(
		float(objectID & 0xFF) / 255.0, 
		float((objectID >> 8) & 0xFF) / 255.0, 
		float((objectID >> 16) & 0xFF) / 255.0, 
		float((objectID >> 24) & 0xFF) / 255.0);		
		
	// Transform and project particle position.
	vec4 eye_position = modelview_matrix * vec4(position, 1);
		
	// Assign texture coordinates. 
	texcoords = imposter_texcoords[gl_VertexID % 6];

	// Transform and project particle position.
	gl_Position = projection_matrix * (eye_position + particle_radius * imposter_voffsets[gl_VertexID % 6]);

#else
	float objectID = pickingBaseID + floor(vertexID / 6);
	gl_FrontColor = vec4(
		floor(mod(objectID, 256.0)) / 255.0,
		floor(mod(objectID / 256.0, 256.0)) / 255.0, 
		floor(mod(objectID / 65536.0, 256.0)) / 255.0, 
		floor(mod(objectID / 16777216.0, 256.0)) / 255.0);	
		
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
