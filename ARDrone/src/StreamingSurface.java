package com.example.ardrone;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

	public class StreamingSurface {

	static {    	
    	System.loadLibrary("FFMPEG_ARTOOLKIT");
    }
	
	// Native libraries (FFMPEG)
	private static native int naInit(String pVideoFileName);
	private static native int[] naGetVideoRes();
	private static native int naGetDuration();
	private static native int naGetFrameRate();
	private static native int naFinish(Bitmap bitmap);
	private static native int naGetVideoFrame(Surface surface);
	private static native int naPrepareDisplay(Bitmap bitmap, int width, int height);
	private static native void naQuit();
	
	// Native libraries (ARTOOLKIT)
	private static native boolean nativeVideoInit(int w, int h, int cameraIndex, boolean cameraIsFrontFacing);
	private static native void nativeVideoFrame(byte[] image);
    
	// Private variables
	static AssetManager assetManager;
	private static final String TAG = "SimpleVideoView";
	private Bitmap mBitmap;
	private Bitmap mBitmapPikachu;
	private final Paint prFramePaint = new Paint();
	private int mDrawTop, mDrawLeft;
	public int mDisplayWidth, mDisplayHeight;	
	private Thread thread;
	private SurfaceHolder surfaceHolder;
	private int mStatus = 0;	//0: not started, 1: playing, 2: paused
	private int mVideoDuration = 0;
	private int mVideoCurrentFrame = 0;
	private int mVideoFR = 0;
	private String mVideoFileName = "";
	private int screenWidth;
	private int screenHeight;
	private Handler GLhandler;
	private Surface mSurface;
	
	public StreamingSurface(Surface surface) {
		
		mSurface     = surface;
		mDrawTop     = 0;
		mDrawLeft    = 0;
		mVideoCurrentFrame = 0;
		
		this.setVideoFile();
	}
	
	public void setVideoFile() {		
		
		// Initialise FFMPEG
		naInit("");
		
		// Get stream video res
		int[] res = naGetVideoRes();
		mDisplayWidth = (int)(res[0]);
		mDisplayHeight = (int)(res[1]);
		
		// Prepare Display

		// we are not performing any further image scaling
		mBitmap = Bitmap.createBitmap(mDisplayWidth, mDisplayHeight, Bitmap.Config.ARGB_8888);
		naPrepareDisplay(mBitmap, mDisplayWidth, mDisplayHeight);
	}
	
	public void grabFrame() {
		mVideoCurrentFrame = naGetVideoFrame(mSurface);
	}
}