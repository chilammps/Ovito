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
	
	flat in vec4 vertex_color_fs;
	in vec3 vertex_normal_fs;
	out vec4 FragColor;

#else

	#define vertex_color_fs gl_Color
	varying vec3 vertex_normal_fs;
	#define FragColor gl_FragColor

#endif

const float ambient = 0.4;
const float diffuse_strength = 0.6;
const float shininess = 6.0;
const vec3 specular_lightdir = normalize(vec3(1.8, -1.5, 0.2));

void main() 
{
	float diffuse = abs(vertex_normal_fs.z) * diffuse_strength;	
	float specular = pow(max(0.0, dot(reflect(specular_lightdir, vertex_normal_fs), vec3(0,0,1))), shininess) * 0.25;
	
	FragColor = vec4(vertex_color_fs.rgb * (diffuse + ambient) + vec3(specular), vertex_color_fs.a);
}
