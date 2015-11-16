package com.kari.imgsdk;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import org.imgsdk.view.RenderSurfaceView;

public class RenderActivity extends Activity {

    private RenderSurfaceView mSurfaceView;
    private Button mBtnRun;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_render);
        findViews();
        setListeners();
    }

    private void findViews() {
        mSurfaceView = (RenderSurfaceView) findViewById(R.id.render_surface_view);
        mBtnRun = (Button) findViewById(R.id.render_btn_run);
    }

    private void setListeners() {
        mBtnRun.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mSurfaceView.setEffectCmd("reset");
                mSurfaceView.invalidate();
            }
        });
    }

}
