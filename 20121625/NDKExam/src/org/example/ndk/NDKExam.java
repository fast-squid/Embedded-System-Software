package org.example.ndk;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;

public class NDKExam extends Activity {
	LinearLayout linear;
	public native int add(int x, int y);
	public native int testString(int mode);
	public native void close();
	@Override
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.main);
        
        System.loadLibrary("ndk-exam");
        
        //int z = add(x, y);
        super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		linear = (LinearLayout)findViewById(R.id.container);
		
		Button btn=(Button)findViewById(R.id.newactivity);
		OnClickListener listener=new OnClickListener(){
			public void onClick(View v){
				Intent intent=new Intent(NDKExam.this, NDKExam2.class);
				startActivity(intent);
			}
		};
		btn.setOnClickListener(listener);
	}



}
