package org.egokituz.arduino2android.models;

import android.util.Log;

/**
 * Created by Elena on 01/06/2015.
 */
public class ContextData {

    private static final String TAG = "ContextData";

    public static final int CONTEXT_DATA = 69841326;

    final static String WALKING = "Walking";
    final static String STANDING = "Standing";
    final static String STAIRS = "Going through the stairs";
    final static String RUNNING = "Running";


    private float speed = 0;
    private String latitude = "not found";
    private String longitude = "not found";
    private boolean backpack_open = false;
    private String activity = "not detected";
    private double tc =0.0;
    private String locality = "Linz";

    private final String delims = "[ ]+";

    /**
     * Constructor from string containing data values.
     * String format: [speed backpackState activity temperature]
     * The data values are separated by a blank space.
     * All values must be available
     * @param str
     */
    public ContextData(String str){
        //TODO parse string to get values

        String[] tokens = str.split(delims);
        if(tokens.length != 4)
            Log.e(TAG, "Bad context string!");

        activity = tokens[0];
        String bagStatus = tokens[1];
        if(bagStatus.equals("open"))
            backpack_open = true;
        else
            backpack_open = false;
        tc = Double.parseDouble(tokens[2]);
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

    public String getLocality() {
        return locality;
    }

    public void setLocality(String locality) {
        this.locality = locality;
    }
}
