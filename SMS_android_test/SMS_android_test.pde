/*
Example code to receive SMS in Processing, Tested on Processing 2.0b7 and a phone on Android Jelly bean
by Julien Rat
feb 2013

Make sure to enable this permission in your sketch:
RECEIVE_SMS
*/
import java.net.URI;
import java.net.URL;
import java.net.*;
import java.lang.String;
import java.util.Locale;

//adding decode lbr, yang

import android.util.Base64;
import java.lang.Object.*;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import android.content.IntentFilter;
import android.os.Bundle;
import android.telephony.gsm.SmsMessage;
import android.telephony.gsm.SmsManager;
import android.net.Uri;

 //private Context mContext;

 boolean changed = true;
 boolean sent = false;

 String message = ""; 
 String number = ""; 
 
 int textSize = 100;
 
 SmsReceiver mySMSReceiver = new SmsReceiver();
 SmsManager sm = SmsManager.getDefault();
 
 void setup(){
  
   size(displayWidth, displayHeight,P3D);
   orientation(PORTRAIT);
   

 }
 
 void draw(){
   
   textSize(textSize);
   
   fill(0);
   text("STATUS",600, 4 * textSize + 20);  
   text("WHERE",600, 12 * textSize + 20);

   
   
   if (mousePressed && !sent)
   {
     fill(255, 0, 0);
     rect(0,0,displayWidth,displayHeight);
     sent = true;
     String outMsg = "";
     
     if (mouseY > displayHeight /2)
     {
       outMsg = "WHERE";
     }
     else
     {
       outMsg = "STATUS";
     }
     
     if (mouseX > displayWidth/2)
     {
       sm.sendTextMessage("17783194200", null, outMsg, null, null);
       delay(1000);
     }
     else
     {
       delay(5000);
       message = "SAFEZONE";
     }
     
     
   }
   else
   {
     sent = false;
   }
   
   
   
   if(message.equals(""))
   {
     
   }
   
   
   else{ // waiting for new messages
   
     if (changed)
     {
       fill(255);
       rect(0,0,displayWidth,displayHeight);
       fill(255,0,0);
       text("New message",10, 6 * textSize + 20);
       text(message,10, 7 * textSize + 20);
       text("From : "+number,10, 8 * textSize + 20);
       message=""; 
       
       changed = false;
     }
   }
 }
 
public class SmsReceiver extends BroadcastReceiver //Class to get SMS
{
    @Override
    public void onReceive(Context context, Intent intent) 
    {
        //---get the SMS message passed in---
        Bundle bundle = intent.getExtras();        
        SmsMessage[] msgs = null;
        String caller="";
        String str="";
                  
        if (bundle != null)
        {
            //---retrieve the SMS message received---
            Object[] pdus = (Object[]) bundle.get("pdus");
            msgs = new SmsMessage[pdus.length];            
            for (int i=0; i<msgs.length; i++){
                msgs[i] = SmsMessage.createFromPdu((byte[])pdus[i]);                
                caller += msgs[i].getOriginatingAddress();
                str += msgs[i].getMessageBody().toString();
            }
                    
        }
        
        //adding decoder to the code, remove original message passing
        //message=str;
        message=decode(str);
        
        if (message.contains("gps"))
        {
           //-122.848480,49.188087,2015/11/18,20:22:05
           
           String[] values = split(message, ',');
           
           //LAT , LON for uri. BUT FONA REVERSES THIS. FONA gives us LON, LAT
           String uri = "geo:";
           uri += values[1];
           uri += ",";
           uri += values[0];
           uri += "?q=";
           uri += values[1];
           uri += ",";
           uri += values[0];
           uri += "(" + "senior" + ")";
           
           message = values[1] + values[0];
                     
           Uri gMapsUrl = Uri.parse(uri);
           //Intent ii = new Intent();
           //ii.putExtra(Intent.ACTION_VIEW, gMapsUrl);
           Intent newIntent = new Intent(Intent.ACTION_VIEW, gMapsUrl );
           startActivity(newIntent);     
        }
        
        if (message.contains("Battery"))
        {
           String[] values = split(message, ". "); 
           message = values[1];
        }
       
        
        
        number=caller;  
        
        changed = true;
        
    }
}

//adding decode function

public String decode(String msg){
  byte[] decoded = Base64.decode(msg, Base64.DEFAULT);
  String decodeMsg = new String(decoded);
  return decodeMsg;
}
  



@Override
 public void onCreate(Bundle savedInstanceState) {
 super.onCreate(savedInstanceState);
  IntentFilter filter = new IntentFilter("android.provider.Telephony.SMS_RECEIVED");   
  getActivity().registerReceiver(mySMSReceiver, filter); // launch class when SMS are RECEIVED
}