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

// Input from calling program:
uniform mat4 projection_matrix;
uniform mat4 inverse_projection_matrix;
uniform bool is_perspective;
uniform vec2 viewport_origin;		// Specifies the transformation from screen coordinates to viewport coordinates.
uniform vec2 inverse_viewport_size;	// Specifies the transformation from screen coordinates to viewport coordinates.

#if __VERSION__ >= 130
	flat in vec4 particle_color_fs;
	flat in vec3 surface_normal_fs;
	out vec4 FragColor;
#else
	#define particle_color_fs gl_Color
	#define ec_pos gl_TexCoord[1].xyz
	#define FragColor gl_FragColor
#endif

const float ambient = 0.4;
const float diffuse_strength = 1.0 - ambient;
const float shininess = 6.0;
const vec3 specular_lightdir = normalize(vec3(-1.8, 1.5, -0.2));

void main() 
{
#if __VERSION__ < 130
	vec3 surface_normal_fs = normalize(cross(dFdx(ec_pos), dFdy(ec_pos)));
#endif

	// Calculate viewing ray direction in view space
	vec3 ray_dir;
	if(is_perspective) {
		// Calculate the pixel coordinate in viewport space.
		vec2 view_c = ((gl_FragCoord.xy - viewport_origin) * inverse_viewport_size) - 1.0;
		ray_dir = normalize(vec3(inverse_projection_matrix * vec4(view_c.x, view_c.y, 1.0, 1.0)));
	}
	else {
		ray_dir = vec3(0.0, 0.0, -1.0);
	}

	float diffuse = abs(surface_normal_fs.z) * diffuse_strength;
	float specular = pow(max(0.0, dot(reflect(specular_lightdir, surface_normal_fs), ray_dir)), shininess) * 0.25;
	FragColor = vec4(particle_color_fs.rgb * (diffuse + ambient) + vec3(specular), particle_color_fs.a);
}
