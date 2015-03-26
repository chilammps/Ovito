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

#if __VERSION__ >= 130
	in vec3 position;
	in vec4 color;
#else
	#define in attribute
	#define out varying
	#define flat
	#define color gl_Color
	#define position gl_Vertex
#endif

in vec3 cylinder_axis;
in float cylinder_radius;

out vec4 color_gs;
out vec3 cylinder_axis_gs;
out float cylinder_radius_gs;

void main()
{
	color_gs = color;
	cylinder_axis_gs = cylinder_axis;
	cylinder_radius_gs = cylinder_radius;
	gl_Position = vec4(position, 1);	
}
