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

    private void contextChanged(ContextData newContext) {
        if(oldContext!=null){
            if(!oldContext.getActivity().equals(newContext.getActivity())){
                speaker.speak("You are now "+newContext.getActivity());
                speaker.pause(Speaker.SHORT_DURATION);
            }
            if(oldContext.getLocality().equals("Linz") && newContext.getLocality().equals("Hagenberg")) {
                speaker.speak("You arrived safely in Hagenberg.");
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
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        speaker.destroy();
    }
}
