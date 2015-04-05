///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2014) Alexander Stukowski
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
uniform vec3 cubeVerts[14];

#if __VERSION__ >= 130

	// The particle data:
	in vec3 position;
	in vec4 color;
	in vec3 shape;
	in vec4 orientation;
	in float particle_radius;
	
	// Outputs to fragment shader
	flat out vec4 particle_color_fs;
	flat out mat3 particle_quadric_fs;
	flat out vec3 particle_view_pos_fs;
	
#else
	
	// The particle data:
	attribute vec3 shape;
	attribute vec4 orientation;
	attribute float particle_radius;
	attribute float vertexID;

	// Outputs to fragment shader
	#define particle_view_pos_fs gl_TexCoord[1].xyz

#endif

void main()
{
	mat3 rot;
	if(orientation != vec4(0)) {
		// Normalize quaternion.
		vec4 quat = normalize(orientation);
		rot = mat3(
			1.0 - 2.0*(quat.y*quat.y + quat.z*quat.z),
			2.0*(quat.x*quat.y + quat.w*quat.z),
			2.0*(quat.x*quat.z - quat.w*quat.y),			
			2.0*(quat.x*quat.y - quat.w*quat.z),
			1.0 - 2.0*(quat.x*quat.x + quat.z*quat.z),
			2.0*(quat.y*quat.z + quat.w*quat.x),			
			2.0*(quat.x*quat.z + quat.w*quat.y),
			2.0*(quat.y*quat.z - quat.w*quat.x),
			1.0 - 2.0*(quat.x*quat.x + quat.y*quat.y)
		);
	}
	else {
		rot = mat3(1.0);
	}
	
	vec3 shape2 = shape;
	if(shape2.x == 0.0) shape2.x = particle_radius;
	if(shape2.y == 0.0) shape2.y = particle_radius;
	if(shape2.z == 0.0) shape2.z = particle_radius;
	
	mat3 qmat = mat3(1.0/(shape2.x*shape2.x), 0, 0,
			 		 0, 1.0/(shape2.y*shape2.y), 0,
			  		 0, 0, 1.0/(shape2.z*shape2.z));

	mat3 view_rot = mat3(modelview_matrix) * rot;
	
#if __VERSION__ >= 130

    particle_quadric_fs = view_rot * qmat * transpose(view_rot);

	// Forward color to fragment shader.
	particle_color_fs = color;

	// Transform and project vertex.
	int cubeCorner = gl_VertexID % 14;
	vec3 delta = cubeVerts[cubeCorner] * shape2;
	gl_Position = modelviewprojection_matrix * vec4(position + rot * delta, 1);

	particle_view_pos_fs = (modelview_matrix * vec4(position, 1)).xyz;

#else

    mat3 particle_quadric_fs = view_rot * qmat * transpose(view_rot);
    gl_TexCoord[2].x = particle_quadric_fs[0][0];
	gl_TexCoord[2].y = particle_quadric_fs[1][0];
	gl_TexCoord[2].z = particle_quadric_fs[2][0];
	gl_TexCoord[2].w = particle_quadric_fs[1][1];
	gl_TexCoord[3].x = particle_quadric_fs[2][1];
	gl_TexCoord[3].y = particle_quadric_fs[2][2];

	// Forward color to fragment shader.
	gl_FrontColor = gl_Color;

	// Transform and project vertex.
	int cubeCorner = int(mod(vertexID+0.5, 14.0));
	vec3 delta = cubeVerts[cubeCorner] * shape2;
		
	gl_Position = modelviewprojection_matrix * vec4(gl_Vertex.xyz + rot * delta, 1);

	particle_view_pos_fs = (modelview_matrix * gl_Vertex).xyz;

#endif
}
