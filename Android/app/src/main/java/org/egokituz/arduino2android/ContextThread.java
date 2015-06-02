package org.egokituz.arduino2android;

import android.content.Context;
import android.os.Handler;
import android.os.Message;

import org.egokituz.arduino2android.models.ContextData;

/**
 * Created by User on 01/06/2015.
 */
public class ContextThread extends Thread {

    private static final String TAG = "ContextThread";

    private ContextData oldContext;
    private Speaker speaker;

    /**
     * Context of the main {Application} of this app
     */
    private Context m_AppContext;

    /**
     * Handler of the main {Application} class
     */
    private Handler m_mainHandler;

    public Handler contextThreadHandler = new Handler(){
        @SuppressWarnings("unchecked")
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case ContextData.CONTEXT_DATA:
                    contextChanged((ContextData) msg.obj);
                    break;
            }
        }
    };

    private boolean speedWarned = false;
    private boolean temperatureRiseWarned = false;
    private boolean temperatureFallWarned = false;

    private void contextChanged(ContextData newContext) {
        if(oldContext!=null){
            if(!oldContext.getActivity().equals(newContext.getActivity())){
                speaker.speak("You are now "+newContext.getActivity());
                speaker.pause(Speaker.SHORT_DURATION);
            }
            if(oldContext.getSpeed() < newContext.getSpeed() && !speedWarned){
                speedWarned = true;
                speaker.speak("You are going faster");
                speaker.pause(Speaker.SHORT_DURATION);
            }
            if(!oldContext.isBackpack_open() && newContext.isBackpack_open()){
                speaker.speak("Your backpack is now open");
                speaker.pause(Speaker.SHORT_DURATION);
                if(newContext.getActivity().equals("Walking")){
                    speaker.speak("You are getting robbed!");
                    speaker.pause(Speaker.SHORT_DURATION);
                }
            } else if(oldContext.isBackpack_open() && !newContext.isBackpack_open()){
                speaker.speak("Your backpack is now closed.");
                speaker.pause(Speaker.SHORT_DURATION);
            }
            if(newContext.getTc()>25.0 ) {
                if(!temperatureRiseWarned) {
                    temperatureRiseWarned = true;
                    temperatureFallWarned = false;
                    speaker.speak("Temperature is rising. Go eat an ice cream!");
                    speaker.pause(Speaker.SHORT_DURATION);
                }
            } else
                temperatureRiseWarned = false;
            if(newContext.getTc()<22.0){
                if(!temperatureFallWarned) {
                    temperatureFallWarned = true;
                    temperatureRiseWarned = false;
                    speaker.speak("Temperature is falling, go grab a coat!");
                    speaker.pause(Speaker.SHORT_DURATION);
                }
            } else
                temperatureFallWarned = false;
            if(!oldContext.getLocality().equals(newContext.getLocality())){
                if(oldContext.getLocality().equals("Linz") && newContext.getLocality().equals("Hagenberg")) {
                    speaker.speak("You arrived safely in Hagenberg.");
                }
                if(newContext.getLocality().equals("Disco"))
                    speaker.speak("Move that butt to the beat!");
            }
        }
        oldContext = newContext;
    }


    public ContextThread(Context mainCtx, Handler mainHandler) {
        super();
        setName("ContextThread");
        m_AppContext = mainCtx;
        m_mainHandler = mainHandler;

        speaker = new Speaker(m_AppContext);

        //TODO remove hardcoded initial context
        oldContext = new ContextData("standstill closed 10.0 Linz");

        speaker.speak("Hello, I am your sentient backpack!");
        speaker.pause(Speaker.SHORT_DURATION);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        speaker.destroy();
    }
}
