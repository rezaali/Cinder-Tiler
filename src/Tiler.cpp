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

Tiler::Tiler( int32_t imageWidth, int32_t imageHeight, int32_t tileWidth, int32_t tileHeight, ci::app::WindowRef window, bool alpha )
: mImageWidth( app::toPixels( imageWidth ) ), mImageHeight( app::toPixels( imageHeight ) ), mWindowRef( window ), mDrawFn( nullptr ), mDrawBgFn( nullptr ), mDrawHudFn( nullptr ), mAlpha( alpha )
{
    mWindowWidth = app::toPixels( mWindowRef->getWidth() );
    mWindowHeight = app::toPixels( mWindowRef->getHeight() );
    
    mTileWidth = std::min( ( int32_t ) app::toPixels( tileWidth ), mWindowWidth );
    mTileHeight = std::min( ( int32_t ) app::toPixels( tileHeight ), mWindowHeight );

    mNumTilesX = ( int32_t ) ceil( mImageWidth / (float)mTileWidth );
    mNumTilesY = ( int32_t ) ceil( mImageHeight / (float)mTileHeight );
    
    mCurrentTile = -1;
    
    if( mAlpha ) {
        mFboRef = gl::Fbo::create( mWindowWidth, mWindowHeight, mAlpha );
        mFboRef->bindFramebuffer();
        gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );
        mFboRef->unbindFramebuffer();
    }
}

bool Tiler::nextTile()
{
    if( mCurrentTile >= mNumTilesX * mNumTilesY ) {
        if( mAlpha ) {
            mSurface.copyFrom( mFboRef->readPixels8u( Area( ivec2( 0 ) , ci::app::toPixels( mWindowRef->getSize() ) ), mCurrentArea.getHeight() ), Area( 0, 0, mCurrentArea.getWidth(), mCurrentArea.getHeight() ), mCurrentArea.getUL() );
        } else {
            mSurface.copyFrom( mWindowRef->getRenderer()->copyWindowSurface( Area( ivec2( 0 ) , mWindowRef->getSize() ), mCurrentArea.getHeight() ), Area( 0, 0, mCurrentArea.getWidth(), mCurrentArea.getHeight() ), mCurrentArea.getUL() );
        }
        mCurrentTile = -1;
        return false;
    }
    
    if( mCurrentTile == -1 ) {
        mCurrentTile = 0;
        mSurface = Surface( mImageWidth, mImageHeight, mAlpha );
    }
    else {
        if( mAlpha ) {
            mSurface.copyFrom( mFboRef->readPixels8u( Area( ivec2( 0 ) , ci::app::toPixels( mWindowRef->getSize() ) ), mCurrentArea.getHeight() ), Area( 0, 0, mCurrentArea.getWidth(), mCurrentArea.getHeight() ), mCurrentArea.getUL() );
        } else {            
            mSurface.copyFrom( mWindowRef->getRenderer()->copyWindowSurface( Area( ivec2( 0 ) , ci::app::toPixels( mWindowRef->getSize() ) ), mCurrentArea.getHeight() ), Area( 0, 0, mCurrentArea.getWidth(), mCurrentArea.getHeight() ), mCurrentArea.getUL() );
        }
    }
    
    int tileX = mCurrentTile % mNumTilesX;
    int tileY = mCurrentTile / mNumTilesX;

    int currentTileWidth = ( ( tileX == mNumTilesX - 1 ) && ( mImageWidth != mTileWidth * mNumTilesX ) ) ? ( mImageWidth % mTileWidth ) : mTileWidth;
    int currentTileHeight = ( ( tileY == mNumTilesY - 1 ) && ( mImageHeight != mTileHeight * mNumTilesY ) ) ? ( mImageHeight % mTileHeight ) : mTileHeight;

    mCurrentArea.x1 = tileX * mTileWidth;
    mCurrentArea.x2 = mCurrentArea.x1 + currentTileWidth;
    mCurrentArea.y1 = tileY * mTileHeight;
    mCurrentArea.y2 = mCurrentArea.y1 + currentTileHeight;
    
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

bool Tiler::getAlpha()
{
    return mAlpha; 
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
    
    if( camera.isPersp() ) {
        frustum( left, right, bottom, top, nearPlane, farPlane );
    }
    else {
        ortho( left, right, bottom, top, nearPlane, farPlane );
    }
}

void Tiler::update()
{
    if( mAlpha ) {
        mFboRef->bindFramebuffer();
        gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );
    }
    
    float sx = (float) mCurrentArea.x1 / (float) mImageWidth;
    float sy = (float) mCurrentArea.y1 / (float) mImageHeight;
    float ex = (float) mCurrentArea.x2 / (float) mImageWidth;
    float ey = (float) mCurrentArea.y2 / (float) mImageHeight;
    
    vec2 ul = vec2(sx, sy);
    vec2 ur = vec2(ex, sy);
    vec2 lr = vec2(ex, ey);
    vec2 ll = vec2(sx, ey);
    
    float left = mCurrentFrustumCoords.x1 + mCurrentArea.x1 / (float)mImageWidth * mCurrentFrustumCoords.getWidth();
    float right = left + mCurrentArea.getWidth() / (float)mImageWidth * mCurrentFrustumCoords.getWidth();
    float top = mCurrentFrustumCoords.y1 + mCurrentArea.y1 / (float)mImageHeight * mCurrentFrustumCoords.getHeight();
    float bottom = top + mCurrentArea.getHeight() / (float)mImageHeight * mCurrentFrustumCoords.getHeight();

    if( mDrawBgFn ) {
        gl::pushMatrices();
        gl::pushViewport();
        gl::viewport( mCurrentArea.getSize() );
        mDrawBgFn( ul, ur, lr, ll );
        gl::popViewport();
        gl::popMatrices();
    }
    
    CameraPersp cam = mCamera;
    gl::pushMatrices();
    
    gl::pushViewport();
    gl::viewport( mCurrentArea.getSize() );

    gl::pushProjectionMatrix();
    if( mCurrentFrustumPersp ) {
        gl::setProjectionMatrix( glm::frustum( left, right, bottom, top, mCurrentFrustumNear, mCurrentFrustumFar ) );
    } else {
        gl::setProjectionMatrix( glm::ortho( left, right, bottom, top, mCurrentFrustumNear, mCurrentFrustumFar ) );
    }
    
    gl::pushViewMatrix();
    gl::setViewMatrix( cam.getViewMatrix() );
    
    if( mDrawFn ) {
        mDrawFn();
    }

    gl::popViewMatrix();
    gl::popProjectionMatrix();
    gl::popViewport();
    gl::popMatrices();
    
    if( mDrawHudFn ) {
        mDrawHudFn( ul, ur, lr, ll );
    }
    
    if( mAlpha ) {
        mFboRef->unbindFramebuffer();
    }
}