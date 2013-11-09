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
uniform float basePointSize;
uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;
uniform int pickingBaseID;

#if __VERSION__ >= 130

// The particle data:
in vec3 particle_pos;
in float particle_radius;

// Output to fragment shader:
flat out vec4 particle_color_out;
flat out float depth_radius;		// The particle's radius.
flat out float ze0;					// The particle's Z coordinate in eye coordinates.

#else

// The particle data:
attribute float particle_radius;
attribute float vertexID;
#define gl_VertexID int(vertexID)

#endif

void main()
{
	// Compute color from object ID.
	int objectID = pickingBaseID + gl_VertexID;
#if __VERSION__ >= 130
	particle_color_out = vec4(
		float(objectID & 0xFF) / 255.0, 
		float((objectID >> 8) & 0xFF) / 255.0, 
		float((objectID >> 16) & 0xFF) / 255.0, 
		float((objectID >> 24) & 0xFF) / 255.0);		
		
	// Transform and project particle position.
	vec4 eye_position = modelview_matrix * vec4(particle_pos, 1);
		
#else
	gl_FrontColor = vec4(
		float(mod(objectID, 0x100)) / 255.0, 
		float(mod(objectID / 0x100, 0x100)) / 255.0, 
		float(mod(objectID / 0x10000, 0x100)) / 255.0, 
		float(mod(objectID / 0x1000000, 0x100)) / 255.0);		

	// Transform and project particle position.
	vec4 eye_position = modelview_matrix * gl_Vertex;
#endif

	// Transform and project particle position.
	gl_Position = projection_matrix * eye_position;

	// Compute sprite size.		
	gl_PointSize = basePointSize * particle_radius / (eye_position.z * projection_matrix[2][3] + projection_matrix[3][3]);
	
#if __VERSION__ >= 130

	// Forward particle radius to fragment shader.
	depth_radius = particle_radius;
	
	// Pass particle position in eye coordinates to fragment shader.
	ze0 = eye_position.z;

#else

	// Forward particle radius to fragment shader.
	gl_FogFragCoord = particle_radius;
	
	// Pass particle position in eye coordinates to fragment shader.
	gl_FrontColor.a = eye_position.z;

#endif
}
