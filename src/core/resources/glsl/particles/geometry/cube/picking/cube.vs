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
uniform int pickingBaseID;

#if __VERSION__ >= 130

// The particle data:
in vec3 position;
in float particle_radius;

// Output to geometry shader.
out vec4 particle_color_gs;
out float particle_radius_gs;

#endif

void main()
{
#if __VERSION__ >= 130
	// Compute color from object ID.
	int objectID = pickingBaseID + gl_VertexID;
	particle_color_gs = vec4(
		float(objectID & 0xFF) / 255.0, 
		float((objectID >> 8) & 0xFF) / 255.0, 
		float((objectID >> 16) & 0xFF) / 255.0, 
		float((objectID >> 24) & 0xFF) / 255.0);	
		
	// Pass radius to geometry shader.
	particle_radius_gs = particle_radius;

	// Pass original particle position to geometry shader.
	gl_Position = vec4(position, 1);
#endif
}
