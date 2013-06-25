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

// The particle radius:
attribute float particle_radius;

void main()
{
	// Forward color to fragment shader.
	gl_FrontColor = gl_Color;

	// Transform and project particle position.
	vec4 position = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ProjectionMatrix * position;

	// Compute sprite size.		
	gl_PointSize = basePointSize * particle_radius / (position.z * gl_ProjectionMatrix[2][3] + gl_ProjectionMatrix[3][3]);
}
