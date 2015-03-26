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
uniform mat4 projection_matrix;
uniform mat4 inverse_projection_matrix;
uniform bool is_perspective;			// Specifies the projection mode.
uniform vec2 viewport_origin;			// Specifies the transformation from screen coordinates to viewport coordinates.
uniform vec2 inverse_viewport_size;		// Specifies the transformation from screen coordinates to viewport coordinates.

#if __VERSION__ < 130
	#define in varying
	#define flat
	#define FragColor gl_FragColor
#else
	out vec4 FragColor;
#endif

// Inputs from vertex shader
flat in vec4 cylinder_color_fs;			// The base color of the cylinder.
flat in vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
flat in vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
flat in float cylinder_radius_sq_fs;	// The squared radius of the cylinder
flat in float cylinder_length;			// The length of the cylinder

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

	// Perform ray-cylinder intersection test.
	vec3 n = cross(ray_dir, cylinder_view_axis);
	float ln = length(n);
	vec3 RC = ray_origin - cylinder_view_base;
	
	vec3 view_intersection_pnt = ray_origin;
	
	if(ln < 1e-7) {
		float t = dot(RC, ray_dir);
		float v = dot(RC, RC);
		if(v-t*t > cylinder_radius_sq_fs) {
			discard;
			return;
		}
		view_intersection_pnt -= t * ray_dir;		
		float tfar = dot(cylinder_view_axis, ray_dir);
		if(tfar < 0.0) {
			view_intersection_pnt += tfar * ray_dir;
		}
	}
	else {
	
		n /= ln;
		float d = dot(RC,n);
		d *= d;
		
		// Test if ray missed the cylinder.
		if(d > cylinder_radius_sq_fs) {
			discard;
			return;
		}
			
		// Calculate closest intersection position.
		float t = dot(cross(cylinder_view_axis, RC), n) / ln;
		float s = abs(sqrt(cylinder_radius_sq_fs - d) / dot(cross(n, cylinder_view_axis),ray_dir) * cylinder_length);
		float tnear = t - s;
	
		// Calculate intersection point in view coordinate system.
		view_intersection_pnt += tnear * ray_dir;
	
		// Find intersection position along cylinder axis.
		float anear = dot(view_intersection_pnt - cylinder_view_base, cylinder_view_axis) / (cylinder_length*cylinder_length);
		if(anear >= 0 && anear <= 1.0) {
		}
		else {
			// Calculate second intersection point.
			float tfar = t + s;
			vec3 far_view_intersection_pnt = ray_origin + tfar * ray_dir;
			float afar = dot(far_view_intersection_pnt - cylinder_view_base, cylinder_view_axis) / (cylinder_length*cylinder_length);
			
			if(anear < 0 && afar >= 0) {
				view_intersection_pnt += (anear / (anear - afar) * 2.0 * s) * ray_dir;
			}
			else if(anear > 1.0 && afar < 1.0) {
				view_intersection_pnt += ((anear - 1.0) / (anear - afar) * 2.0 * s) * ray_dir;
			}
			else {
				discard;
				return;
			}
		}
	}
		
	// Output the ray-cylinder intersection point as the fragment depth 
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device 
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = projection_matrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = (projected_intersection.z / projected_intersection.w + 1.0) * 0.5;

	FragColor = cylinder_color_fs;
}
