package org.example.ndk;

import java.util.Random;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;

public class NDKExam2 extends Activity {
	LinearLayout linear;
	LinearLayout lr[] = new LinearLayout[4];
	Button pzl[];
	int posx, posy;
	int row, col;
	int b_pos[][] = new int[16][2];
	int target;
	int check = 0;

	OnClickListener listener2 = new OnClickListener(){
		@Override
		public void onClick(View v){
			int idx = v.getId();
		
			int r = b_pos[idx][0];
			int c = b_pos[idx][1];
			/* SWITCH BUTTONS */
			if((Math.abs(r - posy)==1 && Math.abs(c - posx)==0)
					||(Math.abs(r - posy)==0 && Math.abs(c - posx)==1)){				
				Button tmp = pzl[target];
				pzl[target] = pzl[r*col+c];
				pzl[r*col+c]=tmp;
				
				int pzlx = b_pos[pzl[target].getId()][1];
				int pzly = b_pos[pzl[target].getId()][0];
				b_pos[pzl[target].getId()][1]=b_pos[pzl[r*col+c].getId()][1];
				b_pos[pzl[target].getId()][0]=b_pos[pzl[r*col+c].getId()][0];
				b_pos[pzl[r*col+c].getId()][1]=pzlx;
				b_pos[pzl[r*col+c].getId()][0]=pzly;
				
				target=r*col+c;
				posx=target%col;
				posy=target/col;
				NDKExam jni = new NDKExam();
				jni.testString(0);
			}
			else return;
			
			for(int i=0;i<row;i++){
				linear.removeView(lr[i]);
			}
			
			/* CHECK SORTED */
			int cnt=0;
			for(int i=0;i< row * col ;i++){
				if(pzl[i].getText()=="") continue;
				if(Integer.parseInt(pzl[i].getText().toString())==i+1){
					cnt++;
				}
			}
			if(cnt==row*col-1){
				NDKExam jni = new NDKExam();
				check=1;
				jni.close();
				finish();
			}
			/* REFRESH VIEW */
			for(int i=0;i<row;i++){
				for(int j=0;j<col;j++){
					((LinearLayout)(View)pzl[i*col+j].getParent()).removeView(pzl[i*col+j]);
					lr[i].addView(pzl[i*col+j]);
				}
				linear.addView(lr[i]);
			}
		}
	};
	public void createPuzzle(int row,int col){
		
		/* Disable Start Button*/
		Button btn = (Button)findViewById(R.id.go);
		btn.setEnabled(false);
		
		/* Get Screen Size*/
		DisplayMetrics metrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(metrics);
		int height = metrics.heightPixels;
		int width = metrics.widthPixels;
		
		/* Generate Buttons */
		pzl = new Button[row*col];
		for(int i =0;i<row*col;i++){ 			
			pzl[i] = new Button(this);
			pzl[i].setOnClickListener(listener2);
			pzl[i].setId(i);
			pzl[i].setText(Integer.toString(i+1));
			pzl[i].setHeight((height-200)/row);
			pzl[i].setWidth(width/col);
			if(i == row*col-1){
				pzl[i].setBackgroundColor(Color.WHITE);
				pzl[i].setText("");
			}
			b_pos[i][0] = i/col;
			b_pos[i][1] = i%col;
		}
		
		/* Shuffle */
		posy = row-1;
		posx = col-1;
		target = posy*col+posx;
		for(int i=0;i<500;i++){
			Button temp;
			Random random=new Random();
			int tr = target/col;
			int tc = target%col;
			int sibal;
			temp = pzl[target];
			if(tc<col-1 && (random.nextInt()%2)==0){
				pzl[target]=pzl[target+1];
				pzl[target+1] = temp;
				
				sibal = b_pos[pzl[target].getId()][1];
				b_pos[pzl[target].getId()][1]=b_pos[pzl[target+1].getId()][1];				
				b_pos[pzl[target+1].getId()][1]=sibal;
				
				target+=1;
				posx+=1;
			}
			else if(tc>0 && (random.nextInt()%2)==0){
				pzl[target]=pzl[target-1];
				pzl[target-1] = temp;
				
				sibal = b_pos[pzl[target].getId()][1];
				b_pos[pzl[target].getId()][1]=b_pos[pzl[target-1].getId()][1];				
				b_pos[pzl[target-1].getId()][1]=sibal;
				
				target-=1;
				posx-=1;
			}
			else if(tr<row-1 && (random.nextInt()%2)==0){
				pzl[target]=pzl[target+col];
				pzl[target+col] = temp;
				
				sibal = b_pos[pzl[target].getId()][0];
				b_pos[pzl[target].getId()][0]=b_pos[pzl[target+col].getId()][0];				
				b_pos[pzl[target+col].getId()][0]=sibal;
				
				target+=col;
				posy+=1;
			}
			else if(tr>0 && (random.nextInt()%2)==0){
				pzl[target]=pzl[target-col];
				pzl[target-col] = temp;
				
				sibal = b_pos[pzl[target].getId()][0];
				b_pos[pzl[target].getId()][0]=b_pos[pzl[target-col].getId()][0];				
				b_pos[pzl[target- col].getId()][0]=sibal;
				
				target-=col;
				posy-=1;
			}
		}
		
		/* Draw Screen*/
		setContentView(R.layout.activity_main2);
		linear = (LinearLayout)findViewById(R.id.container);
		linear.setOrientation(LinearLayout.VERTICAL);
				
		for(int i=0;i<row;i++){
			lr[i] = new LinearLayout(this);
			lr[i].setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT,LayoutParams.WRAP_CONTENT));
			for(int j =0;j<col;j++){
				int idx = i*col+j;
				lr[i].addView(pzl[idx]);
			}
			linear.addView(lr[i]);
		}
		
		/* Back Thread */
		class BT extends Thread{
			Handler mHandler;
			BT(Handler handler){
				mHandler = handler;
			}
			public void run(){
				while(true){
					Message msg = Message.obtain();
					mHandler.sendMessage(msg);
					try{Thread.sleep(300);}
					catch(InterruptedException e){;}
					if(check==1) break;
				}
			}
		}

		Handler mHandler = new Handler(Looper.getMainLooper()){
			@SuppressWarnings("deprecation")
			public void handleMessage(Message msg){
					NDKExam jni = new NDKExam();
					int a = jni.testString(1);
					
					System.out.print(a);
					if(a==-1){
						check=1;
						jni.close();
						Thread.currentThread().interrupt();
						finish();
					}
			}
		};
		BT mt;
		
		mt=new BT(mHandler);
		mt.setDaemon(true);
		mt.start();
		
	}

	@Override	
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main2);
		
		
		Button btn = (Button)findViewById(R.id.go);
		OnClickListener listener=new OnClickListener(){
			public void onClick(View v){
				EditText et1 = (EditText)findViewById(R.id.editText1);
				String str = et1.getText().toString();
				String[] mat = str.split(" ");
				
				row = Integer.parseInt(mat[0]);
				col = Integer.parseInt(mat[1]);
				createPuzzle(row,col);
				
				System.out.println(row);
				System.out.println(col);
				NDKExam jni = new NDKExam();
				jni.add(0, 0);
			}
		};
		btn.setOnClickListener(listener);
	}
}
