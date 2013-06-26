#version 130 

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
 * This OpenGL fragment shader renders a shaded particle 
 * on a textured imposter.
 ***********************************************************************/

// Input from calling program:
uniform mat4 projection_matrix;
uniform sampler2D tex;			// The imposter texture.

// Input from vertex shader:
flat in vec4 particle_color_out;
flat in float depth_radius;		// The particle radius.
flat in float ze0;				// The particle's Z coordinate in eye coordinates.

out vec4 FragColor;

void main() 
{
	vec2 shifted_coords = gl_PointCoord - vec2(0.5, 0.5);
	float rsq = dot(shifted_coords, shifted_coords);
	if(rsq >= 0.25) discard;
	vec4 texValue = texture(tex, gl_PointCoord);
	
	// Specular highlights are stored in the green channel of the texture. 
	// Modulate diffuse color with brightness value stored in the red channel of the texture.
	FragColor = vec4(texValue.r * particle_color_out.rgb + texValue.g, particle_color_out.a);

	// Vary the depth value across the imposter to obtain proper intersections between particles.	
	float dz = sqrt(1.0 - 4.0 * rsq) * depth_radius;
	float ze = ze0 + dz;
	float zn = (projection_matrix[2][2] * ze + projection_matrix[3][2]) / (projection_matrix[2][3] * ze + projection_matrix[3][3]);
	gl_FragDepth = 0.5 * (zn * gl_DepthRange.diff + (gl_DepthRange.far + gl_DepthRange.near));
}
