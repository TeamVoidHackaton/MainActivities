// ============================================================================
//	Includes
// ============================================================================

#include <jni.h>
#include <android/bitmap.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>

#include <AR/ar.h>
#include <AR/arMulti.h>
#include <AR/video.h>
#include <AR/gsub_es.h>
#include <AR/arFilterTransMat.h>
#include <AR/arosg.h>

//#include "ARNative.h"

#define LOG_LEVEL 10
/*for android logs*/
#include <android/log.h>
#define LOG_TAG "libFFMPEGstream"
#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <stdlib.h> // malloc()

// ============================================================================
//	Android NDK function signatures
// ============================================================================

#define JNIFUNCTION_NATIVE(sig) Java_com_example_ardrone_StreamingSurface_##sig
//#define JNIFUNCTION_NATIVE(sig) Java_com_example_ardrone_StreamingSurfaceView_##sig

// ============================================================================
//	Types
// ============================================================================

typedef struct VideoState {
	AVFormatContext *pFormatCtx;
	AVStream *pVideoStream;
	AVFrame* frame;
	int videoStreamIdx;
	int fint;
	int64_t nextFrameTime;		//track the next frame display time
	int status;
}VideoState;

typedef struct VideoDisplayUtil {
	struct SwsContext *img_resample_ctx;
	struct SwsContext *bgr_resample_ctx;
	AVFrame *pFrameRGBA;
	AVFrame *pFrameBGR;
	int width, height;
	void* pBitmap;	//use the bitmap as the buffer for pFrameRGBA
	void* pBGR;
	int numBytes;
	int frameNum;
} VideoDisplayUtil;

char *gVideoFileName;
VideoState *gvs;
VideoDisplayUtil *gvdu;

static ANativeWindow* theNativeWindow;


//
// Lifecycle functions.
// See http://developer.android.com/reference/android/app/Activity.html#ActivityLifecycle
//

JNIEXPORT jint JNICALL JNIFUNCTION_NATIVE(naGetVideoFrame(JNIEnv *pEnv, jobject pObj , jobject surface)) {

	VideoState* vs        = gvs;
	VideoDisplayUtil* vdu = gvdu;
	ARUint8* gVideoFrame;

	int width   = vs->pVideoStream->codec->width;
	int height  = vs->pVideoStream->codec->height;

	ANativeWindow* window = ANativeWindow_fromSurface(pEnv, surface);
	ANativeWindow_Buffer buffer;
	ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

	AVPacket packet;
	int framefinished;
	// Read frames and save first five frames to disk
	while ((!vs->status) && 0 == av_read_frame(vs->pFormatCtx, &packet)) {
		 // Is this a packet from the video stream?
		if (vs->videoStreamIdx == packet.stream_index) {
			//LOGI(10, "start decoding a new frame");
			// Decode video frame
			avcodec_decode_video2(vs->pVideoStream->codec, vs->frame, &framefinished, &packet);
			if (framefinished) {

				// Convert the image from its native format to RGB
       			sws_scale
				(
					vdu->img_resample_ctx,
					vs->frame->data,
					vs->frame->linesize,
					0,
					vs->pVideoStream->codec->height,
					vdu->pFrameRGBA->data,
					vdu->pFrameRGBA->linesize);

       			sws_scale
				(
					vdu->bgr_resample_ctx,
					vs->frame->data,
					vs->frame->linesize,
					0,
					vs->pVideoStream->codec->height,
					vdu->pFrameBGR->data,
					vdu->pFrameBGR->linesize);

				// Send frame in native format to ARtoolkit
				gVideoFrame = (ARUint8*)vdu->pFrameBGR->data[0];
				nativeVideoFrame(gVideoFrame);

				if (ANativeWindow_lock(window, &buffer, NULL) == 0) {
					memcpy(buffer.bits, vdu->pBitmap ,  width * height * 4);
					ANativeWindow_unlockAndPost(window);
				}

       			int64_t curtime = av_gettime();
				if (vs->nextFrameTime - curtime > 20*1000) {//only sleep if the delay bigger than 20 ms
					//LOGI(10, "sleep for %lld microseconds", vs->nextFrameTime - curtime);
					usleep(vs->nextFrameTime-curtime);
				}
				++vdu->frameNum;
				vs->nextFrameTime += vs->fint*1000;			//update the next frame play time, fint in milliseconds, convert to macro seconds
				//LOGI(10, "display thread processed frame %d", vdu->frameNum);
				return vdu->frameNum;
			}
		}
		av_free_packet(&packet);
		//ANativeWindow_release(window);
	}
	//LOGI(10, "end of decoding");
	ANativeWindow_release(window);
	return 0;
}

JNIEXPORT jint JNICALL JNIFUNCTION_NATIVE(naPrepareDisplay(JNIEnv *pEnv, jobject pObj, jobject pBitmap, jint width, jint height)) {

	VideoState* vs = gvs;
	VideoDisplayUtil* vdu = av_mallocz(sizeof(VideoDisplayUtil));
	gvdu = vdu;
	vs->frame = avcodec_alloc_frame();
	vdu->frameNum = 0;
	vdu->width  = width;
	vdu->height = height;
	vdu->pFrameRGBA = avcodec_alloc_frame();
	vdu->pFrameBGR  = avcodec_alloc_frame();

	// Allocating memory for the NV21 buffer
	vdu->numBytes = avpicture_get_size(PIX_FMT_NV21, vdu->width, vdu->height);
	vdu->pBGR = (uint8_t *)av_malloc(vdu->numBytes*sizeof(uint8_t));
	avpicture_fill((AVPicture*)vdu->pFrameBGR, vdu->pBGR, PIX_FMT_NV21, vdu->width, vdu->height);
	vdu->bgr_resample_ctx = sws_getContext(vs->pVideoStream->codec->width, vs->pVideoStream->codec->height, vs->pVideoStream->codec->pix_fmt, vdu->width, vdu->height, PIX_FMT_NV21, SWS_BICUBIC, NULL, NULL, NULL);

	if (NULL == vdu->bgr_resample_ctx) {
		LOGE(1, "error initialize the video NV21 frame conversion context");
		return -1;
	}

	AndroidBitmapInfo linfo;
	int lret;
	//1. retrieve information about the bitmap
	if ((lret = AndroidBitmap_getInfo(pEnv, pBitmap, &linfo)) < 0) {
		LOGE(1, "AndroidBitmap_getinfo failed! error = %d", lret);
		return -1;
	}
	if (linfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE(1, "bitmap format is not rgba_8888!");
		return- 1;
	}
	//2. lock the pixel buffer and retrieve a pointer to it
	if ((lret = AndroidBitmap_lockPixels(pEnv, pBitmap, &vdu->pBitmap)) < 0) {
		LOGE(1, "AndroidBitmap_lockpixels() failed! error = %d", lret);
		return -1;
	}
	//for android, we use the bitmap buffer as the buffer for pFrameRGBA
	avpicture_fill((AVPicture*)vdu->pFrameRGBA, vdu->pBitmap, PIX_FMT_RGBA, vdu->width, vdu->height);
	vdu->img_resample_ctx = sws_getContext(vs->pVideoStream->codec->width, vs->pVideoStream->codec->height, vs->pVideoStream->codec->pix_fmt, vdu->width, vdu->height, PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);

	if (NULL == vdu->img_resample_ctx) {
		LOGE(1, "error initialize the video RGBA frame conversion context");
		return -1;
	}

	vs->nextFrameTime = av_gettime() + 50*1000;	//introduce 50 milliseconds of initial delay

	//Initialize ARToolkit
	if (!nativeVideoInit(vdu->width, vdu->height, 0, 0)){
		LOGE(1, "Artoolkit is failing");
		return- 1;
	}

	return 0;
}

JNIEXPORT jint JNICALL JNIFUNCTION_NATIVE(naInit(JNIEnv *pEnv, jobject pObj, jstring pfilename)) {

	gVideoFileName = (char *)(*pEnv)->GetStringUTFChars(pEnv, pfilename, NULL);
	//LOGI(10, "video file name is %s", gVideoFileName);

	// Register all formats and codecs
	av_register_all();
	VideoState *vs;
	vs = av_mallocz(sizeof(VideoState));
	gvs = vs;
	av_register_all();

	// Open video file original
	gVideoFileName = "tcp://192.168.1.1:5555";
	if (0 != avformat_open_input(&vs->pFormatCtx, gVideoFileName, NULL, NULL)) {
		LOGE(1, "could not open video file: %s", gVideoFileName);
		return -1;
	}
	//LOGI(10, "opened file %s", gVideoFileName);

	// Retrieve stream information
	if (0 > avformat_find_stream_info(vs->pFormatCtx, NULL)) {
		LOGE(1, "could not find stream info");
		return -1;
	}

	// Locate first video stream
	int i;
	vs->videoStreamIdx = -1;
	for (i = 0; i < vs->pFormatCtx->nb_streams; ++i) {
		if (AVMEDIA_TYPE_VIDEO == vs->pFormatCtx->streams[i]->codec->codec_type) {
			vs->videoStreamIdx = i;
			vs->pVideoStream = vs->pFormatCtx->streams[i];
			break;
		}
	}
	if (-1 == vs->videoStreamIdx) {
		//LOGI(1, "could not find a video stream");
		return -1;
	}

	//Find the frame rate
	/*if(vs->pVideoStream->avg_frame_rate.den && vs->pVideoStream->avg_frame_rate.num) {
		vs->fint = 1000/av_q2d(vs->pVideoStream->avg_frame_rate);
    } else if(vs->pVideoStream->r_frame_rate.den && vs->pVideoStream->r_frame_rate.num) {
		vs->fint = 1000/av_q2d(vs->pVideoStream->r_frame_rate);
	} else if(vs->pVideoStream->time_base.den && vs->pVideoStream->time_base.num) {
		vs->fint = 1000*av_q2d(vs->pVideoStream->time_base);
	} else if(vs->pVideoStream->codec->time_base.den && vs->pVideoStream->codec->time_base.num) {
		vs->fint = 1000*av_q2d(vs->pVideoStream->codec->time_base);
	}*/
	/*if (vs->fint < 20) {
		vs->fint = 20;	//min interval 20 => max frame rate 1000/20=50;
	} else if (vs->fint > 100) {
		vs->fint = 100;	//max interval 100=> min frame rate 1000/100=10;
	} */
	vs->fint = 30;
	//LOGI(1, "frame display interval: %d ms", vs->fint);

	//Get the codec context for the video stream
	AVCodecContext *pcodecctx;
	pcodecctx = vs->pFormatCtx->streams[vs->videoStreamIdx]->codec;

	//Find the decoder for the video stream
	AVCodec *pcodec;
	pcodec = avcodec_find_decoder(pcodecctx->codec_id);
	if (NULL == pcodec) {
		LOGE(1, "unsupported codec");
		return -1;
	}

	//Open the codec
	if (0 > avcodec_open2(pcodecctx, pcodec, NULL)) {
		LOGE(1, "could not open codec");
		return -1;
	}
	vs->status = 0;
	return 0;
}

JNIEXPORT jint JNICALL JNIFUNCTION_NATIVE(naFinish(JNIEnv *pEnv, jobject pObj, jobject pBitmap)) {
	VideoDisplayUtil* vdu = gvdu;
	//free RGBA images
	AndroidBitmap_unlockPixels(pEnv, pBitmap);
	avcodec_free_frame(&vdu->pFrameRGBA);
	avcodec_free_frame(&vdu->pFrameBGR);
	VideoState* vs = gvs;
	avcodec_free_frame(&vs->frame);
	//close codec
	avcodec_close(vs->pVideoStream->codec);
	//close video file
	avformat_close_input(&vs->pFormatCtx);
	av_free(vs);
	return 0;
}

JNIEXPORT jintArray JNICALL JNIFUNCTION_NATIVE(naGetVideoRes(JNIEnv *pEnv, jobject pObj)) {
    jintArray lRes;
	AVCodecContext* vCodecCtx = gvs->pVideoStream->codec;
	if (NULL == vCodecCtx) {
		return NULL;
	}
	lRes = (*pEnv)->NewIntArray(pEnv, 2);
	if (lRes == NULL) {
		//LOGI(1, "cannot allocate memory for video size");
		return NULL;
	}
	jint lVideoRes[2];
	lVideoRes[0] = vCodecCtx->width;
	lVideoRes[1] = vCodecCtx->height;
	(*pEnv)->SetIntArrayRegion(pEnv, lRes, 0, 2, lVideoRes);
	return lRes;
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(naQuit(JNIEnv *pEnv, jobject pObj)) {
	//quit the playback
	gvs->status = 1;
}

JNIEXPORT jint JNICALL JNIFUNCTION_NATIVE(naGetDuration(JNIEnv *pEnv, jobject pObj)) {
	if (NULL != gvs->pFormatCtx) {
		return (gvs->pFormatCtx->duration / AV_TIME_BASE);
	} else {
		return -1;
	}
}

JNIEXPORT jint JNICALL JNIFUNCTION_NATIVE(naGetFrameRate(JNIEnv *pEnv, jobject pObj)) {
	return 1000/gvs->fint;
}


