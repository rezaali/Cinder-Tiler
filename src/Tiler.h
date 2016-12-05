#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/Window.h"
#include "cinder/app/AppBase.h"
#include "cinder/gl/gl.h"
#include "cinder/Exception.h"
#include "cinder/Surface.h"
#include "cinder/Camera.h"
#include "cinder/Area.h"
#include "glm/glm.hpp"

namespace reza { namespace tiler {

typedef std::shared_ptr<class Tiler> TilerRef;
class Tiler {
public:
    static TilerRef create( glm::ivec2 imageSize, glm::ivec2 tileSize = glm::ivec2( 512, 512 ), ci::app::WindowRef window = ci::app::getWindow(), bool alpha  = false )
    {
        return TilerRef( new Tiler( imageSize.x, imageSize.y, tileSize.x, tileSize.y, window, alpha ) );
    }
    
    static TilerRef create( int32_t imageWidth, int32_t imageHeight, int32_t tileWidth = 512, int32_t tileHeight = 512, ci::app::WindowRef window = ci::app::getWindow(), bool alpha = false )
    {
        return TilerRef( new Tiler( imageWidth, imageHeight, tileWidth, tileHeight, window, alpha ) );
    }
    
    bool nextTile();
    
    int32_t getImageWidth() const { return mImageWidth; }
    int32_t	getImageHeight() const { return mImageHeight; }
    float getImageAspectRatio() const { return mImageWidth / (float)mImageHeight; }
    ci::Area getCurrentTileArea() const { return mCurrentArea; }
    ci::Surface& getSurface();
    
    void setDrawBgFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawBgFn );
    void setDrawFn( const std::function<void()> &drawFn );
    void setDrawHudFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawHudFn );
    
    void setMatrices( const ci::CameraPersp &camera );
    void setMatricesWindowPersp( int screenWidth, int screenHeight, float fovDegrees, float nearPlane, float farPlane );
    void setMatricesWindow( int32_t windowWidth, int32_t windowHeight );
    void frustum( float left, float right, float bottom, float top, float nearPlane, float farPlane );
    void ortho( float left, float right, float bottom, float top, float nearPlane, float farPlane ); 
    
    bool getAlpha(); 
    
protected:
    Tiler( int32_t imageWidth, int32_t imageHeight, int32_t tileWidth = 512, int32_t tileHeight = 512, ci::app::WindowRef window = ci::app::getWindow(), bool alpha = false );
    
    void update();
    
    ci::app::WindowRef mWindowRef;
    
    int32_t mWindowWidth, mWindowHeight; 
    int32_t mImageWidth, mImageHeight;
    int32_t	mTileWidth, mTileHeight;
    int32_t	mNumTilesX, mNumTilesY;
    
    int32_t	mCurrentTile;
    ci::Area mCurrentArea;
    ci::Rectf mCurrentFrustumCoords;
    float mCurrentFrustumNear, mCurrentFrustumFar;
    bool mCurrentFrustumPersp;

    ci::CameraPersp mCamera;
    ci::SurfaceRef	mSurfaceRef;

    bool mAlpha = false;
    ci::gl::FboRef mFboRef = nullptr; 
    std::function<void()> mDrawFn = nullptr;
    std::function<void( glm::vec2,glm::vec2,glm::vec2,glm::vec2 )> mDrawBgFn = nullptr;
    std::function<void( glm::vec2,glm::vec2,glm::vec2,glm::vec2 )> mDrawHudFn = nullptr;
};

} } // namespace cinder::gl