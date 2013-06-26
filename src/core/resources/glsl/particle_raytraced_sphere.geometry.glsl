#version 120
#extension GL_EXT_geometry_shader4 : enable

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

//layout(points) in;
//layout(triangle_strip, max_vertices=24) out;

// Inputs from vertex shader
//varying in float particle_radius_in[1];

// Outputs to fragment shader
//varying out vec3 particle_color;
//varying out float particle_radius;
//varying out vec4 eye_position;

/*
const vec4 cubeVerts[8] = vec4[8](
    vec4(-1 , -1, -1, 0),  //LB   0
     vec4(-1, 1, -1, 0), //L T   1
    vec4(1, -1, -1, 0), //R B    2
    vec4(1, 1, -1, 0),  //R T   3
                        //back face
    vec4(-1, -1, 1, 0), // LB  4
     vec4(-1, 1, 1, 0), // LT  5
    vec4(1, -1, 1, 0),  // RB  6
     vec4(1, 1, 1, 0)  // RT  7
    );
    
const int  cubeIndices[24]  = int [24]
(
      0,1,2,3, //front
      7,6,3,2, //right
      7,5,6,4,  //back or whatever
      4,0,6,2, //btm 
      1,0,5,4, //left
      3,1,7,5
    );
    */
    
void main()
{
	//vec4 eye_position = gl_in[0].gl_Position; 
	//particle_color = particle_color_in[0];
	//float particle_radius = particle_radius_in[0];

	//gl_Position = gl_ProjectionMatrix * (eye_position + vec3(1,1,1) * particle_radius);
        for (int i=0;i<gl_VerticesIn;++i) {
                gl_FrontColor = gl_FrontColorIn[i];
                gl_Position = gl_PositionIn[i];
                EmitVertex();
        }
}
