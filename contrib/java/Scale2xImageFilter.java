/**
 * This class applies the Scale2x algorithm to scale an image up
 * by doubling the width and height, using a special calculation for
 * each pixel.
 * <p>
 * The algorithm was created, and generously shared, by Andrea Mazzoleni,
 * and implemented in this Java class by Randy Schnedler.
 * See the <a href="http://scale2x.sourceforge.net/">Scale2X</a>
 * web site for more information.
 * <p>
 * Note that this version is completely unoptimized and has some limitations,
 * and therefore has limited applications where speed is not critical.  Such
 * applications might be an image viewer or a front end for an emulator which
 * displays snapshots.
 * <p>
 * CHANGE LOG (latest first):
 * -----------------------------------
 * 2002-11-30  v0.1     R. Schnedler          Initial release
 * <p>
 * <p>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * <p>
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * @author Randy Schnedler (schnedlr@satx.rr.com)
 */

package com.schnedler.image;

import java.awt.image.ColorModel;
import java.awt.image.ReplicateScaleFilter;
import java.util.Hashtable;

public class Scale2xImageFilter extends ReplicateScaleFilter {

	public final static String version = "0.1";

	private ColorModel colorModel = null;

	/**
	 * Constructor for Scale2xImageFilter.
	 */
	public Scale2xImageFilter() {
		super(-1, -1); // call the super constructor with invalid arguments
	}

	/**
	 * Override the dimensions of the source image and pass the dimensions
	 * of the new scaled size to the ImageConsumer.
	 * <p>
	 * Note: This method is intended to be called by the 
	 * <code>ImageProducer</code> of the <code>Image</code> whose pixels 
	 * are being filtered. Developers using
	 * this class to filter pixels from an image should avoid calling
	 * this method directly since that operation could interfere
	 * with the filtering operation.
	 * @see ImageConsumer
	 */
	public void setDimensions(int w, int h) {
		srcWidth = w;
		srcHeight = h;
		destWidth = srcWidth * 2;
		destHeight = srcHeight * 2;

		consumer.setDimensions(destWidth, destHeight);
	}

	public void setProperties(Hashtable props) {
		props = (Hashtable) props.clone();
		String key = "scale2x";
		String val = destWidth + "x" + destHeight;
		Object o = props.get(key);
		if (o != null && o instanceof String) {
			val = ((String) o) + ", " + val;
		}
		props.put(key, val);
		super.setProperties(props);
	}

	/**
	 * Apply Scale2x here.
	 * FIXME: This method (setPixels() with a byte array) has not been
	 * tested, mainly because I don't readily have an ImageProducer which
	 * serves up bytes for the pixels.  Also, this method just copies the bytes
	 * to integers and calls the other method, which isn't very efficient.  Once
	 * the int method is optimized, this method will be re-written.
	 * 
	 * <p>
	 * Note: This method is intended to be called by the 
	 * <code>ImageProducer</code> of the <code>Image</code> whose pixels 
	 * are being filtered. Developers using
	 * this class to filter pixels from an image should avoid calling
	 * this method directly since that operation could interfere
	 * with the filtering operation.
	 */

	public void setPixels(
		int x,
		int y,
		int w,
		int h,
		ColorModel model,
		byte pixels[],
		int off,
		int scansize) {
		// System.out.println("Unexpectedly in Scale2xImageFilter.setPixels() with byte[]!");

		int newPixels[] = new int[pixels.length];

		for (int i = 0; i < pixels.length; i++) {
			newPixels[i] = pixels[i];
		}

		setPixels(x, y, w, h, model, newPixels, off, scansize);
	}

	/**
	 * This method is where the Scale2x algorithm is applied.
	 * <p>
	 * Note: This method is intended to be called by the 
	 * <code>ImageProducer</code> of the <code>Image</code> whose pixels 
	 * are being filtered. Developers using
	 * this class to filter pixels from an image should avoid calling
	 * this method directly since that operation could interfere
	 * with the filtering operation.
	 */
	public void setPixels(
		int x,
		int y,
		int w,
		int h,
		ColorModel model,
		int pixels[],
		int off,
		int scansize) {

		// "s"ource image pixels
		int e0, e1, e2, e3;
		int sa, sb, sc, sd, se, sf, sg, sh, si;

		boolean doTesselization = false; // Turn off tesselization

		for (x = 0; x < w; x++) {
			for (y = 0; y < h; y++) {
				sa = getPixel(x - 1, y - 1, pixels, w, h, doTesselization);
				sb = getPixel(x, y - 1, pixels, w, h, doTesselization);
				sc = getPixel(x + 1, y - 1, pixels, w, h, doTesselization);
				sd = getPixel(x - 1, y, pixels, w, h, doTesselization);
				se = getPixel(x, y, pixels, w, h, doTesselization);
				sf = getPixel(x + 1, y, pixels, w, h, doTesselization);
				sg = getPixel(x - 1, y + 1, pixels, w, h, doTesselization);
				sh = getPixel(x, y + 1, pixels, w, h, doTesselization);
				si = getPixel(x + 1, y + 1, pixels, w, h, doTesselization);
				/*
					ABC
					DEF
					GHI
				
					E0E1
					E2E3
				*/
				e0 = (sd == sb) && (sb != sf) && (sd != sh) ? sd : se;
				e1 = (sb == sf) && (sb != sd) && (sf != sh) ? sf : se;
				e2 = (sd == sh) && (sd != sb) && (sh != sf) ? sd : se;
				e3 = (sh == sf) && (sd != sh) && (sb != sf) ? sf : se;

				putPixel(x * 2, y * 2, model, e0);
				putPixel(x * 2 + 1, y * 2, model, e1);
				putPixel(x * 2, y * 2 + 1, model, e2);
				putPixel(x * 2 + 1, y * 2 + 1, model, e3);
			}
		}
	}

	private static int getPixel(
		int x,
		int y,
		int sourcePixels[],
		int sourceWidth,
		int sourceHeight,
		boolean doTesselization) {
		int returnPixel = 0;

		if (doTesselization) {
			if (x < 0)
				x += sourceWidth;
			if (x >= sourceWidth)
				x -= sourceWidth;
			if (y < 0)
				y += sourceHeight;
			if (y >= sourceHeight)
				y -= sourceHeight;
		} else {
			if (x < 0)
				x = 0;
			if (x >= sourceWidth)
				x = sourceWidth - 1;
			if (y < 0)
				y = 0;
			if (y >= sourceHeight)
				y = sourceHeight - 1;
		}

		returnPixel = sourcePixels[(y * sourceWidth) + x];

		return returnPixel;
	}

	void putPixel(int x, int y, ColorModel model, int newPixelValue) {
		int newPixelsArray[] = { newPixelValue };

		consumer.setPixels(x, y, 1, 1, model, newPixelsArray, 0, 1);
	}

}
