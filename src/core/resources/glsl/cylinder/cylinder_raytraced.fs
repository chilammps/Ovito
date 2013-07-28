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

// Inputs from vertex shader
flat in vec4 cylinder_color_in;			// The base color of the cylinder.
flat in vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
flat in vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
flat in float cylinder_radius_in;		// The radius of the cylinder
flat in float cylinder_length;			// The length of the cylinder

out vec4 FragColor;

const float ambient = 0.4;
const float diffuse_strength = 1.0 - ambient;
const float shininess = 6.0;
const vec3 specular_lightdir = normalize(vec3(-1.8, 1.5, -0.2));

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

	vec3 RC = ray_origin - cylinder_view_base;
	
	// Perform ray-cylinder intersection test.
	vec3 n = cross(ray_dir, cylinder_view_axis);
	
	float ln = length(n);
	bool hitCylinder = false;
	vec3 surface_normal;
	vec3 view_intersection_pnt;
	
	// Check if ray is parallel to cylinder.
	if(ln > 1e-8) {
		
		n /= ln;
		float d = abs(dot(RC,n));
		
		// Check if ray missed cylinder.
		if(d > cylinder_radius_in)
			discard;
			
		// Calculate closest intersection position.
		vec3 O = cross(RC, cylinder_view_axis);
		float t = -dot(O, n) / ln;
		O = cross(n, cylinder_view_axis);
		float s = abs(sqrt(cylinder_radius_in*cylinder_radius_in - d*d) / dot(ray_dir, O) * cylinder_length);
		float tnear = t - s;
	
		// Calculate intersection point in view coordinate system.
		view_intersection_pnt = ray_origin + tnear * ray_dir;
		
		// Find intersection position along cylinder axis.
		float a = dot(view_intersection_pnt - cylinder_view_base, cylinder_view_axis) / (cylinder_length*cylinder_length);

		if(a >= 0 && a <= 1.0) {
			// Discard intersections behind the view point.
			if(!is_perspective || tnear > 0.0) {
		
				// Calculate surface normal in view coordinate system.
				surface_normal = (view_intersection_pnt - (cylinder_view_base + a * cylinder_view_axis)) / cylinder_radius_in;
			
				hitCylinder = true;
			}
		}
	}

	// Test for intersection with cylinder caps.
	if(!hitCylinder) {

		// Compute intersection of ray with first cap plane.
 		float d = dot(cylinder_view_axis, ray_dir);
 		if(abs(d) < 1e-8)
	 		discard;
		float t1 = dot(cylinder_view_base - ray_origin, cylinder_view_axis) / d;

		// Compute intersection of ray with second cap plane.
		float t2 = t1 + cylinder_length * cylinder_length / d;
 
 		if(t1 < t2) {
			if(length(ray_origin + t1 * ray_dir - cylinder_view_base) >= cylinder_radius_in)
				discard;
			surface_normal = -cylinder_view_axis / cylinder_length;
 			t1 += 3e-3;
			view_intersection_pnt = ray_origin + t1 * ray_dir;
	 	}
	 	else {
			if(length(ray_origin + t2 * ray_dir - cylinder_view_base - cylinder_view_axis) >= cylinder_radius_in)
				discard;
			surface_normal = cylinder_view_axis / cylinder_length;
 			t2 += 3e-3;
			view_intersection_pnt = ray_origin + t2 * ray_dir;
	 	}
	}
		
	// Output the ray-cylinder intersection point as the fragment depth 
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device 
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = projection_matrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = (projected_intersection.z / projected_intersection.w + 1.0) * 0.5;

	float diffuse = abs(surface_normal.z) * diffuse_strength;
	float specular = pow(max(0.0, dot(reflect(specular_lightdir, surface_normal), ray_dir)), shininess) * 0.25;
	
	FragColor = vec4(cylinder_color_in.rgb * (diffuse + ambient) + vec3(specular), cylinder_color_in.a);
}
