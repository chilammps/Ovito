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
 * This OpenGL fragment shader renders a spherical atom using 
 * the raytracing method. 
 ***********************************************************************/

// Inputs from calling program
uniform bool isPerspective;			// Specifies the projection mode.
uniform vec2 viewport_origin;		// Specifies the transformation from pixel coordinates to viewport coordinates.
uniform vec2 inverse_viewport_size;	// Specifies the transformation from pixel coordinates to viewport coordinates.

// Inputs from vertex shader
varying vec3 particle_view_pos;			// Transformed sphere position in view coordinates
varying float particle_radius_squared;	// The squared radius

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

	vec3 sphere_dir = particle_view_pos - ray_origin;
	
	// Perform ray-sphere intersection test.
	float b = dot(ray_dir, sphere_dir);
	float temp = dot(sphere_dir, sphere_dir);
	float disc = b*b + particle_radius_squared - temp;
		
	// Only calculate the intersection closest to the viewer.
	if(disc <= 0.0)
		discard; // Ray missed sphere entirely, discard fragment
		
	// Calculate closest intersection position.
	float tnear = b - sqrt(disc);

	// Discard intersections behind the view point.
	if(isPerspective && tnear < 0.0)
		discard;
		
	// Calculate intersection point in view coordinate system.
	vec3 view_intersection_pnt = ray_origin + tnear * ray_dir;
	
	// Output the ray-sphere intersection point as the fragment depth 
	// rather than the depth of the bounding box polygons.
	// The eye coordinate Z value must be transformed to normalized device 
	// coordinates before being assigned as the final fragment depth.
	vec4 projected_intersection = gl_ProjectionMatrix * vec4(view_intersection_pnt, 1.0);
	gl_FragDepth = (projected_intersection.z / projected_intersection.w + 1.0) * 0.5;

	// Calculate surface normal in view coordinate system.
	vec3 surface_normal = normalize(view_intersection_pnt - particle_view_pos);
	
	float ambient = 0.4;
	float diffuse = abs(surface_normal.z) * 0.6;
	
	float shininess = 6.0;
	vec3 specular_lightdir = normalize(vec3(-1.8, 1.5, -0.2));
	float specular = pow(max(0.0, dot(reflect(specular_lightdir, surface_normal), ray_dir)), shininess) * 0.25;
	
	gl_FragColor = vec4(gl_Color.rgb * (diffuse + ambient) + vec3(specular), gl_Color.a);
}
