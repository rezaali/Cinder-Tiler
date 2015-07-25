#pragma once

#include "cinder/Cinder.h"
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
    static TilerRef create( int32_t imageWidth, int32_t imageHeight, int32_t tileWidth = 512, int32_t tileHeight = 512 )
    {
        return TilerRef( new Tiler( imageWidth, imageHeight, tileWidth, tileHeight ) );
    }
    
    bool nextTile();
    
    int32_t getImageWidth() const { return mImageWidth; }
    int32_t	getImageHeight() const { return mImageHeight; }
    float getImageAspectRatio() const { return mImageWidth / (float)mImageHeight; }
    ci::Area getCurrentTileArea() const { return mCurrentArea; }
    ci::Surface getSurface();
    
    void setDrawBgFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawBgFn );
    void setDrawFn( const std::function<void()> &drawFn );
    void setDrawHudFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawHudFn );
    
    void setMatrices( const ci::CameraPersp &camera );
    void setMatricesWindowPersp( int screenWidth, int screenHeight, float fovDegrees, float nearPlane, float farPlane );
    void setMatricesWindow( int32_t windowWidth, int32_t windowHeight );
    void frustum( float left, float right, float bottom, float top, float nearPlane, float farPlane );
    void ortho( float left, float right, float bottom, float top, float nearPlane, float farPlane ); 
    
protected:
    Tiler( int32_t imageWidth, int32_t imageHeight, int32_t tileWidth = 512, int32_t tileHeight = 512 );
    
    void update();
    
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
    ci::Surface	mSurface;
    std::function<void()> mDrawFn;
    std::function<void( glm::vec2,glm::vec2,glm::vec2,glm::vec2 )> mDrawBgFn;
    std::function<void( glm::vec2,glm::vec2,glm::vec2,glm::vec2 )> mDrawHudFn;
};

} } // namespace cinder::gl