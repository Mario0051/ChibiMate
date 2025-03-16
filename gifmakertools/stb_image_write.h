/* stb_image_write - v1.16 - public domain - http://nothings.org/stb
   writes out PNG/BMP/TGA/JPEG/HDR images to C stdio - Sean Barrett 2010-2015
                                     no warranty implied; use at your own risk

   Before #including,

       #define STB_IMAGE_WRITE_IMPLEMENTATION

   in the file that you want to have the implementation.

   Will probably not work correctly with strict-aliasing optimizations.

ABOUT:

   This header file is a library for writing images to C stdio or a callback.

   The PNG output is not optimal; it is 20-50% larger than the file
   written by a decent optimizing implementation; though providing a custom
   zlib compress function (see STBIW_ZLIB_COMPRESS) can mitigate that.
   This library is designed for source code compactness and simplicity,
   not optimal image file size or run-time performance.

BUILDING:

   You can #define STBIW_ASSERT(x) before the #include to avoid using assert.h.
   You can #define STBIW_MALLOC(), STBIW_REALLOC(), and STBIW_FREE() to replace
   malloc,realloc,free.
   You can #define STBIW_MEMMOVE() to replace memmove()
   You can #define STBIW_ZLIB_COMPRESS to use a custom zlib-style compress function
   for PNG compression (instead of the builtin one), it must have the following signature:
   unsigned char * my_compress(unsigned char *data, int data_len, int *out_len, int quality);
   The returned data will be freed with STBIW_FREE() (free() by default),
   so it must be heap allocated with STBIW_MALLOC() (malloc() by default),
*/

#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#include <stdlib.h>

// if STB_IMAGE_WRITE_STATIC causes problems, try defining STBIWDEF to 'inline' or 'static inline'
#ifndef STBIWDEF
#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF  static
#else
#ifdef __cplusplus
#define STBIWDEF  extern "C"
#else
#define STBIWDEF  extern
#endif
#endif
#endif

#ifndef STB_IMAGE_WRITE_STATIC  // C++ forbids static forward declarations
STBIWDEF int stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);
#endif

#endif//INCLUDE_STB_IMAGE_WRITE_H

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef _WIN32
   #ifndef _CRT_SECURE_NO_WARNINGS
   #define _CRT_SECURE_NO_WARNINGS
   #endif
   #ifndef _CRT_NONSTDC_NO_DEPRECATE
   #define _CRT_NONSTDC_NO_DEPRECATE
   #endif
#endif

#ifndef STBI_WRITE_NO_STDIO
#include <stdio.h>
#endif // STBI_WRITE_NO_STDIO

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(STBIW_MALLOC) && defined(STBIW_FREE) && (defined(STBIW_REALLOC) || defined(STBIW_REALLOC_SIZED))
// ok
#elif !defined(STBIW_MALLOC) && !defined(STBIW_FREE) && !defined(STBIW_REALLOC) && !defined(STBIW_REALLOC_SIZED)
// ok
#else
#error "Must define all or none of STBIW_MALLOC, STBIW_FREE, and STBIW_REALLOC (or STBIW_REALLOC_SIZED)."
#endif

#ifndef STBIW_MALLOC
#define STBIW_MALLOC(sz)        malloc(sz)
#define STBIW_REALLOC(p,newsz)  realloc(p,newsz)
#define STBIW_FREE(p)           free(p)
#endif

#ifndef STBIW_REALLOC_SIZED
#define STBIW_REALLOC_SIZED(p,oldsz,newsz) STBIW_REALLOC(p,newsz)
#endif

#ifndef STBIW_MEMMOVE
#define STBIW_MEMMOVE(a,b,sz) memmove(a,b,sz)
#endif

#ifndef STBIW_ASSERT
#include <assert.h>
#define STBIW_ASSERT(x) assert(x)
#endif

typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

static void writefv(FILE *f, const char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = (unsigned char) va_arg(v, int); fputc(x,f); break; }
         case '2': { int x = va_arg(v,int); unsigned char b[2];
                     b[0] = (unsigned char) x; b[1] = (unsigned char) (x>>8);
                     fwrite(b,2,1,f); break; }
         case '4': { stbiw_uint32 x = va_arg(v,int); unsigned char b[4];
                     b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
                     b[2]=(unsigned char)(x>>16); b[3]=(unsigned char)(x>>24);
                     fwrite(b,4,1,f); break; }
         default:
            STBIW_ASSERT(0);
            return;
      }
   }
}

static void write3(FILE *f, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a; arr[1] = b; arr[2] = c;
   fwrite(arr, 3, 1, f);
}

static void write_pixels(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad)
{
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   stbiw_uint32 zero = 0;
   int i,j,k, j_end;

   if (y <= 0)
      return;

   if (vdir < 0)
      j_end = -1, j = y-1;
   else
      j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         if (write_alpha < 0)
            fwrite(&d[comp-1], 1, 1, f);
         switch (comp) {
            case 1:
            case 2: write3(f, d[0],d[0],d[0]);
                    break;
            case 4:
               if (!write_alpha) {
                  // composite against pink background
                  for (k=0; k < 3; ++k)
                     px[k] = bg[k] + ((d[k] - bg[k]) * d[3])/255;
                  write3(f, px[1-rgb_dir],px[1],px[1+rgb_dir]);
                  break;
               }
               /* FALLTHROUGH */
            case 3:
               write3(f, d[1-rgb_dir],d[1],d[1+rgb_dir]);
               break;
         }
         if (write_alpha > 0)
            fwrite(&d[comp-1], 1, 1, f);
      }
      fwrite(&zero,scanline_pad,1,f);
   }
}

static int outfile(char const *filename, int rgb_dir, int vdir, int x, int y, int comp, void *data, int alpha, int pad, const char *fmt, ...)
{
   FILE *f = fopen(filename, "wb");
   if (f) {
      va_list v;
      va_start(v, fmt);
      writefv(f, fmt, v);
      va_end(v);
      write_pixels(f,rgb_dir,vdir,x,y,comp,data,alpha,pad);
      fclose(f);
   }
   return f != NULL;
}

STBIWDEF int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes)
{
   int len;
   unsigned char *png = NULL;
   FILE *f;
   int ok;

   len = x * y * comp;
   png = (unsigned char *) STBIW_MALLOC(len);
   if (png == NULL) return 0;

   if (stride_bytes == 0)
      stride_bytes = x * comp;

   for (int j = 0; j < y; ++j) {
      unsigned char *src = (unsigned char *) data + j * stride_bytes;
      unsigned char *dest = png + j * x * comp;
      memcpy(dest, src, x * comp);
   }

   f = fopen(filename, "wb");
   if (!f) { STBIW_FREE(png); return 0; }

   ok = outfile(filename, 0, 1, x, y, comp, png, 0, 0,
         "11 PNG\r\n\x1a\n"
         "0000 IHDR %4 %4 8 %1 0 0 0",
         x, y, comp==4?6:2);

   STBIW_FREE(png);
   fclose(f);
   return ok;
}

#endif // STB_IMAGE_WRITE_IMPLEMENTATION 