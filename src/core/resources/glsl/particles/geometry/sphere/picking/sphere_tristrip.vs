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
uniform int pickingBaseID;
uniform vec3 cubeVerts[14];

#if __VERSION__ >= 130

	// The particle data:
	in vec3 position;
	in float particle_radius;
	in float vertexID;
	
	// Outputs to fragment shader
	flat out vec4 particle_color_fs;
	flat out float particle_radius_squared_fs;
	flat out vec3 particle_view_pos_fs;

#else

	// The particle data:
	attribute float particle_radius;
	attribute float vertexID;

	// Outputs to fragment shader
	varying float particle_radius_squared_fs;
	varying vec3 particle_view_pos_fs;
	
#endif

void main()
{
#if __VERSION__ >= 130

	particle_radius_squared_fs = particle_radius * particle_radius;
	particle_view_pos_fs = vec3(modelview_matrix * vec4(position, 1));

	// Compute color from object ID.
	int objectID = pickingBaseID + int(vertexID) / 14;
	particle_color_fs = vec4(
		float(objectID & 0xFF) / 255.0, 
		float((objectID >> 8) & 0xFF) / 255.0, 
		float((objectID >> 16) & 0xFF) / 255.0, 
		float((objectID >> 24) & 0xFF) / 255.0);
		
	// Transform and project vertex.
	gl_Position = projection_matrix * modelview_matrix * vec4(position + cubeVerts[gl_VertexID % 14] * particle_radius, 1);
	
#else

	particle_radius_squared_fs = particle_radius * particle_radius;
	particle_view_pos_fs = vec3(modelview_matrix * gl_Vertex);

	// Compute color from object ID.
	int objectID = pickingBaseID + int(vertexID) / 14;
	gl_FrontColor = vec4(
		mod(objectID, 0x100) / 255.0, 
		mod(objectID / 0x100, 0x100) / 255.0, 
		mod(objectID / 0x10000, 0x100) / 255.0, 
		mod(objectID / 0x1000000, 0x100) / 255.0);	
		
	// Transform and project vertex.
	int cubeCorner = int(floor(int(vertexID) - 14 * floor(int(vertexID) / 14 + 0.5) + 0.5));
	gl_Position = projection_matrix * modelview_matrix * (gl_Vertex + vec4(cubeVerts[cubeCorner] * particle_radius, 0));
	
#endif
}
