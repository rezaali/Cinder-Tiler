#include "Tiler.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/Rand.h"

#include <algorithm>
#include <sstream>

using namespace reza::tiler;
using namespace cinder;
using namespace glm;
using namespace std;

Tiler::Tiler( int32_t imageWidth, int32_t imageHeight, int32_t tileWidth, int32_t tileHeight, ci::app::WindowRef window )
: mImageWidth( app::toPixels( imageWidth ) ), mImageHeight( app::toPixels( imageHeight ) ), mWindowRef( window ), mDrawFn( nullptr ), mDrawBgFn( nullptr ), mDrawHudFn( nullptr )
{
    mWindowWidth = app::toPixels( mWindowRef->getWidth() );
    mWindowHeight = app::toPixels( mWindowRef->getHeight() );
    
    mTileWidth = std::min( ( int32_t ) app::toPixels( tileWidth ), mWindowWidth );
    mTileHeight = std::min( ( int32_t ) app::toPixels( tileHeight ), mWindowHeight );

    mNumTilesX = ( int32_t ) ceil( mImageWidth / (float)mTileWidth );
    mNumTilesY = ( int32_t ) ceil( mImageHeight / (float)mTileHeight );
    
    mCurrentTile = -1;
    
//    cout << "mRetina Ratio: " << app::toPixels( 1.0 ) << endl;
//    cout << "mWindowWidth : " << mWindowWidth << " mWindowHeight: " << mWindowHeight << endl;
//    cout << "mImageWidth : " << mImageWidth << " mImageHeight: " << mImageHeight << endl;
//    cout << "mTileWidth : " << mTileWidth << " mTileHeight: " << mTileHeight << endl;
//    cout << "mNumTilesX : " << mNumTilesX << " mNumTilesY: " << mNumTilesY << endl;
}

bool Tiler::nextTile()
{
    if( mCurrentTile >= mNumTilesX * mNumTilesY ) {
        // suck the pixels out of the final tile
        mSurface.copyFrom( mWindowRef->getRenderer()->copyWindowSurface( Area( ivec2( 0 ) , mWindowRef->getSize() ), mWindowRef->getHeight() ), Area( 0, 0, mCurrentArea.getWidth(), mCurrentArea.getHeight() ), mCurrentArea.getUL() );
        mCurrentTile = -1;
        return false;
    }
    
    if( mCurrentTile == -1 )
    { // first tile of this frame
        mCurrentTile = 0;
        mSurface = Surface( mImageWidth, mImageHeight, false );
    }
    else {
        // suck the pixels out of the previous tile
        mSurface.copyFrom( mWindowRef->getRenderer()->copyWindowSurface( Area( ivec2( 0 ) , mWindowRef->getSize() ), mWindowRef->getHeight() ), Area( 0, 0, mCurrentArea.getWidth(), mCurrentArea.getHeight() ), mCurrentArea.getUL() );
    }
    
    int tileX = mCurrentTile % mNumTilesX;
    int tileY = mCurrentTile / mNumTilesX;

    int currentTileWidth = ( ( tileX == mNumTilesX - 1 ) && ( mImageWidth != mTileWidth * mNumTilesX ) ) ? ( mImageWidth % mTileWidth ) : mTileWidth;
    int currentTileHeight = ( ( tileY == mNumTilesY - 1 ) && ( mImageHeight != mTileHeight * mNumTilesY ) ) ? ( mImageHeight % mTileHeight ) : mTileHeight;

    mCurrentArea.x1 = tileX * mTileWidth;
    mCurrentArea.x2 = mCurrentArea.x1 + currentTileWidth;
    mCurrentArea.y1 = tileY * mTileHeight;
    mCurrentArea.y2 = mCurrentArea.y1 + currentTileHeight;
    
//    cout << "TILE NUMBER: " << mCurrentTile << endl;
//    cout << "mCurrentArea.x1: " << mCurrentArea.x1 << " mCurrentArea.y1: " << mCurrentArea.y1 << endl;
//    cout << "mCurrentArea.x2: " << mCurrentArea.x2 << " mCurrentArea.y2: " << mCurrentArea.y2 << endl;
//    cout << "AREA: " << mCurrentArea.getWidth() << " " << mCurrentArea.getHeight() << endl;
//    cout << endl; 
    
    update();
    mCurrentTile++;
    return true;
}

void Tiler::setMatricesWindowPersp( int screenWidth, int screenHeight, float fovDegrees, float nearPlane, float farPlane )
{
    CameraPersp cam( screenWidth, screenHeight, fovDegrees, nearPlane, farPlane );
    setMatrices( cam );
}

void Tiler::setMatricesWindow( int32_t windowWidth, int32_t windowHeight )
{
    ortho( 0, (float)windowWidth, (float)windowHeight, 0, -1.0f, 1.0f );
}

void Tiler::frustum( float left, float right, float bottom, float top, float nearPlane, float farPlane )
{
    mCurrentFrustumCoords = Rectf( vec2( left, top ), vec2( right, bottom ) );
    mCurrentFrustumNear = nearPlane;
    mCurrentFrustumFar = farPlane;
    mCurrentFrustumPersp = true;
}

void Tiler::ortho( float left, float right, float bottom, float top, float nearPlane, float farPlane )
{
    mCurrentFrustumCoords = Rectf( vec2( left, top ), vec2( right, bottom ) );
    mCurrentFrustumNear = nearPlane;
    mCurrentFrustumFar = farPlane;
    mCurrentFrustumPersp = false;
}

ci::Surface Tiler::getSurface()
{
    while ( nextTile() ) { }
    return mSurface;
}

void Tiler::setDrawBgFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawBgFn )
{
    mDrawBgFn = drawBgFn;
}

void Tiler::setDrawFn( const std::function<void()> &drawFn )
{
    mDrawFn = drawFn;
}

void Tiler::setDrawHudFn( const std::function<void( glm::vec2, glm::vec2, glm::vec2, glm::vec2 )> &drawHudFn )
{
    mDrawHudFn = drawHudFn;
}

void Tiler::setMatrices( const CameraPersp &camera )
{
    mCamera = camera;
    
    float left, top, right, bottom, nearPlane, farPlane;
    camera.getFrustum( &left, &top, &right, &bottom, &nearPlane, &farPlane );

//    cout << endl;
//    cout << "LEFT: " << left << endl;
//    cout << "RIGHT: " << right << endl;
//    cout << "TOP: " << top << endl;
//    cout << "BOTTOM: " << bottom << endl;
//    cout << "NEAR: " << nearPlane << endl;
//    cout << "FAR: " << farPlane << endl;
//    cout << endl;
    
    if( camera.isPersp() )
    {
        frustum( left, right, bottom, top, nearPlane, farPlane );
    }
    else
    {
        ortho( left, right, bottom, top, nearPlane, farPlane );
    }
}

void Tiler::update()
{
    float sx = (float) mCurrentArea.x1 / (float)mImageWidth;
    float sy = (float) mCurrentArea.y1 / (float)mImageHeight;
    float ex = (float) mCurrentArea.x2 / (float)mImageWidth;
    float ey = (float) mCurrentArea.y2 / (float)mImageHeight;
    
    vec2 ul = vec2(sx, sy);
    vec2 ur = vec2(ex, sy);
    vec2 lr = vec2(ex, ey);
    vec2 ll = vec2(sx, ey);

    if( mDrawBgFn )
    {
        mDrawBgFn( ul, ur, lr, ll );
    }
    
    CameraPersp cam = mCamera;
    gl::pushMatrices();
    gl::pushViewport();
    gl::viewport( 0, 0, mCurrentArea.getWidth(), mCurrentArea.getHeight() );
    gl::pushProjectionMatrix();

    float left = mCurrentFrustumCoords.x1 + mCurrentArea.x1 / (float)mImageWidth * mCurrentFrustumCoords.getWidth();
    float right = left + mCurrentArea.getWidth() / (float)mImageWidth * mCurrentFrustumCoords.getWidth();
    float top = mCurrentFrustumCoords.y1 + mCurrentArea.y1 / (float)mImageHeight * mCurrentFrustumCoords.getHeight();
    float bottom = top + mCurrentArea.getHeight() / (float)mImageHeight * mCurrentFrustumCoords.getHeight();
    
    if( mCurrentFrustumPersp )
    {
        gl::setProjectionMatrix( glm::frustum( left, right, bottom, top, mCurrentFrustumNear, mCurrentFrustumFar ) );
    }
    else
    {
        gl::setProjectionMatrix( glm::ortho( left, right, bottom, top, mCurrentFrustumNear, mCurrentFrustumFar ) );
    }
    
    gl::pushViewMatrix();
    gl::setViewMatrix( cam.getViewMatrix() );

    if( mDrawFn ) {
        mDrawFn();
    }

    gl::popViewMatrix();
    gl::pushProjectionMatrix();
    gl::popViewport();
    gl::popMatrices();
    
    if( mDrawHudFn ) {
        mDrawHudFn( ul, ur, lr, ll );
    }
}