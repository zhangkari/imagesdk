package com.kari.imgsdk.adapter;

import android.content.Context;
import android.graphics.BitmapFactory;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.TextView;

import com.kari.imgsdk.R;
import com.nostra13.universalimageloader.core.ImageLoader;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by ws-kari on 16-1-7.
 */
public class ImagePickerAdapter extends RecyclerView.Adapter<ImagePickerAdapter.PickerViewHolder> {

    private Context context;
    private LayoutInflater mInflater;
    private List<String> mDataSource;
    private AdapterView.OnItemClickListener mClickListener;
    private AdapterView.OnItemLongClickListener mLongClickListener;

    public ImagePickerAdapter(Context context) {
        this(context, new ArrayList<String>());
    }

    public ImagePickerAdapter(Context context, List<String> dataSource) {
        this.context = context;
        mInflater = LayoutInflater.from(context);
        mDataSource = dataSource;
    }

    public List<String> getDataSource() {
        return mDataSource;
    }

    @Override
    public PickerViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = mInflater.inflate(R.layout.picker_item_view, parent, false);
        return new PickerViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final PickerViewHolder holder, int position) {
        String path = mDataSource.get(position);
        ImageLoader.getInstance().displayImage(path, holder.image);
        holder.image.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (null != mClickListener) {
                    mClickListener.onItemClick(null, view, holder.getAdapterPosition(), holder.getItemId());
                }
            }
        });
        holder.image.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View view) {
                if (null != mLongClickListener) {
                    return mLongClickListener.onItemLongClick(null, view, holder.getAdapterPosition(), holder.getItemId());
                }
                return false;
            }
        });

        holder.type.setText(path.substring(path.lastIndexOf(".") + 1));

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(path, options);
        String aspect = context.getString(R.string.aspect_ration, options.outWidth, options.outHeight);
        holder.ratio.setText(aspect);
    }

    @Override
    public int getItemCount() {
        return mDataSource.size();
    }

    public void setItemClickListener(AdapterView.OnItemClickListener listener) {
        mClickListener = listener;
    }

    public void setItemLongClickListener(AdapterView.OnItemLongClickListener listener) {
        mLongClickListener = listener;
    }

    static class PickerViewHolder extends RecyclerView.ViewHolder {
        public ImageView image;
        public TextView ratio;
        public TextView type;

        public PickerViewHolder(View view) {
            super(view);
            image = (ImageView) view.findViewById(R.id.picker_image);
            ratio = (TextView) view.findViewById(R.id.picker_ratio);
            type = (TextView) view.findViewById(R.id.picker_type);
        }
    }
}
