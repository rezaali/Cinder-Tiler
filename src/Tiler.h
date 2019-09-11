/*
Copyright(c) 2017 Reza Ali syed.reza.ali@gmail.com www.syedrezaali.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "cinder/Area.h"
#include "cinder/Camera.h"
#include "cinder/Cinder.h"
#include "cinder/Exception.h"
#include "cinder/Surface.h"
#include "cinder/app/AppBase.h"
#include "cinder/app/Window.h"
#include "cinder/gl/gl.h"
#include "glm/glm.hpp"

namespace reza {
namespace tiler {

typedef std::shared_ptr<class Tiler> TilerRef;
class Tiler {
  public:
	static TilerRef create( glm::ivec2 imageSize, glm::ivec2 tileSize = glm::ivec2( 512, 512 ), ci::app::WindowRef window = ci::app::getWindow(), bool alpha = false )
	{
		return TilerRef( new Tiler( imageSize.x, imageSize.y, tileSize.x, tileSize.y, window, alpha ) );
	}

	static TilerRef create( int imageWidth, int imageHeight, int tileWidth = 512, int tileHeight = 512, ci::app::WindowRef window = ci::app::getWindow(), bool alpha = false )
	{
		return TilerRef( new Tiler( imageWidth, imageHeight, tileWidth, tileHeight, window, alpha ) );
	}

	bool nextTile();

	int getImageWidth() const { return mImageWidth; }
	int getImageHeight() const { return mImageHeight; }
	float getImageAspectRatio() const { return mImageWidth / (float)mImageHeight; }
	ci::Area getCurrentTileArea() const { return mCurrentArea; }
	ci::Surface &getSurface();

	void setDrawBgFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawBgFn );
	void setDrawPostFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawPostFn );
	void setDrawFn( const std::function<void()> &drawFn );
	void setDrawHudFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawHudFn );

	void setMatrices( const ci::CameraPersp &camera );
	void setMatricesWindowPersp( int screenWidth, int screenHeight, float fovDegrees, float nearPlane, float farPlane );
	void setMatricesWindow( int windowWidth, int windowHeight );
	void frustum( float left, float right, float bottom, float top, float nearPlane, float farPlane );
	void ortho( float left, float right, float bottom, float top, float nearPlane, float farPlane );

	bool getAlpha();

  protected:
	Tiler( int imageWidth, int imageHeight, int tileWidth = 512, int tileHeight = 512, ci::app::WindowRef window = ci::app::getWindow(), bool alpha = false );

	void update();
	ci::gl::FboRef setupFbo();

	ci::app::WindowRef mWindowRef;

	int mWindowWidth, mWindowHeight;
	int mImageWidth, mImageHeight;
	int mTileWidth, mTileHeight;
	int mNumTilesX, mNumTilesY;

	int mCurrentTile;
	ci::Area mCurrentArea;
	ci::Rectf mCurrentFrustumCoords;
	float mCurrentFrustumNear, mCurrentFrustumFar;
	bool mCurrentFrustumPersp;

	ci::CameraPersp mCamera;
	ci::SurfaceRef mSurfaceRef;

	bool mAlpha = false;
	ci::gl::FboRef mFboRef = nullptr;
	ci::gl::FboRef mPostFboRef = nullptr;
	std::function<void()> mDrawFn = nullptr;
	std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> mDrawBgFn = nullptr;
	std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> mDrawPostFn = nullptr;
	std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> mDrawHudFn = nullptr;
};

} // namespace tiler
} // namespace reza