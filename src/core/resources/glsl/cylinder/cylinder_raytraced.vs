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
uniform float modelview_uniform_scale;

#if __VERSION__ >= 130

	in vec3 position;
	in vec3 normal;
	in vec4 color;

#else

	#define in attribute
	#define out varying
	#define flat
	
	#define color gl_Color
	
#endif

// The cylinder data:
in vec3 cylinder_base;				// The position of the cylinder in model coordinates.
in vec3 cylinder_axis;				// The axis of the cylinder in model coordinates.
in float cylinder_radius;			// The radius of the cylinder in model coordinates.

// Outputs to geometry shader
out vec4 cylinder_color_gs;			// The base color of the cylinder.
out float cylinder_radius_gs;		// The radius of the cylinder
out vec3 cylinder_view_base_gs;		// Transformed cylinder position in view coordinates
out vec4 cylinder_view_axis_gs;		// Transformed cylinder axis in view coordinates

void main()
{
	// Pass color to geometry shader.
	cylinder_color_gs = color;
	
	// Pass radius to geometry shader.
	cylinder_radius_gs = cylinder_radius * modelview_uniform_scale;

	// Transform cylinder to eye coordinates.
	gl_Position = modelview_matrix * vec4(position, 1);
	cylinder_view_base_gs = gl_Position.xyz;
	cylinder_view_axis_gs = modelview_matrix * vec4(cylinder_axis, 0);
}
