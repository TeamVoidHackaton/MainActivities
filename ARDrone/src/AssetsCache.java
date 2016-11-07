

public class AssetsCache extends Application {

	private static Application sInstance;
	 
	// Anywhere in the application where an instance is required, this method
	// can be used to retrieve it.
    public static Application getInstance() {
    	return sInstance;
    }
    
    @Override
    public void onCreate() {
    	super.onCreate(); 
    	sInstance = this;
    	((AssetsCache) sInstance).initializeInstance();
    }
    
    // Here we do one-off initialisation which should apply to all activities
	// in the application.
    protected void initializeInstance() {
    	
    	//PreferenceManager.setDefaultValues(this, org.artoolkit.ar.base.R.xml.preferences, false);
    	
		// Unpack assets to cache directory so native library can read them.
    	// N.B.: If contents of assets folder changes, be sure to increment the
    	// versionCode integer in the AndroidManifest.xml file.
		AssetHelper assetHelper = new AssetHelper(getAssets());        
    }
}