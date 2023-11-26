/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec: 114137  Name: Diogo Fernandes
// NMec: 113736  Name: Raquel Vinagre
//
//
// Date:
//

#include "image8bit.h"

#include "instrumentation.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
//
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8 *pixel; // pixel data (a raster scan)
};

// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
//
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char *errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char *ImageErrMsg() { ///
  return errCause;
}

// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success =
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
//
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
//
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
//
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)

// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char *failmsg) {
  errCause = (char *)(condition ? "" : failmsg);
  return condition;
}

/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem"; // InstrCount[0] will count pixel array acesses
  // Name other counters here...
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...

// TIP: Search for PIXMEM or InstrCount to see where it is incremented!

/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert(width >= 0);
  assert(height >= 0);
  assert(0 < maxval && maxval <= PixMax);
  // Written by us

  Image img = NULL; // Define uma variável do tipo Image
  int success = // Verifica se a criação da imagem foi bem sucedida (1) ou não (0)
      // Alocação de memória para a imagem e para o array de pixeis
      check((img = (Image)malloc(sizeof(struct image))) != NULL, "Allocation failed") &&
      check((img->pixel = (uint8 *)calloc(width * height, sizeof(uint8))) != NULL,"Allocation failed");

  // Alocar o conteúdo
  img->width = width;
  img->height = height;
  img->maxval = maxval;

  // Cleanup caso a criação da imagem não tenha sido bem sucedida
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  return img;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image *imgp) { ///
  assert(imgp != NULL);
  // Written by us
  if (*imgp != NULL) {    // Verifica se a imagem existe
    free((*imgp)->pixel); // Liberta a memória alocada para o array de pixeis
    free(*imgp);          // Liberta a memória alocada para a imagem
    *imgp = NULL;         // Define o endereço da imagem como NULL
  }
}

/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE *f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char *filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE *f = NULL;
  Image img = NULL;

  int success =
      check((f = fopen(filename, "rb")) != NULL, "Open failed") &&
      // Parse PGM header
      check(fscanf(f, "P%c ", &c) == 1 && c == '5', "Invalid file format") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &h) == 1 && h >= 0, "Invalid height") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d", &maxval) == 1 && 0 < maxval &&
                maxval <= (int)PixMax,
            "Invalid maxval") &&
      check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected") &&
      // Allocate image
      (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
      // Read pixels
      check(fread(img->pixel, sizeof(uint8), w * h, f) == w * h,
            "Reading pixels");
  PIXMEM += (unsigned long)(w * h); // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL)
    fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char *filename) { ///
  assert(img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE *f = NULL;

  int success = check((f = fopen(filename, "wb")) != NULL, "Open failed") &&
                check(fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0,
                      "Writing header failed") &&
                check(fwrite(img->pixel, sizeof(uint8), w * h, f) == w * h,
                      "Writing pixels failed");
  PIXMEM += (unsigned long)(w * h); // count pixel memory accesses

  // Cleanup
  if (f != NULL)
    fclose(f);
  return success;
}

/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert(img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert(img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert(img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8 *min, uint8 *max) { ///
  assert(img != NULL);
  // Written by us
  //  Tamanho do array
  int size = img->width * img->height;

  // Inicializar min e max
  *min = img->pixel[0];
  *max = *min;
  PIXMEM += 1;

  // Percorrer o array para dar Update caso necessário
  for (int i = 0; i < size; i++) {
    uint8 pixel_val = img->pixel[i];
    if (pixel_val < *min) {
      *min = pixel_val;
      PIXMEM += 1;
    } // Update no min
    if (pixel_val > *max) {
      *max = pixel_val;
      PIXMEM += 1;
    } // Update no max
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert(img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  assert(img != NULL);
  // written by us
  assert(w >= 0 && h >= 0);
  assert(x >= 0 && y >= 0);
  // (x,y) -> canto superior esquerdo
  // (x+w,y+h) -> canto inferior direito do retangulo
  return (x + w <= img->width &&
          y + h <= img->height); //  1 -> está dentro da imagem, 0 -> não está
}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel.
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y) {
  int index;
  // Written by us
  //  Transformar para um index linear
  index = y * img->width + x; // Transformar (33,0) -> [33] e (22,1) -> [122]
  assert(0 <= index && index < img->width * img->height);
  return index;
}

/// Get the pixel (level) at position (x,y).
uint8 ImageGetPixel(Image img, int x, int y) { ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1; // count one pixel access (read)
  return img->pixel[G(img, x, y)];
}

/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level) { ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1; // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
}

/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.

/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) { ///
  assert(img != NULL);
  // Written by us
  int size = img->width * img->height;
  uint8 maxval = img->maxval;
  // Percorrer o array de pixeis e aplicar a transformação
  for (int i = 0; i < size; i++) {
    PIXMEM += 1;
    img->pixel[i] = maxval - img->pixel[i]; // Aplicar a transformação
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) { ///
  assert(img != NULL);
  // Written by us
  int size = img->width * img->height;
  for (int i = 0; i < size; i++) {
    PIXMEM += 1;
    if (img->pixel[i] < thr)
      img->pixel[i] = 0; // Se < thr -> preto
    else
      img->pixel[i] = img->maxval; // Se >= thr -> branco
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) { ///
  assert(img != NULL);
  assert(factor >= 0.0);
  // Written by us
  int size = img->width * img->height;
  uint8 maxval = img->maxval;
  // Percorrer o array de pixeis e aplicar a transformação
  for (int i = 0; i < size; i++) {
    double new_pixel = img->pixel[i] * factor; // multiplicar pelo fator
    PIXMEM += 1;
    if (new_pixel > maxval) {
      // Saturar
      img->pixel[i] = maxval;
      PIXMEM += 1; 
    } else {
      // Não saturar (0.5 é para arredondar)
      img->pixel[i] = (int)(new_pixel + 0.5);
      PIXMEM += 1; 
    }
  }
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint:
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img) { ///
  assert(img != NULL);
  // Written by us
  // Criação da nova imagem
  Image img_rotated = ImageCreate(img->height, img->width, img->maxval);
  // Percorrer linhas & colunas
  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height;
         y++) { // aqui não é preciso o Pixmem++, pois já é contado nas funções
                // chamadas
      uint8 pixel = ImageGetPixel(img, x, y);
      ImageSetPixel(img_rotated, y, img->width - x - 1,
                    pixel); // 90 graus anti-clockwise
    }
  }

  return img_rotated;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) { ///
  assert(img != NULL);
  // written by us
  // Criação da nova imagem
  Image img_mirrored = ImageCreate(img->width, img->height, img->maxval);
  for (int x = 0; x < img->width; x++) {
    for (int y = 0; y < img->height; y++) {
      uint8 pixel = ImageGetPixel(img, x, y);
      ImageSetPixel(img_mirrored, img->width - x - 1, y,
                    pixel); // Mirror/Flip left-right
    }
  }
  return img_mirrored;
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) { ///
  assert(img != NULL);
  assert(ImageValidRect(img, x, y, w, h));
  // Written by us
  // x,y,w,h já estão asserted no ImageValidRect
  Image img_cropped = ImageCreate(w, h, img->maxval);

  for (int x_cord = x; x_cord < x + w; x_cord++) {
    for (int y_cord = y; y_cord < y + h; y_cord++) {
      uint8 pixel = ImageGetPixel(img, x_cord, y_cord);
      ImageSetPixel(img_cropped, x_cord - x, y_cord - y, pixel);
    }
  }
  return img_cropped;
}

/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) { ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));
  // Written by us
  for (int x_cord = x; x_cord < x + img2->width; x_cord++) {
    for (int y_cord = y; y_cord < y + img2->height; y_cord++) {
      uint8 pixel = ImageGetPixel(img2, x_cord - x, y_cord - y);
      ImageSetPixel(img1, x_cord, y_cord, pixel);
    }
  }
}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) {
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));
  // Written by us
  for (int x_cord = x; x_cord < x + img2->width; x_cord++) {
    for (int y_cord = y; y_cord < y + img2->height; y_cord++) {
      uint8 pixel1 = ImageGetPixel(img1, x_cord, y_cord);
      uint8 pixel2 = ImageGetPixel(img2, x_cord - x, y_cord - y);
      double new_pixel =
          (int)(pixel1 * (1 - alpha) + pixel2 * alpha + 0.5); // Arredondar
      if (new_pixel > img1->maxval) {
        new_pixel = img1->maxval;
        PIXMEM += 1; // Saturar
      }
      if (new_pixel < 0)
        new_pixel = 0; // Saturar
      ImageSetPixel(img1, x_cord, y_cord, new_pixel);
    }
  }
}

/* 1ª Abordagem - Sem memcmp
/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) {
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidPos(img1, x, y));
  // written by us
  // x,y já estão asserted no ImageValidPos

  for (int x_cord = x; x_cord < x + img2->width; x_cord++) {
    for (int y_cord = y; y_cord < y + img2->height; y_cord++) {
      uint8 pixel1 = ImageGetPixel(img1, x_cord, y_cord);
      uint8 pixel2 = ImageGetPixel(img2, x_cord - x, y_cord - y);
      if (pixel1 != pixel2)
        return 0; // Para não ser necessário percorrer o loop todo
    }
  }
  return 1; // caso sejam todos iguais
}
*/

int ImageMatchSubImage(Image img1, int x, int y, Image img2) {
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidPos(img1, x, y));

  for (int y_cord = 0; y_cord < img2->height; y_cord++) {
    // Use memcmp to compare entire rows at once
    // Use G to get the index
    PIXMEM+=img2->width;
    if (memcmp(&img1->pixel[G(img1, x, y + y_cord)], &img2->pixel[G(img2, 0, y_cord)], img2->width) != 0) {
      return 0; // Rows are not equal
    }
  }
  return 1; // All rows are equal
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px,
/// *py). If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int *px, int *py, Image img2) { ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  // Written by us
  int x_space = img1->width-img2->width; //the space left between the two images in the x axis
  int y_space = img1->height-img2->height; //the space left between the two images in the y axis
  for (int x = 0; x < x_space; x++) {
    for (int y = 0; y < y_space; y++) {
      if (ImageMatchSubImage(img1, x, y, img2)) {
        *px = x;
        *py = y;
        return 1;
      }
    }
  }
  return 0;
}

/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.


void ImageBlur(Image img, int dx, int dy) {
  assert(img != NULL);
  assert(dx >= 0 && dy >= 0);

  // Image dimensions
  int w = img->width;
  int h = img->height;
  int x, y;

  // Dimensions of the summed table
  const int sum_w = w + 2 * dx; // this is so there are enough pixels to the left and right
  const int sum_h = h + 2 * dy; // this is so there are enough pixels to the left and above

  // Allocate memory for the summed Table
  int *sumTable = (int *)malloc(sum_h * sum_w * sizeof(int)); 
 
  // Calculate the area of the filter kernel
  const int area = (2 * dx + 1) * (2 * dy + 1); // 2* because of the left and right side and +1 because of the center pixel

  // Computing the summed Table
  for (y = 0; y < h + 2 * dy; y++) {
    for (x = 0; x < w + 2 * dx; x++) {
      // Coordinates inside the original image
      const int x_dentro = x < dx ? 0 : (x - dx >= w ? w - 1 : x - dx); // 2 comparisons 
      const int y_dentro = y < dy ? 0 : (y - dy >= h ? h - 1 : y - dy); // 2 comparisons

      // Pixel value at the calculated coordinates
      int pixelVal = ImageGetPixel(img, x_dentro, y_dentro);
    
      // Adding the values from left and above,subrtract the overlapping corner
      pixelVal += x > 0 ? sumTable[y * sum_w + (x - 1)] : 0; // 1 comparison
      pixelVal += y > 0 ? sumTable[(y - 1) * sum_w + x] : 0; // 1 comparison
      pixelVal -= x > 0 && y > 0 ? sumTable[(y - 1) * sum_w + (x - 1)] : 0; // 2 comparisons
      
      // Storing the cumulative sum in the summed Table
      sumTable[y * sum_w + x] = pixelVal;
    }
  }

  // Applying the box filter in each pixel
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      // Defining the coordinates of the filter window
      int x1 = x;
      int y1 = y;
      int x2 = x + 2 * dx;
      int y2 = y + 2 * dy;

      // Doing the calculations in the summed table

      // Start in bottom right corner of the sum table
      int sum = sumTable[y2 * sum_w + x2];

      // Remove the bottom left corner of the sum table
      sum -= x1 > 0 ? sumTable[y2 * sum_w + (x1 - 1)] : 0; // 1 comparison

      // Remove the top right corner of the sum table
      sum -= y1 > 0 ? sumTable[(y1 - 1) * sum_w + x2] : 0; // 1 comparison

      // Add the top left corner of the sum table.
      // this gives us the sum at (x, y) considering the
      // filter kernel size of (dx, dy).
      sum += x1 > 0 && y1 > 0 ? sumTable[(y1 - 1) * sum_w + (x1 - 1)] : 0; // 2 comparisons
      
      // (area >> 1) is the same as (area / 2) but faster and avoiding floating point arithmetic
      // Setting the blurred pixel back into the original image
      ImageSetPixel(img, x, y, (uint8)((sum + (area >> 1)) / area)); 
    }
  }

  // Free allocated memory
  free(sumTable);
}



// 3ª Abordagem - Sem Clamping -> tenho que alterar algumas coisas
// void ImageBlur(Image img, int dx, int dy) {
//     assert(img != NULL);
//     assert(dx >= 0 && dy >= 0);

//     int w = img->width;
//     int h = img->height;
//     int x, y;

//     // Sum table horizontal
//    double *sumTable = (double *)malloc(h * w * sizeof(double));

// // Preenchendo a matriz de soma cumulativa
// for (y = 0; y < h; y++) {
//     for (x = 0; x < w; x++) {
//         double pixelVal = ImageGetPixel(img, x, y);
//         pixelVal += (x > 0 ? sumTable[y * w + (x - 1)] : 0);
//         pixelVal += (y > 0 ? sumTable[(y - 1) * w + x] : 0);
//         pixelVal -= (x > 0 && y > 0 ? sumTable[(y - 1) * w + (x - 1)] : 0);
//         sumTable[y * w + x] = pixelVal;
//     }
// }

// // Aplicar o desfoque, usando a matriz de soma cumulativa
// for (y = 0; y < h; y++) {
//     for (x = 0; x < w; x++) {
//         int x1 = (x - dx > 0) ? x - dx : 0;
//         int y1 = (y - dy > 0) ? y - dy : 0;
//         int x2 = (x + dx < w) ? x + dx : w - 1;
//         int y2 = (y + dy < h) ? y + dy : h - 1;

//         int area = (x2 - x1 + 1) * (y2 - y1 + 1);

//         int sum = sumTable[y2 * w + x2];
//         sum -= (x1 > 0 ? sumTable[y2 * w + x1 - 1] : 0);
//         sum -= (y1 > 0 ? sumTable[(y1 - 1) * w + x2] : 0);
//         sum += (x1 > 0 && y1 > 0 ? sumTable[(y1 - 1) * w + x1 - 1] : 0);

//       ImageSetPixel(img, x, y, (uint8)((sum + (area >> 1)) / area)); 
//     }
// }

// // Free allocated memory
// free(sumTable);
// }



// 1ª Abordagem - Versão simples
// void ImageBlur(Image img, int dx, int dy) {
//   assert(img != NULL);
//   assert(dx >= 0 && dy >= 0);
//   // Written by us
//   int width = img->width;
//   int height = img->height;
//   int size = width * height;

//   uint8 *blurredPixels = (uint8 *)malloc(size * sizeof(uint8));

//   uint8 *originalPixels = img->pixel;

//   for (int y = 0; y < height; y++) {
//     for (int x = 0; x < width; x++) {
//       int sum = 0;
//       int count = 0;
//       int newYLim = y + dy;
//       int newXLim = x + dx;

//       for (int newY= y-dy; newY <= newYLim; newY++) {
//         if (newY >= 0 && newY < height) {

//           for (int newX = x-dx; newX <= newXLim; newX++) {

//             if (newX >= 0 && newX < width) {
//               // int pixelIndex = newY * width + newX;
//               // sum += originalPixels[pixelIndex];
//               // PIXMEM += 1;
//               sum += ImageGetPixel(img,newX,newY);
//               count++;
//             }
//           }
//         }
//       }

//       int pixelIndex = y * width + x;
//       // count >> 1 is the same as count / 2 but faster and avoiding floating point arithmetic
//       *(blurredPixels + pixelIndex) = (uint8)((sum + (count >> 1)) / count);
//     }
//   }

//   memcpy(originalPixels, blurredPixels, size * sizeof(uint8)); 
//   PIXMEM+=size;
//   free(blurredPixels);
// }






