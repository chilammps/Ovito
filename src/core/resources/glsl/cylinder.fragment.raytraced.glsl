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

/***********************************************************************
 * This OpenGL fragment shader renders a cylindirical bond using 
 * the raytracing method. 
 ***********************************************************************/

// Inputs from calling program
uniform bool isPerspective;			// Specifies the projection mode.
uniform vec2 viewport_origin;		// Specifies the transformation from pixel coordinates to viewport coordinates.
uniform vec2 inverse_viewport_size;	// Specifies the transformation from pixel coordinates to viewport coordinates.

// Inputs from vertex shader
varying vec4 cylinder_color;			// The base color of the bond.
varying vec3 cylinder_view_base;		// Transformed cylinder position in view coordinates
varying vec3 cylinder_view_axis;		// Transformed cylinder axis in view coordinates
varying float cylinder_radius;			// The radius of the cylinder
varying float cylinder_length_squared;	// The squared length of the cylinder

void main() 
{
	// Calculate the pixel coordinate in viewport space first.
	vec2 view_c = ((gl_FragCoord.xy - viewport_origin) * inverse_viewport_size) - 1.0;		

	// Calculate viewing ray direction in world space
	vec3 ray_dir;
	vec3 ray_origin;
	if(isPerspective) {
		ray_dir = normalize(vec3(gl_ProjectionMatrixInverse * vec4(view_c.x, view_c.y, 1.0, 1.0)));
		ray_origin = vec3(0.0,0.0,0.0);
	}
	else {
		ray_origin = vec3(gl_ProjectionMatrixInverse * vec4(view_c.x, view_c.y, 0.0, 1.0));
		ray_dir = normalize(vec3(gl_ProjectionMatrixInverse * vec4(0.0, 0.0, 1.0, 0.0)));
	}

	vec3 RC = ray_origin - cylinder_view_base;
	
	// Perform ray-cylinder intersection test.
	vec3 n = cross(ray_dir, cylinder_view_axis);
	
	float ln = length(n);
	if(ln <= 1e-8)
		discard;	// Ray is parallel to cylinder, discard fragment.
		
	n /= ln;
	float d = abs(dot(RC,n));
	
	if(d > cylinder_radius) 
		discard;	// Ray missed cylinder, discard fragment.
			
	// Calculate closest intersection position.
	vec3 O = cross(RC, cylinder_view_axis);
	float t = -dot(O, n) / ln;
	O = cross(n, cylinder_view_axis);
	float s = abs(sqrt(cylinder_radius*cylinder_radius - d*d) / dot(ray_dir, O) * sqrt(cylinder_length_squared));
	float tnear = t - s;

	// Discard intersections behind the view point.
	if(isPerspective && tnear < 0.0)
		discard;
		
	// Calculate intersection point in view coordinate system.
	vec3 view_intersection_pnt = ray_origin + tnear * ray_dir;
	
	// Find position along cylinder axis.
	float a = dot(view_intersection_pnt - cylinder_view_base, cylinder_view_axis) / cylinder_length_squared;
	if(a < 0.0 || a > 1.0)
		discard;
		
	// Output the ray-cylinder intersection point as the fragment depth 
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device 
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = gl_ProjectionMatrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = (projected_intersection.z / projected_intersection.w + 1.0) * 0.5;

	// Calculate surface normal in view coordinate system.
	vec3 surface_normal = (view_intersection_pnt - (cylinder_view_base + a * cylinder_view_axis)) / cylinder_radius;
	
	float ambient = 0.4;
	float diffuse = abs(surface_normal.z) * 0.6;
	
	float shininess = 6.0;
	vec3 specular_lightdir = normalize(vec3(-1.8, 1.5, -0.2));
	float specular = pow(max(0.0, dot(reflect(specular_lightdir, surface_normal), ray_dir)), shininess) * 0.25;
	
	gl_FragColor.rgb = cylinder_color.rgb * (diffuse + ambient) + vec3(specular);
	gl_FragColor.a = cylinder_color.a;
}
