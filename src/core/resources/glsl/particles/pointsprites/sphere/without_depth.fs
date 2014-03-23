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

uniform sampler2D tex;			// The imposter texture.

#if __VERSION__ >= 130

	flat in vec4 particle_color_fs;
	out vec4 FragColor;

#else

	#define particle_color_fs gl_Color
	#define FragColor gl_FragColor
	#define texture texture2D
	
	#if __VERSION__ < 120
		#define gl_PointCoord gl_TexCoord[0].xy
	#endif

#endif

void main() 
{
	vec2 shifted_coords = gl_PointCoord - vec2(0.5, 0.5);
	if(dot(shifted_coords, shifted_coords) >= 0.25) discard;
	vec4 texValue = texture(tex, gl_PointCoord);
	
	// Specular highlights are stored in the green channel of the texture. 
	// Modulate diffuse color with brightness value stored in the red channel of the texture.
	FragColor = vec4(texValue.r * particle_color_fs.rgb + texValue.g, particle_color_fs.a);
}
