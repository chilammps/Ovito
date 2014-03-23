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
	flat in float particle_radius_squared_fs;
	flat in vec3 particle_view_pos_fs;
	out vec4 FragColor;
#else
	#define particle_color_fs gl_Color
	#define particle_radius_squared_fs gl_TexCoord[1].w
	#define particle_view_pos_fs gl_TexCoord[1].xyz
	#define FragColor gl_FragColor
#endif

void main() 
{
	// Calculate the pixel coordinate in viewport space.
	vec2 view_c = ((gl_FragCoord.xy - viewport_origin) * inverse_viewport_size) - 1.0;

	// Calculate viewing ray direction in view space
	vec3 ray_dir;
	vec3 ray_origin;
	if(is_perspective) {
		ray_dir = normalize(vec3(inverse_projection_matrix * vec4(view_c.x, view_c.y, 1.0, 1.0)));
		ray_origin = vec3(0.0);
	}
	else {
		ray_origin = vec3(inverse_projection_matrix * vec4(view_c.x, view_c.y, 0.0, 1.0));
		ray_dir = vec3(0.0, 0.0, -1.0);
	}

	vec3 sphere_dir = particle_view_pos_fs - ray_origin;
	
	// Perform ray-sphere intersection test.
	float b = dot(ray_dir, sphere_dir);
	float temp = dot(sphere_dir, sphere_dir);
	float disc = b*b + particle_radius_squared_fs - temp;
		
	// Only calculate the intersection closest to the viewer.
	if(disc <= 0.0)
		discard; // Ray missed sphere entirely, discard fragment
		
	// Calculate closest intersection position.
	float tnear = b - sqrt(disc);

	// Discard intersections behind the view point.
	if(is_perspective && tnear < 0.0)
		discard;
		
	// Calculate intersection point in view coordinate system.
	vec3 view_intersection_pnt = ray_origin + tnear * ray_dir;
	
	// Output the ray-sphere intersection point as the fragment depth 
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device 
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = projection_matrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = (projected_intersection.z / projected_intersection.w + 1.0) * 0.5;

	FragColor = particle_color_fs;
}
