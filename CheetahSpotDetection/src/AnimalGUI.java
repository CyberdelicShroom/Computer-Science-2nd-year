import java.awt.Color;
import java.util.Scanner;

/**
 * This class converts a given image to greyscale, reduces noise in the image,
 * highlights the borders/edges within the image and identifies as well as
 * counts spots in the image such as images of cheetahs. It also implements a
 * basic GUI to display an image of each step of the process from greyscale to
 * spot detection.
 *
 * @author Keagan Selwyn Gill
 * @version 1.0
 */
public class AnimalGUI {
  static int count = 0;
  /**
   * Main method
   *
   * @param args input arguments
   */
  public static void main(String[] args) {
    if (args.length == 0 || args.length > 1) {
      System.err.println("ERROR: invalid number of arguments");
      System.exit(0);
    }
    String filename = "";
    try {
      filename = args[0] + "";
    } catch (Exception e) {
      System.err.println("ERROR: invalid argument type");
      System.exit(0);
    }

    Picture pic = new Picture(1, 1);
    try {
      pic = new Picture(filename);
    } catch (Exception e) {
      System.err.println("ERROR: invalid or missing file");
      System.exit(0);
    }

    String picName = "";

    String extensionType1 = 
    filename.substring(filename.length() - 4, filename.length());
    String extensionType2 = 
    filename.substring(filename.length() - 5, filename.length());

    String[] path = null;

    if (extensionType1.equals(".png") || extensionType1.equals(".jpg")) {
      String dir = filename.substring(0, filename.length() - 4);
      path = dir.split("/");
      picName = path[path.length - 1];
    } else if (extensionType2.equals(".jpeg")) {
      String dir = filename.substring(0, filename.length() - 5);
      path = dir.split("/");
      picName = path[path.length - 1];
    }

    int w = pic.width();
    int h = pic.height();
    StdDraw.setCanvasSize(w, h);
    StdDraw.picture(0.5, 0.5, filename);

    while (true) {
      if (StdDraw.hasNextKeyTyped()) {
        char input = StdDraw.nextKeyTyped();
        if (input == '0') {
          convertToGreyscale(pic, picName, input);
          StdDraw.clear();
          StdDraw.picture(0.5, 0.5, "../out/" + picName + "_GS.png");
          System.out.println("Displaying the greyscaled image");
        } else if (input == '1') {
          noiseReduction(pic, picName, input);
          StdDraw.clear();
          StdDraw.picture(0.5, 0.5, "../out/" + picName + "_NR.png");
          System.out.println("Displaying the noise reduced image");
        } else if (input == '2') {
          System.out.println("Enter epsilon value: ");
          Scanner in = new Scanner(System.in);

          double epsilon = 0;
          try {
            epsilon = in.nextDouble();
          } catch (Exception e) {
            System.err.println("ERROR: invalid argument type");
            System.exit(0);
          }
          if (epsilon < 0 || epsilon > 255) {
            System.err.println("ERROR: invalid epsilon");
            System.exit(0);
          }

          edgeDetection(pic, picName, epsilon, input);
          StdDraw.clear();
          StdDraw.picture(0.5, 0.5, "../out/" + picName + "_ED.png");
          System.out.println("Displaying the edge detected image "
               + "with an epsilon value of "
               + epsilon);
        } else if (input == '3') {
          System.out.println("Enter epsilon value: ");
          Scanner in = new Scanner(System.in);
          double epsilon = 0;
          try {
            epsilon = in.nextDouble();
          } catch (Exception e) {
            System.err.println("ERROR: invalid argument type");
            System.exit(0);
          }
          int lowerLimit = 0;
          int upperLimit = 0;
          try {
            System.out.println("Enter lower limit value: ");
            lowerLimit = in.nextInt();
            System.out.println("Enter upper limit value: ");
            upperLimit = in.nextInt();
          } catch (Exception e) {
            System.err.println("ERROR: invalid argument type");
            System.exit(0);
          }
          if (epsilon < 0 || epsilon > 255) {
            System.err.println("ERROR: invalid epsilon");
            System.exit(0);
          }

          Picture edges = edgesMap(pic, picName, epsilon);
          spotDetection(edges, picName, lowerLimit, upperLimit);

          StdDraw.clear();
          StdDraw.picture(0.5, 0.5, "../out/" + picName + "_SD.png");
          System.out.println(
              "Displaying the spot detected image with an epsilon value of "
              + epsilon + ", a lower radius limit of "
              + lowerLimit + " and a upper radius limit of " + upperLimit);
        } else if (input == 'q') {
          System.out.println("Exited the program");
          System.exit(0);
        }
      }
    }
  }

  /**
   * This method takes in an image, the image name (without the file extension)
   * and the mode that the program is running. It then captures the RGB values
   * of each pixel and converts them to the given greyscale criteria in order
   * to produce a new image that is greyscaled and saved into the 'out' folder.
   *
   * @param pic The input image
   * @param picName The image name (without the file extension)
   * @param input The key that the user entered as input which represents 
   * the current mode that the program must run.
   */
  public static void convertToGreyscale(Picture pic, String picName,
   char input) {

    int r = 0;
    int b = 0;
    int g = 0;

    int w = pic.width();
    int h = pic.height();
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        Color c = pic.get(x, y);
        r = c.getRed();
        b = c.getBlue();
        g = c.getGreen();

        int greyscale = (int) (0.299 * r + 0.114 * b + 0.587 * g);
        c = new Color(greyscale, greyscale, greyscale);

        pic.set(x, y, c);
      }
    }

    if (input == '0') {
      pic.save("../out/" + picName + "_GS.png");
    }
  }

  /**
   * This method takes in an image, the image name (without the file extension)
   * and the mode that the program is running. It then captures the greyscale
   * value of each pixel in the Von Neumann neighborhood. Finally, it does a
   * list of checks to set the centre pixels' value to the value of the majority
   * in the Von Neumann neighborhood, in order to reduce noise in the image.
   * The noise reduced image is then saved into the 'out' folder.
   *
   * @param pic The input image
   * @param picName The image name (without the file extension)
   * @param input The key that the user entered as input which represents 
   * the current mode that the program must run.
   */
  public static void noiseReduction(Picture pic, String picName, char input) {
    convertToGreyscale(pic, picName, input);

    int w = pic.width();
    int h = pic.height();
    Picture out = new Picture(w, h);
    int right = 0;
    int left = 0;
    int up = 0;
    int down = 0;
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        right = x + 1;
        left = x - 1;
        up = y - 1;
        down = y + 1;

        Color centrePixel = pic.get(x, y);
        Color rightPixel = null;
        Color leftPixel = null;
        Color upPixel = null;
        Color downPixel = null;

        int blueCentre = centrePixel.getBlue();
        int blueRight = 0;
        int blueLeft = 0;
        int blueUp = 0;
        int blueDown = 0;

        if (right < w) {
          rightPixel = pic.get(right, y);
          blueRight = rightPixel.getBlue();
        }

        if (left >= 0) {
          leftPixel = pic.get(left, y);
          blueLeft = leftPixel.getBlue();
        }

        if (up >= 0) {
          upPixel = pic.get(x, up);
          blueUp = upPixel.getBlue();
        }

        if (down < h) {
          downPixel = pic.get(x, down);
          blueDown = downPixel.getBlue();
        }

        if (blueLeft == blueUp && blueUp == blueRight
         && blueDown != blueUp) {
          out.set(x, y, new Color(blueUp, blueUp, blueUp));
        } else if (blueUp == blueRight && blueRight == blueDown
         && blueLeft != blueRight) {
          out.set(x, y, new Color(blueRight, blueRight, blueRight));
        } else if (blueRight == blueDown && blueDown == blueLeft
         && blueUp != blueDown) {
          out.set(x, y, new Color(blueDown, blueDown, blueDown));
        } else if (blueDown == blueLeft && blueLeft == blueUp
         && blueRight != blueLeft) {
          out.set(x, y, new Color(blueLeft, blueLeft, blueLeft));
        } else if (blueLeft == blueUp
            && blueUp == blueRight
            && blueRight == blueDown
            && blueDown == blueLeft) {
          out.set(x, y, new Color(blueUp, blueUp, blueUp));
        } else if (blueLeft == blueRight && blueUp != blueDown) {
          if (blueUp != blueCentre && blueDown != blueCentre) {
            out.set(x, y, new Color(blueLeft, blueLeft, blueLeft));
          } else if (blueUp == blueCentre) {
            out.set(x, y, new Color(blueUp, blueUp, blueUp));
          } else if (blueDown == blueCentre) {
            out.set(x, y, new Color(blueDown, blueDown, blueDown));
          }

        } else if (blueUp == blueDown && blueLeft != blueRight) {
          if (blueLeft != blueCentre && blueRight != blueCentre) {
            out.set(x, y, new Color(blueUp, blueUp, blueUp));
          } else if (blueLeft == blueCentre) {
            out.set(x, y, new Color(blueLeft, blueLeft, blueLeft));
          } else if (blueRight == blueCentre) {
            out.set(x, y, new Color(blueRight, blueRight, blueRight));
          }

        } else if (blueUp == blueRight && blueDown != blueLeft) {
          if (blueDown != blueCentre && blueLeft != blueCentre) {
            out.set(x, y, new Color(blueUp, blueUp, blueUp));
          } else if (blueDown == blueCentre) {
            out.set(x, y, new Color(blueDown, blueDown, blueDown));
          } else if (blueLeft == blueCentre) {
            out.set(x, y, new Color(blueLeft, blueLeft, blueLeft));
          }

        } else if (blueDown == blueLeft && blueUp != blueRight) {
          if (blueUp != blueCentre && blueRight != blueCentre) {
            out.set(x, y, new Color(blueLeft, blueLeft, blueLeft));
          } else if (blueUp == blueCentre) {
            out.set(x, y, new Color(blueUp, blueUp, blueUp));
          } else if (blueRight == blueCentre) {
            out.set(x, y, new Color(blueRight, blueRight, blueRight));
          }

        } else if (blueLeft == blueUp && blueDown != blueRight) {
          if (blueDown != blueCentre && blueRight != blueCentre) {
            out.set(x, y, new Color(blueLeft, blueLeft, blueLeft));
          } else if (blueDown == blueCentre) {
            out.set(x, y, new Color(blueDown, blueDown, blueDown));
          } else if (blueRight == blueCentre) {
            out.set(x, y, new Color(blueRight, blueRight, blueRight));
          }
        } else if (blueDown == blueRight && blueLeft != blueUp) {
          if (blueLeft != blueCentre && blueUp != blueCentre) {
            out.set(x, y, new Color(blueDown, blueDown, blueDown));
          } else if (blueLeft == blueCentre) {
            out.set(x, y, new Color(blueLeft, blueLeft, blueLeft));
          } else if (blueUp == blueCentre) {
            out.set(x, y, new Color(blueUp, blueUp, blueUp));
          }
        } else {
          out.set(x, y, new Color(blueCentre, blueCentre, blueCentre));
        }
      }
    } // end of loop
    if (input == '1') {
      out.save("../out/" + picName + "_NR.png");
    }
  }

  /**
   * This method takes in an image, the image name (without the file extension)
   * and an epsilon value. It then captures the greyscale value of each pixel in
   * the Von Neumann neighborhood. Finally, it determines the difference between
   * each pixel in the Von Neumann neighborhood to the centre pixel and checks
   * whether the difference is less than epsilon and sets the value of the
   * centre pixel accordingly in order to mark the edges within the image.
   * The last double for loop at the end changes the pixels on the far right
   * border and the bottom-most border of the image to black.
   * The new image is then saved into the 'out' folder.
   *
   * @param pic The input image
   * @param picName The image name (without the file extension)
   * @param epsilon Threshold required to mark the edges
   * @param input The key that the user entered as input which represents 
   * the current mode that the program must run.
   */
  public static void edgeDetection(Picture pic, String picName,
   double epsilon, char input) {
    noiseReduction(pic, picName, input);

    int w = pic.width();
    int h = pic.height();
    Picture out = new Picture(w, h);

    int right = 0;
    int left = 0;
    int up = 0;
    int down = 0;
    for (int y = 1; y < h; y++) {
      for (int x = 1; x < w; x++) {
        right = x + 1;
        left = x - 1;
        up = y - 1;
        down = y + 1;

        Color centrePixel = pic.get(x, y);
        Color rightPixel = null;
        Color leftPixel = null;
        Color upPixel = null;
        Color downPixel = null;

        int blueCentre = centrePixel.getBlue();
        int blueRight = 0;
        int blueLeft = 0;
        int blueUp = 0;
        int blueDown = 0;

        if (right < w) {
          rightPixel = pic.get(right, y);
          blueRight = rightPixel.getBlue();
        }

        if (left >= 0) {
          leftPixel = pic.get(left, y);
          blueLeft = leftPixel.getBlue();
        }

        if (up >= 0) {
          upPixel = pic.get(x, up);
          blueUp = upPixel.getBlue();
        }

        if (down < h) {
          downPixel = pic.get(x, down);
          blueDown = downPixel.getBlue();
        }
        if (Math.abs(blueCentre - blueRight) < epsilon
            && Math.abs(blueCentre - blueLeft) < epsilon
            && Math.abs(blueCentre - blueUp) < epsilon
            && Math.abs(blueCentre - blueDown) < epsilon) {
          Color newCentre = new Color(0, 0, 0);
          out.set(x, y, newCentre);
        } else {
          Color newCentre = new Color(255, 255, 255);
          out.set(x, y, newCentre);
        }
      }
    }
    for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
        if (j == w - 1) {
          out.set(w - 1, i, new Color(0, 0, 0));
        }
        if (i == h - 1) {
          out.set(j, h - 1, new Color(0, 0, 0));
        }
      }
    }
    if (input == '2') {
      out.save("../out/" + picName + "_ED.png");
    }
  }

  /**
   * Takes in a radius, width and delta value. Creates a square 2D int array
   * with values corresponding to either a white pixel or black pixel which
   * forms an optimal white spot/circle to compare to image blocks from the
   * edges image in order to help find spots.
   *
   * @param radius Radius
   * @param width Width
   * @param delta Delta
   * @return Returns the 2D int array "mask"
   */
  public static int[][] createSpotMask(int radius, int width, int delta) {
    int[][] xx = new int[2 * radius + 1][2 * radius + 1];
    int[][] yy = new int[2 * radius + 1][2 * radius + 1];
    int[][] circle = new int[2 * radius + 1][2 * radius + 1];
    boolean[][] donut = new boolean[2 * radius + 1][2 * radius + 1];
    int[][] mask = new int[2 * radius + 1][2 * radius + 1];

    for (int i = 0; i < 2 * radius + 1; i++) {
      for (int j = 0; j < 2 * radius + 1; j++) {
        xx[i][j] = i;
        yy[i][j] = j;

        circle[i][j] = (int) (Math.pow(xx[i][j] - radius, 2)
         + Math.pow(yy[i][j] - radius, 2));

        donut[i][j] =
            (circle[i][j] < (Math.pow(radius - delta, 2) + width))
                && (circle[i][j] > (Math.pow(radius - delta, 2) - width));

        if (donut[i][j]) {
          mask[i][j] = 255;
        } else {
          mask[i][j] = 0;
        }
      }
    }
    return mask;
  }

  /**
   * Creates an image block of the width and height of the mask, the 
   * current pixel being the center of the square. 
   * Copies the values from the edges map at these coordinates 
   * into this image block. Then, for every pixel in the image block: 
   * gets the absolute value of the difference between 
   * the pixel’s value and the pixel at the same coordinates 
   * in the mask and sums all these differences together. Check
   * to see whether the sum of differences is less than the diff value 
   * for this radius, if it is then it is a spot and this image block 
   * is added to the spots image, and increase the count by 1. Also,
   * removes the current image block from the edges map so that spots
   * are not re-counted. Finally, the spots image is returned.
   * 
   * @param edges Edges map (the edge detected image)
   * @param spots Spot detected image
   * @param mask 2D in array "mask"
   * @param radius Radius
   * @param diff Difference value associated to a specific radius
   * @return Returns the spot detected image called "Spots"
   */
  public static Picture findSpots(Picture edges, Picture spots,
   int[][] mask, int radius, int diff) {
    int sum = 0;
    
    for (int x = radius; x < edges.height() - radius; x++) {
      for (int y = radius; y < edges.width() - radius - 1; y++) {
        Picture imageBlock = new Picture(mask.length, mask[0].length);
        sum = 0;
        for (int i = 0; i < mask[0].length; i++) {
          for (int j = 0; j < mask.length; j++) {
            imageBlock.set(j, i, edges.get(y - radius + j, x - radius + i));
          }
        }

        for (int i = 0; i < imageBlock.height(); i++) {
          for (int j = 0; j < imageBlock.width(); j++) {
            sum = sum + Math.abs(imageBlock.get(j, i).getBlue() - mask[j][i]);
          }
        }

        if (sum < diff) {

          for (int p = 0; p < imageBlock.height(); p++) {
            for (int q = 0; q < imageBlock.width(); q++) {
              spots.set(y - radius + q, x - radius + p, imageBlock.get(q, p));
              edges.set(y - radius + q, x - radius + p, new Color(0, 0, 0));
            }
          }
          count += 1;
        }
      }
    }
    return spots;
  }

  /**
   * This method takes in an image, the image name (without the file extension)
   * and an epsilon value. It then captures the greyscale value of each pixel in
   * the Von Neumann neighborhood. Finally, it determines the difference between
   * each pixel in the Von Neumann neighborhood to the centre pixel and checks
   * whether the difference is less than epsilon and sets the value of the
   * centre pixel accordingly in order to mark the edges within the image.
   * The last double for loop at the end changes the pixels on the far right
   * border and the bottom-most border of the image to black.
   * The new image called 'edges' is then returned as output at the end of
   * the method.
   *
   * @param pic The input image
   * @param picName The image name (without the file extension)
   * @param epsilon Threshold required to mark the edges
   * @return Returns the edge detected image called 'edges'
   */
  public static Picture edgesMap(Picture pic, String picName, double epsilon) {
    noiseReduction(pic, picName, '2');

    int w = pic.width();
    int h = pic.height();
    Picture edges = new Picture(w, h);

    int right = 0;
    int left = 0;
    int up = 0;
    int down = 0;
    for (int y = 1; y < h; y++) {
      for (int x = 1; x < w; x++) {
        right = x + 1;
        left = x - 1;
        up = y - 1;
        down = y + 1;

        Color centrePixel = pic.get(x, y);
        Color rightPixel = null;
        Color leftPixel = null;
        Color upPixel = null;
        Color downPixel = null;

        int blueCentre = centrePixel.getBlue();
        int blueRight = 0;
        int blueLeft = 0;
        int blueUp = 0;
        int blueDown = 0;

        if (right < w) {
          rightPixel = pic.get(right, y);
          blueRight = rightPixel.getBlue();
        }

        if (left >= 0) {
          leftPixel = pic.get(left, y);
          blueLeft = leftPixel.getBlue();
        }

        if (up >= 0) {
          upPixel = pic.get(x, up);
          blueUp = upPixel.getBlue();
        }

        if (down < h) {
          downPixel = pic.get(x, down);
          blueDown = downPixel.getBlue();
        }
        if (Math.abs(blueCentre - blueRight) < epsilon
            && Math.abs(blueCentre - blueLeft) < epsilon
            && Math.abs(blueCentre - blueUp) < epsilon
            && Math.abs(blueCentre - blueDown) < epsilon) {
          Color newCentre = new Color(0, 0, 0);
          edges.set(x, y, newCentre);
        } else {
          Color newCentre = new Color(255, 255, 255);
          edges.set(x, y, newCentre);
        }
      }
    }
    for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
        if (j == w - 1) {
          edges.set(w - 1, i, new Color(0, 0, 0));
        }
        if (i == h - 1) {
          edges.set(j, h - 1, new Color(0, 0, 0));
        }
      }
    }
    return edges;
  }

  /**
   * Takes the edge detected image 'edges', the original pictures name
   * (without the file extension) and the upper and lower limits for the radius.
   * Creates a new picture called 'spots' with the same dimensions as the edges
   * image, loops from the lower radius limit to the upper radius limit, assigns
   * the corresponding width, delta and diff values. Runs the findSpots method
   * and the createSpotMask method within that to create a mask in order to
   * compare that to an image block of the same size over the edges image.
   * If the image block is similar enough to the mask then it gets added to the
   * spots image and the number of found spots increases.
   * Finally, at the end of the iterations the spots image gets saved to the
   * 'out' folder and the number of spots is printed to the terminal.
   *
   * @param edges Edges map (the edge detected image)
   * @param picName The image name (without the file extension)
   * @param lowerLimit Minimum radius value
   * @param upperLimit Maximum radius value
   */
  public static void spotDetection(Picture edges, String picName,
   int lowerLimit, int upperLimit) {
    Picture spots = new Picture(edges.width(), edges.height());
    int diff = 0;
    int width = 0;
    int delta = 0;
    for (int radius = lowerLimit; radius <= upperLimit; radius++) {
      if (radius == 4) {
        width = 6;
        delta = 0;
        diff = 4800;
      } else if (radius == 5) {
        width = 9;
        delta = 1;
        diff = 6625;
      } else if (radius == 6) {
        width = 12;
        delta = 1;
        diff = 11000;
      } else if (radius == 7) {
        width = 15;
        delta = 1;
        diff = 15000;
      } else if (radius == 8) {
        width = 18;
        delta = 1;
        diff = 19000;
      } else if (radius == 9) {
        width = 21;
        delta = 1;
        diff = 23000;
      } else if (radius == 10) {
        width = 24;
        delta = 2;
        diff = 28000;
      } else if (radius == 11) {
        width = 27;
        delta = 2;
        diff = 35000;
      }
      int[][] mask = createSpotMask(radius, width, delta);
      spots = findSpots(edges, spots, mask, radius, diff);
    }
    spots.save("../out/" + picName + "_SD.png");
    System.out.println(count);
  }
}
