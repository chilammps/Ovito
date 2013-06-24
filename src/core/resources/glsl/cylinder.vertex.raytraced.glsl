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
uniform bool isPerspective;			// Specifies the projection mode.
attribute vec3 cylinder_base;		// The position of the cylinder in model coordinates.
attribute vec3 cylinder_axis;		// The axis of the cylinder in model coordinates.
attribute float cylinder_radius_a;	// The radius of the cylinder in model coordinates.
attribute float cylinder_length_squared_a;	// The squared length of the cylinder

// Outputs to fragment shader
varying vec4 cylinder_color;			// The base color of the cylinder.
varying vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
varying vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
varying float cylinder_radius;			// The radius of the cylinder
varying float cylinder_length_squared;	// The squared length of the cylinder

void main()
{
	// Save color for fragment shader.
	cylinder_color = gl_Color;
	// Save radius for fragment shader.
	cylinder_radius = cylinder_radius_a;
	// Save length for fragment shader.
	cylinder_length_squared = cylinder_length_squared_a;

	// Transform and project vertex position.
	gl_Position = ftransform();

	cylinder_view_base = vec3(gl_ModelViewMatrix * vec4(cylinder_base, 1));
	cylinder_view_axis = vec3(gl_ModelViewMatrix * vec4(cylinder_axis, 0));
}
