package org.egokituz.arduino2android;

import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.util.List;
import java.util.Locale;

/**
 * Created by User on 19/05/2015.
 */
public class GPSModule extends Thread implements LocationListener {

    private static final String TAG = "GPSModule";
    private boolean terminate_flag = false;
    private Context m_mainContext;
    private Handler m_mainHandler;

    private Location m_lastLocation = new Location("");
    private float distance_since_last_location = 0.0f;


    public GPSModule(Context mainContext, Handler mainHandler){
        m_mainContext = mainContext;
        m_mainHandler = mainHandler;
        LocationManager locationManager = (LocationManager) m_mainContext.getSystemService(Context.LOCATION_SERVICE);

        //TODO Turn location on if it is turned off

        // Register the listener with the Location Manager to receive location updates
        locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 0, 0, this);
    }

    @Override
    public synchronized void start() {
        super.start();

        Log.v(TAG, "GPS module started!");
    }

    @Override
    public void run() {
        super.run();
        while(!terminate_flag)
            loop();

        try {
            finalize();
        } catch (Throwable throwable) {
            throwable.printStackTrace();
        }
    }

    private void loop() {
        //Log.v(TAG, "coordinates: 00000000 000000000");
        //m_mainHandler.obtainMessage(TestApplication.MESSAGE_GPS, "coordinates: 000 000").sendToTarget();

        try {
            Thread.sleep(9000);
        } catch (InterruptedException e) {
            Log.e(TAG, "Error waiting in the loop of the GPS module");
            e.printStackTrace();
        }
    }

    public void finalize_thread(){
        terminate_flag = true;
    }

    @Override
    public void onLocationChanged(Location loc) {


        String cityName = null; // to get city/locality name from the location coordinates
        Geocoder gcd = new Geocoder(m_mainContext, Locale.getDefault());
        List<Address> addresses;
        try {
            addresses = gcd.getFromLocation(loc.getLatitude(),
                    loc.getLongitude(), 1);
            if (addresses.size() > 0)
                cityName = addresses.get(0).getLocality();
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        //distance_since_last_location = calculateDistance(m_lastLocation, loc);
        distance_since_last_location = m_lastLocation.distanceTo(loc);
        if(!loc.hasSpeed() || (loc.getSpeed() == 0.0 && m_lastLocation != null) ) {
            //float speed = distance_since_last_location/(loc.getTime()-m_lastLocation.getTime());
            float speed = distance_since_last_location*1000/(loc.getTime()-m_lastLocation.getTime());
            Log.v(TAG, "Calculated speed: " + speed);
            loc.setSpeed(speed);
        }
        Message sendMsg = m_mainHandler.obtainMessage(TestApplication.MESSAGE_GPS_LOCATION);
        sendMsg.obj = loc;
        sendMsg.arg1 = (int) distance_since_last_location;  // safe cast, since there's no way the distance (long) will be higher than the Integer.MAX_VALUE = 2147483647
        sendMsg.sendToTarget();

        m_lastLocation = loc; // replace the previous last known location with the new one

        Log.v(TAG, "Location changed: Lat: " + loc.getLatitude() + " Lng: " + loc.getLongitude());
        Log.v(TAG, "Speed: "+loc.getSpeed()+" Distance: "+distance_since_last_location);
        Log.v(TAG, "Current City/locality: " + cityName);

    }

    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {

    }

    @Override
    public void onProviderEnabled(String provider) {

    }

    @Override
    public void onProviderDisabled(String provider) {

    }

    /**
     * Calculates the distance between two locations
     * @param loc1
     * @param loc2
     * @return
     */
    private static long calculateDistance(Location loc1, Location loc2){
        return calculateDistance(loc1.getLatitude(), loc1.getLongitude(), loc2.getLatitude(), loc2.getLongitude());
    }

    /**
     * Calculates the distance between two GPS coordinates
     * @param lat1
     * @param lng1
     * @param lat2
     * @param lng2
     * @return
     */
    private static long calculateDistance(double lat1, double lng1, double lat2, double lng2) {
        double dLat = Math.toRadians(lat2 - lat1);
        double dLon = Math.toRadians(lng2 - lng1);
        double a = Math.sin(dLat / 2) * Math.sin(dLat / 2)
                + Math.cos(Math.toRadians(lat1))
                * Math.cos(Math.toRadians(lat2)) * Math.sin(dLon / 2)
                * Math.sin(dLon / 2);
        double c = 2 * Math.asin(Math.sqrt(a));
        long distanceInMeters = Math.round(6371000 * c);
        return distanceInMeters;
    }
}
