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
uniform mat4 modelview_projection_matrix;
uniform float modelview_uniform_scale;
uniform int pickingBaseID;
uniform int verticesPerElement;

#if __VERSION__ < 130
	#define in attribute
	#define out varying
	#define flat
#endif

// The vertex data
in vec3 vertex_pos;
in float vertexID;

// The cylinder data:
in vec3 cylinder_base;				// The position of the cylinder in model coordinates.
in vec3 cylinder_axis;				// The axis of the cylinder in model coordinates.
in float cylinder_radius;			// The radius of the cylinder in model coordinates.

// Outputs to fragment shader
flat out vec4 cylinder_color_in;		// The base color of the cylinder.
flat out vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
flat out vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
flat out float cylinder_radius_in;		// The radius of the cylinder
flat out float cylinder_length;			// The length of the cylinder

void main()
{
	// Compute color from object ID.
	int objectID = pickingBaseID + (int(vertexID) / verticesPerElement);
#if __VERSION__ >= 130
	cylinder_color_in = vec4(
		float(objectID & 0xFF) / 255.0, 
		float((objectID >> 8) & 0xFF) / 255.0, 
		float((objectID >> 16) & 0xFF) / 255.0, 
		float((objectID >> 24) & 0xFF) / 255.0);
#else
	cylinder_color_in = vec4(
		float(mod(objectID, 0x100)) / 255.0, 
		float(mod(objectID / 0x100, 0x100)) / 255.0, 
		float(mod(objectID / 0x10000, 0x100)) / 255.0, 
		float(mod(objectID / 0x1000000, 0x100)) / 255.0);		
#endif
	
	// Pass radius to fragment shader.
	cylinder_radius_in = cylinder_radius * modelview_uniform_scale;

	// Transform cylinder to eye coordinates.
	cylinder_view_base = vec3(modelview_matrix * vec4(cylinder_base, 1));
	cylinder_view_axis = vec3(modelview_matrix * vec4(cylinder_axis, 0));

	// Pass length to fragment shader.
	cylinder_length = length(cylinder_view_axis);

	// Transform and project vertex position.
	gl_Position = modelview_projection_matrix * vec4(vertex_pos, 1.0);
}
