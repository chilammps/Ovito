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
uniform mat4 modelviewprojection_matrix;
uniform int pickingBaseID;
uniform vec3 cubeVerts[14];

#if __VERSION__ >= 130

	// The particle data:
	in vec3 position;
	in vec3 shape;
	in vec4 orientation;
	in float particle_radius;
	in float vertexID;
	
	// Outputs to fragment shader
	flat out vec4 particle_color_fs;
	flat out vec3 particle_shape_fs;
	
#else

	// The particle data:
	attribute vec3 shape;
	attribute vec4 orientation;
	attribute float particle_radius;	
	attribute float vertexID;

#endif

void main()
{
	mat3 rot;
	if(orientation != vec4(0)) {
		rot = mat3(
			1.0 - 2.0*(orientation.y*orientation.y + orientation.z*orientation.z),
			2.0*(orientation.x*orientation.y + orientation.w*orientation.z),
			2.0*(orientation.x*orientation.z - orientation.w*orientation.y),
			
			2.0*(orientation.x*orientation.y - orientation.w*orientation.z),
			1.0 - 2.0*(orientation.x*orientation.x + orientation.z*orientation.z),
			2.0*(orientation.y*orientation.z + orientation.w*orientation.x),
			
			2.0*(orientation.x*orientation.z + orientation.w*orientation.y),
			2.0*(orientation.y*orientation.z - orientation.w*orientation.x),
			1.0 - 2.0*(orientation.x*orientation.x + orientation.y*orientation.y)
		);
	}
	else {
		rot = mat3(1.0);
	}
	
#if __VERSION__ >= 130	
	
	// Compute color from object ID.
	int objectID = pickingBaseID + int(vertexID) / 14;
	
	particle_color_fs = vec4(
		float(objectID & 0xFF) / 255.0, 
		float((objectID >> 8) & 0xFF) / 255.0, 
		float((objectID >> 16) & 0xFF) / 255.0, 
		float((objectID >> 24) & 0xFF) / 255.0);

	// Transform and project vertex.
	if(shape != vec3(0,0,0))
		particle_shape_fs = shape;
	else
		particle_shape_fs = vec3(particle_radius, particle_radius, particle_radius);
	gl_Position = modelviewprojection_matrix * vec4(position + rot * (cubeVerts[gl_VertexID % 14] * particle_shape_fs), 1);
	
#else

	// Compute color from object ID.
	float objectID = pickingBaseID + floor(vertexID / 14);
	gl_FrontColor = vec4(
		floor(mod(objectID, 256.0)) / 255.0,
		floor(mod(objectID / 256.0, 256.0)) / 255.0, 
		floor(mod(objectID / 65536.0, 256.0)) / 255.0, 
		floor(mod(objectID / 16777216.0, 256.0)) / 255.0);	
				
	// Transform and project vertex.
	int cubeCorner = int(mod(vertexID+0.5, 14.0));
	vec3 delta;
	if(shape != vec3(0,0,0))
		delta = cubeVerts[cubeCorner] * shape;
	else
		delta = cubeVerts[cubeCorner] * particle_radius;
	gl_Position = modelviewprojection_matrix * (gl_Vertex + vec4(rot * delta, 0));

#endif
}
