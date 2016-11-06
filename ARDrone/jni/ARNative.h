#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include <AR/ar.h>
#include <stdbool.h>

// Camera
bool nativeVideoInit(int w, int h, int cameraIndex, bool cameraIsFrontFacing);
void nativeVideoFrame(ARUint8* gVideoFrame);
//static void nativeVideoGetCparamCallback(const ARParam *cparam_p, void *userdata);

static bool initARView(void);
static bool layoutARView(void);

#ifdef __cplusplus
}
#endif

