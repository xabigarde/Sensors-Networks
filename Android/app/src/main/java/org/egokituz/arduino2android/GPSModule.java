package org.egokituz.arduino2android;

import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

import java.io.IOException;
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

    public GPSModule(Context mainContext, Handler mainHandler){
        m_mainContext = mainContext;
        m_mainHandler = mainHandler;
        LocationManager locationManager = (LocationManager) m_mainContext.getSystemService(Context.LOCATION_SERVICE);

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
        String coordinates = loc.getLatitude() + " " + loc.getLongitude();
        Log.v(TAG, "Location changed: Lat: " + loc.getLatitude() + " Lng: " + loc.getLongitude());

        /*------- To get city name from coordinates -------- */
        String cityName = null;
        Geocoder gcd = new Geocoder(m_mainContext, Locale.getDefault());
        List<Address> addresses;
        try {
            addresses = gcd.getFromLocation(loc.getLatitude(),
                    loc.getLongitude(), 1);
            if (addresses.size() > 0)
                System.out.println(addresses.get(0).getLocality());
            //cityName = addresses.get(0).getLocality();
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        Log.v(TAG, "My Current City is: " + cityName);

        m_mainHandler.obtainMessage(TestApplication.MESSAGE_GPS, coordinates).sendToTarget();
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
}
