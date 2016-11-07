package com.example.ardrone;

import android.app.ActionBar;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;


public class MainActivity extends CardboardActivity implements CardboardView.StereoRenderer{

	// CARDBOARD declarations
    private static final String TAG = "MainActivity";
    private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;
    private FloatBuffer vertexBuffer, textureVerticesBuffer, vertexBuffer2;
    private ShortBuffer drawListBuffer, buf2;
    private int mProgram;
    private int mPositionHandle, mPositionHandle2;
    private int mColorHandle;
    private int mTextureCoordHandle;
    static final int COORDS_PER_VERTEX = 2;
    static float squareVertices[] = { // in counterclockwise order:
    	-1.0f, -1.0f,   // 0.left - mid
    	 1.0f, -1.0f,   // 1. right - mid
    	-1.0f, 1.0f,   // 2. left - top
    	 1.0f, 1.0f,   // 3. right - top
    };
    static float textureVertices[] = {
	 0.0f, 1.0f,  // A. left-bottom
	   1.0f, 1.0f,  // B. right-bottom
	   0.0f, 0.0f,  // C. left-top
	   1.0f, 0.0f   // D. right-top  
   };
    private short drawOrder[] =  {0, 2, 1, 1, 2, 3 }; 
    private short drawOrder2[] = {2, 0, 3, 3, 0, 1}; 
    private final int vertexStride = COORDS_PER_VERTEX * 4; 
    private ByteBuffer indexBuffer;    
    private float mAngle;
	
    private int loadGLShader(int type, String code) {
        
    	int shader = GLES20.glCreateShader(type);  // hate GLES 20 I wish I had 10 (but works with cardboard, which is nice)
        GLES20.glShaderSource(shader, code);
        GLES20.glCompileShader(shader);

        // Get the compilation status.
        final int[] compileStatus = new int[1];
        GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compileStatus, 0);

        // If the compilation failed, delete the shader.
        if (compileStatus[0] == 0) {
            Log.e(TAG, "Error compiling shader: " + GLES20.glGetShaderInfoLog(shader));
            GLES20.glDeleteShader(shader);
            shader = 0;
        }

        if (shader == 0) {
            throw new RuntimeException("Error creating shader.");
        }

        return shader;
    }

    /**
     * Checks if we've had an error inside of OpenGL ES, and if so what that error is.
     * @param func
     */
    private static void checkGLError(String func) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            Log.e(TAG, func + ": glError " + error);
            throw new RuntimeException(func + ": glError " + error);
        }
    }

    /**
     * Sets the view to our CardboardView and initializes the transformation matrices we will use
     * to render our scene.
     * @param savedInstanceState
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Initiliace CardboardView
        setContentView(R.layout.common_ui);
        cardboardView = (CardboardView) findViewById(R.id.cardboard_view);
        cardboardView.setRenderer(this);
        setCardboardView(cardboardView);
        
        
        // Calling ARToolkit native create method
        nativeCreate(this);
        
    }
  
    @Override
    public void onStart() 
    {
   	    super.onStart();
    	
   	    // Calling ARToolkit native start method
		nativeStart();
    }
 
    	
    @Override
    public void onSurfaceCreated(EGLConfig config) {
        
    	Log.i(TAG, "onSurfaceCreated");
		mSurface = new Surface(mSurfaceTexture);
		mSurfaceStreaming = new StreamingSurface(mSurface);
		
    }
    
    /**
     * Prepares OpenGL ES before we draw a frame.
     * @param headTransform The head transformation in the new frame.
     */
    @Override
    public void onNewFrame(HeadTransform headTransform) {
    	
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        
        // Update the surface texture holder with the last available frame
    	float[] mtx = new float[16];
        mSurfaceStreaming.grabFrame();
        mSurfaceTexture.updateTexImage();
        mSurfaceTexture.getTransformMatrix(mtx);

    }   

    /**
     * Draws a frame for an eye. The transformation for that eye (from the camera) is passed in as
     * a parameter.
     * @param transform The transformations to apply to render this eye.
     */
    @Override
    public void onDrawEye(EyeTransform transform) {
        
    	GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        
    }
    
    public void StreamSurfaceCreated(){
        
    	// COMPILE SHADERS
    	String vertexShaderCode =
    	        "attribute vec4 position;" +
    	        "attribute vec2 inputTextureCoordinate;" +
    	        "varying vec2 textureCoordinate;" +
    	        "void main()" +
    	        "{"+
    	            "gl_Position = position;"+
    	            "textureCoordinate = inputTextureCoordinate;" +
    	        "}";
        String fragmentShaderCode =
    	        "#extension GL_OES_EGL_image_external : require\n"+
    	        "precision mediump float;" +
    	        "varying vec2 textureCoordinate;                            \n" +
    	        "uniform samplerExternalOES s_texture;               \n" +
    	        "void main(void) {" +
    	        "  gl_FragColor = texture2D( s_texture, textureCoordinate );\n" +
    	        "}";
        int vertexShader = loadGLShader(GLES20.GL_VERTEX_SHADER, vertexShaderCode);
        int fragmentShader = loadGLShader(GLES20.GL_FRAGMENT_SHADER, fragmentShaderCode);
        
        
        // DECLARING BUFFERS
        ByteBuffer bb = ByteBuffer.allocateDirect(squareVertices.length * 4);
        bb.order(ByteOrder.nativeOrder());
        vertexBuffer = bb.asFloatBuffer();
        vertexBuffer.put(squareVertices);
        vertexBuffer.position(0);
       
        ByteBuffer bb2 = ByteBuffer.allocateDirect(textureVertices.length * 4);
        bb2.order(ByteOrder.nativeOrder());
        textureVerticesBuffer = bb2.asFloatBuffer();
        textureVerticesBuffer.put(textureVertices);
        textureVerticesBuffer.position(0);  
    	
    }
    
    public void StreamDrawFrame(){
    	
    	// GET ATTRIBUTE LOCATION & ENABLE VERTEX ARRAY
    	GLES20.glUseProgram(mProgram);
        
    	GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
        
        GLES20.glVertexAttribPointer(mTextureCoordHandle, COORDS_PER_VERTEX, GLES20.GL_FLOAT,false,vertexStride, textureVerticesBuffer);

        //mColorHandle = GLES20.glGetAttribLocation(mProgram, "s_texture");
        
        // DRAW
        GLES20.glDrawElements(GLES20.GL_TRIANGLES, drawOrder.length,GLES20.GL_UNSIGNED_SHORT, drawListBuffer);
            
        GLES20.glDisableVertexAttribArray(mPositionHandle);
        GLES20.glDisableVertexAttribArray(mTextureCoordHandle);
        
    }
    
    static private int createTexture()
    {
        int[] texture = new int[1];

        GLES20.glGenTextures(1,texture, 0);
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture[0]);
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL10.GL_TEXTURE_MIN_FILTER,GL10.GL_LINEAR);        
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE);
                   
        return texture[0];
    }
    
    @Override
    public void onFinishFrame(Viewport viewport) {
    }

    @Override
    public void onCardboardTrigger() {
    }
    
	    @Override
	public void onFrameAvailable(SurfaceTexture arg0) {
		this.cardboardView.requestRender();
	}    	

    
}