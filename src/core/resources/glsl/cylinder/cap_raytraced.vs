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

// Outputs to fragment shader
varying vec4 cap_color;	// The shaded color of the cylinder cap.

void main()
{
	// Transform and project vertex position.
	gl_Position = ftransform();
	
	// Assign texture coordinate to vertex.
	gl_TexCoord[0]  = gl_MultiTexCoord0;

	vec3 surface_normal = normalize(gl_NormalMatrix * gl_Normal);
	float ambient = 0.4;
	float diffuse = abs(surface_normal.z) * 0.6;
	
	float shininess = 6.0;
	vec3 specular_lightdir = normalize(vec3(-1.8, 1.5, -0.2));
	float specular = pow(max(0.0, dot(reflect(specular_lightdir, surface_normal), vec3(0.0,0.0,-1.0))), shininess) * 0.25;
	
	// Pass color to fragment shader.
	cap_color.rgb = gl_Color.rgb * (diffuse + ambient) + vec3(specular);
	cap_color.a = gl_Color.a;
}
