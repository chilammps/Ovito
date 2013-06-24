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

// Inputs from calling program
attribute vec3 particle_pos;		// The position of the sphere in model coordinates.
attribute float particle_radius;	// The radius of the sphere in model coordinates.

// Outputs to fragment shader
varying vec3 particle_view_pos;			// Transformed sphere position in view coordinates.
varying float particle_radius_squared;	// The squared radius.

void main()
{
	// Forward base color to fragment shader.
	gl_FrontColor = gl_Color;
	
	// Pass (squared) radius to fragment shader.
	particle_radius_squared = particle_radius * particle_radius;
	// Store center in view coordinates.
	particle_view_pos = vec3(gl_ModelViewMatrix * vec4(particle_pos, 1.0));

	// Transform and project vertex position.
	gl_Position = ftransform();
}
