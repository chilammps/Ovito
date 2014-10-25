#include "TestWindow.h"
#include "OpenGLBuffer.h"
#include <core/rendering/viewport/OpenGLTexture.h>

#ifndef OVITO_GL_TEST_PARTICLE_WINDOW
#define OVITO_GL_TEST_PARTICLE_WINDOW

class ParticleWindow : public TestWindow
{
public:

	/// Constructor.
	ParticleWindow(int id) : TestWindow(id), _positionsBuffer(id), _colorsBuffer(id), _radiiBuffer(id) {}

	OpenGLBuffer<Ovito::Vector3> _positionsBuffer;
	OpenGLBuffer<Ovito::Color> _colorsBuffer;
	OpenGLBuffer<Ovito::FloatType> _radiiBuffer;

	void initParticleBuffers(int verticesPerParticle) {
		_positionsBuffer.create(QOpenGLBuffer::StaticDraw, 2, verticesPerParticle);
		Ovito::Vector3 pos[2] = {{0,0,0.5}, {0.4,0.4,0.6}};
		_positionsBuffer.fill(pos);

		_colorsBuffer.create(QOpenGLBuffer::StaticDraw, 2, verticesPerParticle);
		Ovito::Color colors[2] = {{1,0,0}, {0,1,0}};
		_colorsBuffer.fill(colors);

		_radiiBuffer.create(QOpenGLBuffer::StaticDraw, 2, verticesPerParticle);
		Ovito::FloatType radii[2] = {0.5f, 0.35f};
		_radiiBuffer.fill(radii);
	}

	int particleCount() const { return 2; }

	/// The maximum resolution of the texture used for billboard rendering of particles. Specified as a power of two.
	static constexpr int BILLBOARD_TEXTURE_LEVELS = 8;

	/******************************************************************************
	* Creates the textures used for billboard rendering of particles.
	******************************************************************************/
	void initializeBillboardTexture()
	{
		using namespace Ovito;

		static std::vector<std::array<GLubyte,4>> textureImages[BILLBOARD_TEXTURE_LEVELS];
		static bool generatedImages = false;

		if(generatedImages == false) {
			generatedImages = true;
			for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
				int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));
				textureImages[mipmapLevel].resize(resolution*resolution);
				size_t pixelOffset = 0;
				for(int y = 0; y < resolution; y++) {
					for(int x = 0; x < resolution; x++, pixelOffset++) {
						Vector2 r((FloatType(x - resolution/2) + 0.5) / (resolution/2), (FloatType(y - resolution/2) + 0.5) / (resolution/2));
						FloatType r2 = r.squaredLength();
						FloatType r2_clamped = std::min(r2, FloatType(1));
						FloatType diffuse_brightness = sqrt(1 - r2_clamped) * 0.6 + 0.4;

						textureImages[mipmapLevel][pixelOffset][0] =
								(GLubyte)(std::min(diffuse_brightness, (FloatType)1.0) * 255.0);

						textureImages[mipmapLevel][pixelOffset][2] = 255;
						textureImages[mipmapLevel][pixelOffset][3] = 255;

						if(r2 < 1.0) {
							// Store specular brightness in alpha channel of texture.
							Vector2 sr = r + Vector2(0.6883, 0.982);
							FloatType specular = std::max(FloatType(1) - sr.squaredLength(), FloatType(0));
							specular *= specular;
							specular *= specular * (1 - r2_clamped*r2_clamped);
							textureImages[mipmapLevel][pixelOffset][1] =
									(GLubyte)(std::min(specular, FloatType(1)) * 255.0);
						}
						else {
							// Set transparent pixel.
							textureImages[mipmapLevel][pixelOffset][1] = 0;
						}
					}
				}
			}
		}

		_billboardTexture.create();
		_billboardTexture.bind();
		for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
			int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));

			OVITO_CHECK_OPENGL(glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGBA,
					resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImages[mipmapLevel].data()));
		}
	}

	/******************************************************************************
	* Activates a texture for billboard rendering of spherical particles.
	******************************************************************************/
	void activateBillboardTexture()
	{
		// Enable texture mapping when using compatibility OpenGL.
		// In the core profile, this is already enabled by default.
		if(isCoreProfile() == false)
			OVITO_CHECK_OPENGL(glEnable(GL_TEXTURE_2D));

		_billboardTexture.bind();

		OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
		OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		OVITO_STATIC_ASSERT(BILLBOARD_TEXTURE_LEVELS >= 3);
		OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, BILLBOARD_TEXTURE_LEVELS - 3));
	}

	/******************************************************************************
	* Deactivates the texture used for billboard rendering of spherical particles.
	******************************************************************************/
	void deactivateBillboardTexture()
	{
		// Disable texture mapping again when not using core profile.
		if(isCoreProfile() == false)
			OVITO_CHECK_OPENGL(glDisable(GL_TEXTURE_2D));
	}

	/// The OpenGL texture that is used for billboard rendering of particles.
	Ovito::OpenGLTexture _billboardTexture;

};

#endif

