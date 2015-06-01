package org.egokituz.arduino2android.models;

/**
 * Created by Elena on 01/06/2015.
 */
public class ContextData {

    public static final int CONTEXT_DATA = 69841326;

    final static String WALKING = "Walking";
    final static String STANDING = "Standing";


    private float speed = 0;
    private String latitude = "not found";
    private String longitude = "not found";
    private boolean backpack_open = false;
    private String activity = "not detected";
    private double tc =0.0;

    /**
     * Constructor from string containing data values.
     * String format: [speed backpackState activity temperature]
     * The data values are separated by a blank space.
     * All values must be available
     * @param str
     */
    public ContextData(String str){
        //TODO parse string to get values

        //TODO remove this hardcoded data:
        backpack_open = true;
        activity = "Running0";
        tc = 20.0;
    }

    public void setTc(double tc) {
        this.tc = tc;
    }

    public void setSpeed(float speed) {
        this.speed = speed;
    }

    public void setLatitude(String latitude) {
        this.latitude = latitude;
    }

    public void setLongitude(String longitude) {
        this.longitude = longitude;
    }

    public void setBackpack_open(boolean backpack_open) {
        this.backpack_open = backpack_open;
    }

    public void setActivity(String activity) {
        this.activity = activity;
    }

    public float getSpeed() {

        return speed;
    }

    public String getLatitude() {
        return latitude;
    }

    public boolean isBackpack_open() {
        return backpack_open;
    }

    public double getTc() {
        return tc;
    }

    public String getLongitude() {
        return longitude;
    }

    public String getActivity() {
        return activity;
    }
}
