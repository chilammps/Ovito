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
uniform mat3 normal_matrix;
uniform vec3 cubeVerts[14];
uniform vec3 normals[14];

#if __VERSION__ >= 130

	// The particle data:
	in vec3 position;
	in vec4 color;
	in float particle_radius;
	
	// Outputs to fragment shader
	flat out vec4 particle_color_fs;
	flat out vec3 surface_normal_fs;
	
#else
	
	// The particle data:
	attribute float particle_radius;
	attribute float vertexID;

	// Outputs to fragment shader
	#define ec_pos gl_TexCoord[1].xyz

#endif

void main()
{
#if __VERSION__ >= 130

	// Forward color to fragment shader.
	particle_color_fs = color;

	// Transform and project vertex.
	int cubeCorner = gl_VertexID % 14;
	gl_Position = modelviewprojection_matrix * vec4(position + cubeVerts[cubeCorner] * particle_radius, 1);

	// Determine face normal.
	surface_normal_fs = normal_matrix * normals[cubeCorner];

#else

	// Forward color to fragment shader.
	gl_FrontColor = gl_Color;

	// Transform and project vertex.
	int cubeCorner = int(mod(vertexID+0.5, 14.0));
	ec_pos = (modelview_matrix * vec4(gl_Vertex.xyz + cubeVerts[cubeCorner] * particle_radius, 1)).xyz;
	gl_Position = projection_matrix * vec4(ec_pos,1);

#endif
}
