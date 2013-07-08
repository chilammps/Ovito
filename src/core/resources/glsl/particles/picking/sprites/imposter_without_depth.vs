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

// The input particle data:
in vec3 particle_pos;
in float particle_radius;

// Output passed to fragment shader.
flat out vec4 particle_color_out;

void main()
{
	// Compute color from object ID.
	uint objectID = uint(pickingBaseID + gl_VertexID);
	particle_color_out = vec4(
		float(objectID & 255u) / 255.0, 
		float((objectID >> 8) & 255u) / 255.0, 
		float((objectID >> 16) & 255u) / 255.0, 
		float((objectID >> 24) & 255u) / 255.0);

	// Transform and project particle position.
	vec4 eye_position = modelview_matrix * vec4(particle_pos, 1);
	gl_Position = projection_matrix * eye_position;

	// Compute sprite size.
	gl_PointSize = basePointSize * particle_radius / (eye_position.z * projection_matrix[2][3] + projection_matrix[3][3]);
}
