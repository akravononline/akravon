package com.cynex.mobil;

import android.app.NativeActivity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

// Cynex: dongu native'de, burasi tam ekran / klavye / pano / titresim
public class CynexActivity extends NativeActivity
{
	static
	{
		// Cynex: nativeOnText jni'den bulunsun diye
		System.loadLibrary("cynexmobil");
	}

	private static native void nativeOnText(String text);

	private ClipboardManager clipboard;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
		hideSystemBars();
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus)
	{
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus)
			hideSystemBars();
	}

	private void hideSystemBars()
	{
		getWindow().getDecorView().setSystemUiVisibility(
			View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
			| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
			| View.SYSTEM_UI_FLAG_FULLSCREEN
			| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
			| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
			| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event)
	{
		// Cynex: yazilabilir karakterler oyuna WM_CHAR olarak gider, tus kodu native'e de iner
		if (event.getAction() == KeyEvent.ACTION_MULTIPLE && event.getKeyCode() == KeyEvent.KEYCODE_UNKNOWN)
		{
			String s = event.getCharacters();
			if (s != null)
				nativeOnText(s);
			return true;
		}
		if (event.getAction() == KeyEvent.ACTION_DOWN)
		{
			int c = event.getUnicodeChar(event.getMetaState());
			if (c >= 32)
				nativeOnText(new String(Character.toChars(c)));
		}
		return super.dispatchKeyEvent(event);
	}

	// Cynex: native tus olayi -> unicode harf
	public int keyToChar(int deviceId, int keyCode, int metaState)
	{
		try
		{
			return android.view.KeyCharacterMap.load(deviceId).get(keyCode, metaState);
		}
		catch (Exception e)
		{
			return 0;
		}
	}

	public void vibrate(int milliseconds)
	{
		Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
		if (v == null || !v.hasVibrator())
			return;
		v.vibrate(VibrationEffect.createOneShot(milliseconds, VibrationEffect.DEFAULT_AMPLITUDE));
	}

	public void openUrl(final String url)
	{
		runOnUiThread(() -> {
			try
			{
				startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
			}
			catch (Exception ignored)
			{
			}
		});
	}

	public void showKeyboard(final boolean show)
	{
		runOnUiThread(() -> {
			InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
			View view = getWindow().getDecorView();
			if (imm == null)
				return;
			if (show)
				imm.showSoftInput(view, InputMethodManager.SHOW_FORCED);
			else
				imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
		});
	}

	public void setClipboard(final String text)
	{
		runOnUiThread(() -> {
			if (clipboard != null)
				clipboard.setPrimaryClip(ClipData.newPlainText("cynex", text));
		});
	}

	public String getClipboard()
	{
		if (clipboard == null || !clipboard.hasPrimaryClip())
			return "";
		ClipData clip = clipboard.getPrimaryClip();
		if (clip == null || clip.getItemCount() == 0)
			return "";
		CharSequence s = clip.getItemAt(0).coerceToText(this);
		return s != null ? s.toString() : "";
	}

	public String getCynexDeviceId()
	{
		String id = Settings.Secure.getString(getContentResolver(), Settings.Secure.ANDROID_ID);
		return id != null ? id : "";
	}

	public void showMessage(final String title, final String text)
	{
		runOnUiThread(() -> Toast.makeText(this, title.isEmpty() ? text : title + ": " + text, Toast.LENGTH_LONG).show());
	}
}
