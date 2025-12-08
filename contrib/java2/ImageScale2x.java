import java.awt.image.BufferedImage;
import java.io.File;

import javax.imageio.ImageIO;

/**
 * A utility to perform the scale2x algorithm on a Java Image
 * 
 * @author Kevin Glass
 */
public class ImageScale2x
{
    /** The src data from the image */
    private int[] srcData;
    /** The width of the image */
    private int width;
    /** The height of the image */
    private int height;
    
    /**
     * Create a new scaler that will scale the passed image
     *
     * @param srcImage The image to be scaled
     */
    public ImageScale2x(BufferedImage srcImage)
    {
        width = srcImage.getWidth();
        height = srcImage.getHeight();
        
        srcData = new int[width*height];
        srcImage.getRGB(0,0,width,height,srcData,0,width);              
    }
    
    /**
     * Retrieve the scaled image. Note this is the method that actually 
     * does the work so it may take some time to return
     * 
     * @return The newly scaled image
     */
    public BufferedImage getScaledImage()
    {
        RawScale2x scaler = new RawScale2x(srcData,width,height);
        
        BufferedImage image = new BufferedImage(width*2,height*2,BufferedImage.TYPE_INT_ARGB);
        image.setRGB(0,0,width*2,height*2,scaler.getScaledData(),0,width*2);
        
        return image;
    }
    
    /**
     * An entry point and a bit of test code
     * 
     * @param argv The arguments passed in to the test code
     */
    public static void main(String argv[])
    {
        String srcFile = "randam_orig.png";
        try
        {
            System.out.println("Reading: "+srcFile);
            BufferedImage src = ImageIO.read(new File(srcFile));
            ImageScale2x scaler = new ImageScale2x(src);
            BufferedImage out = scaler.getScaledImage();

            String outFile = srcFile.substring(0,srcFile.length()-4);
            outFile += "2x";
            outFile += ".png";
            System.out.println("Writing: "+outFile);
            ImageIO.write(out,"PNG",new File(outFile));
        } 
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }
}
