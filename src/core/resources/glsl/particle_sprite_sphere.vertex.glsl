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
uniform bool isPerspective;		// Specifies the projection mode.
uniform float basePointSize;

// The particle radius:
attribute float particle_radius;

// Output to fragment shader:
varying float depthScale;

void main()
{
	// Forward color to fragment shader.
	gl_FrontColor = gl_Color;

	// Transform and project vertex position.
	gl_Position = ftransform();
	
	if(isPerspective) {
		// Calculate point distance from camera.
		vec4 position = gl_ModelViewMatrix * gl_Vertex;	
		float eyeZ = abs(position.z);
		// Calculate point size.
		gl_PointSize = basePointSize * particle_radius / eyeZ;
	}
	else {
		gl_PointSize = basePointSize * particle_radius;
		vec4 projected_intersection = gl_ProjectionMatrix * vec4(view_intersection_pnt, 1.0);
		gl_FragDepth = (projected_intersection.z / projected_intersection.w + 1.0) * 0.5;
		depthScale = 
	}
}
