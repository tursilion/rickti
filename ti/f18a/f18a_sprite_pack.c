/*
 * bmp_to_bitplanes.c
 *
 * Converts a Windows 256-color BMP file (only the first 8 palette entries
 * used) into three bitplane binary files plus a 12-bit RGB palette file.
 *
 * Source file format:
 *   Standard Windows BMP, 8 bits per pixel, with a 256-entry BGRA/BGRX
 *   color table.  Only palette indices 0..7 are used; pixel data is read
 *   via the offsets given in the BMP headers (not hard-coded), so this
 *   works regardless of header variant (BITMAPINFOHEADER etc.) or any
 *   palette size.
 *
 *   Image is 256x192 pixels, same object/tile grid as the TIAP tools:
 *   32 tiles across, 24 tiles down, each tile 8x8 pixels.
 *
 * Object structure (32x32 pixels = 4x4 tiles = 16 tiles):
 *   Same shuffle as tiap_to_bin.c -- each object's 16 tiles are reordered
 *   into sprite-engine order (TL/BL/TR/BR sprite, each holding TL/BL/TR/BR
 *   tile).  See TILE_ORDER[] below.
 *
 * Bitplane output:
 *   Each pixel's palette index (0..7) is 3 bits.  For each of the 3 bits,
 *   a separate bitplane file is produced:
 *     bit 0 -> bitplane file 1
 *     bit 1 -> bitplane file 2
 *     bit 2 -> bitplane file 3
 *   Each bitplane file has the same layout as tiap_to_bin.c's output:
 *   48 objects x 16 tiles x 8 bytes = 6144 bytes, tiles shuffled per
 *   object, MSB = leftmost pixel, no inversion.
 *
 * Palette output:
 *   8 entries x 2 bytes = 16 bytes.
 *   Each entry: 12-bit RGB (4 bits per channel, top 4 bits of each BMP
 *   8-bit channel), packed as 0000RRRRGGGGBBBB, stored as a 16-bit
 *   big-endian value.
 *
 * Build:
 *   gcc -o bmp_to_bitplanes bmp_to_bitplanes.c
 *
 * Usage:
 *   bmp_to_bitplanes <input.bmp> <plane0.bin> <plane1.bin> <plane2.bin> <palette.bin>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint8_t  U8;
typedef uint32_t U32;
typedef uint16_t U16;

/* -------------------------------------------------------------------------
 * Image / object layout constants (same grid as tiap_to_bin.c)
 * ------------------------------------------------------------------------- */
#define IMG_WIDTH        256
#define IMG_HEIGHT       192
#define TILE_BYTES       8        /* bytes per tile (one byte per pixel row) */
#define TILES_ACROSS     32
#define TILES_DOWN       24
#define TOTAL_TILES      (TILES_ACROSS * TILES_DOWN)    /* 768 */

#define OBJ_TILES_W      4
#define OBJ_TILES_H      4
#define OBJ_TILE_COUNT   (OBJ_TILES_W * OBJ_TILES_H)    /* 16 */
#define OBJ_BYTES        (OBJ_TILE_COUNT * TILE_BYTES)  /* 128 */
#define OBJS_ACROSS      8
#define OBJS_DOWN        6
#define TOTAL_OBJS       (OBJS_ACROSS * OBJS_DOWN)      /* 48 */
#define PLANE_BYTES      (TOTAL_OBJS * OBJ_BYTES)       /* 6144 */

#define PALETTE_COLORS   8
#define PALETTE_BYTES    (PALETTE_COLORS * 2)           /* 16 */

/*
 * Tile shuffle: (col_offset, row_offset) relative to the object's top-left
 * tile, in required output order. Same table as tiap_to_bin.c.
 */
static const int TILE_ORDER[OBJ_TILE_COUNT][2] = {
    /* TL sprite */
    {0, 0}, {0, 1}, {1, 0}, {1, 1},
    /* BL sprite */
    {0, 2}, {0, 3}, {1, 2}, {1, 3},
    /* TR sprite */
    {2, 0}, {2, 1}, {3, 0}, {3, 1},
    /* BR sprite */
    {2, 2}, {2, 3}, {3, 2}, {3, 3},
};

/* =========================================================================
 * read_u16_le() / read_u32_le()
 *
 * Read little-endian values from a byte buffer (BMP headers are LE).
 * ========================================================================= */
static U16 read_u16_le(const U8 *p)
{
    return (U16)(p[0] | (p[1] << 8));
}

static U32 read_u32_le(const U8 *p)
{
    return (U32)p[0] | ((U32)p[1] << 8) | ((U32)p[2] << 16) | ((U32)p[3] << 24);
}

/* =========================================================================
 * load_bmp_8bpp()
 *
 * Loads an 8-bit-per-pixel Windows BMP, returning a top-down array of
 * palette-index bytes (IMG_WIDTH x IMG_HEIGHT) and the first 8 palette
 * entries (as B,G,R byte triples from the BMP color table).
 *
 * Parameters:
 *   filename   - path to the .bmp file
 *   pixels     - output buffer, IMG_HEIGHT x IMG_WIDTH bytes (top-down,
 *                 row 0 = top of image), one palette index per pixel
 *   palette_bgr - output buffer, PALETTE_COLORS x 3 bytes (B,G,R per entry)
 *
 * Returns 0 on success, non-zero on error.
 *
 * Notes:
 *   - Reads width/height/bpp/data offset/palette offset directly from the
 *     BMP headers rather than assuming fixed offsets, since palette size
 *     (and therefore pixel data offset) can vary between files.
 *   - BMP color table entries are commonly 4 bytes (B,G,R,reserved) for
 *     BITMAPINFOHEADER; this is handled via the bV4/biSize-derived table
 *     entry size.
 *   - BMP pixel rows are stored bottom-up and may be padded to a 4-byte
 *     boundary; both are accounted for.
 *   - Requires height to be positive (bottom-up) and width/height to
 *     match IMG_WIDTH/IMG_HEIGHT.
 * ========================================================================= */
static int load_bmp_8bpp(const char *filename, U8 pixels[IMG_HEIGHT][IMG_WIDTH],
                          U8 palette_bgr[PALETTE_COLORS][3])
{
    FILE *fp;
    U8    file_hdr[14];
    U8    info_hdr_size_buf[4];
    U32   info_hdr_size;
    U8   *info_hdr;
    U32   data_offset;
    U32   width, height;
    U16   bpp;
    int   color_table_entry_size;
    long  color_table_offset;
    int   i, row, col;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "load_bmp_8bpp: cannot open '%s'\n", filename);
        return 1;
    }

    /* ------------------------------------------------------------------
     * BITMAPFILEHEADER (14 bytes)
     *   0: 'B' 'M'
     *   2: file size (u32)
     *   6: reserved (4 bytes)
     *  10: pixel data offset (u32)
     * ------------------------------------------------------------------ */
    if (fread(file_hdr, 1, 14, fp) != 14 || file_hdr[0] != 'B' || file_hdr[1] != 'M')
    {
        fprintf(stderr, "load_bmp_8bpp: not a BMP file\n");
        fclose(fp);
        return 1;
    }
    data_offset = read_u32_le(file_hdr + 10);

    /* ------------------------------------------------------------------
     * BITMAPINFOHEADER (variable size; first 4 bytes = header size)
     * ------------------------------------------------------------------ */
    if (fread(info_hdr_size_buf, 1, 4, fp) != 4)
    {
        fprintf(stderr, "load_bmp_8bpp: failed reading info header size\n");
        fclose(fp);
        return 1;
    }
    info_hdr_size = read_u32_le(info_hdr_size_buf);
    if (info_hdr_size < 40)
    {
        fprintf(stderr, "load_bmp_8bpp: unsupported info header size %u\n", info_hdr_size);
        fclose(fp);
        return 1;
    }

    info_hdr = (U8 *)malloc(info_hdr_size);
    if (!info_hdr)
    {
        fclose(fp);
        return 1;
    }
    memcpy(info_hdr, info_hdr_size_buf, 4);
    if (fread(info_hdr + 4, 1, info_hdr_size - 4, fp) != info_hdr_size - 4)
    {
        fprintf(stderr, "load_bmp_8bpp: failed reading info header body\n");
        free(info_hdr);
        fclose(fp);
        return 1;
    }

    /*
     * BITMAPINFOHEADER layout (offsets within info_hdr):
     *   4:  width  (i32)
     *   8:  height (i32, positive = bottom-up)
     *   12: planes (u16)
     *   14: bpp    (u16)
     */
    width  = read_u32_le(info_hdr + 4);
    height = read_u32_le(info_hdr + 8);
    bpp    = read_u16_le(info_hdr + 14);

    if (width != IMG_WIDTH || height != IMG_HEIGHT)
    {
        fprintf(stderr, "load_bmp_8bpp: expected %dx%d, got %ux%u\n",
                IMG_WIDTH, IMG_HEIGHT, width, height);
        free(info_hdr);
        fclose(fp);
        return 1;
    }
    if (bpp != 8)
    {
        fprintf(stderr, "load_bmp_8bpp: expected 8 bpp, got %u\n", bpp);
        free(info_hdr);
        fclose(fp);
        return 1;
    }

    /* ------------------------------------------------------------------
     * Color table immediately follows the info header.
     * Standard entry size is 4 bytes (B,G,R,reserved).
     * ------------------------------------------------------------------ */
    color_table_entry_size = 4;
    color_table_offset = 14 + (long)info_hdr_size;

    if (fseek(fp, color_table_offset, SEEK_SET) != 0)
    {
        fprintf(stderr, "load_bmp_8bpp: seek to color table failed\n");
        free(info_hdr);
        fclose(fp);
        return 1;
    }

    /* Read first PALETTE_COLORS (8) entries as B,G,R (drop the 4th byte) */
    for (i = 0; i < PALETTE_COLORS; i++)
    {
        U8 entry[4];
        if (fread(entry, 1, (size_t)color_table_entry_size, fp) != (size_t)color_table_entry_size)
        {
            fprintf(stderr, "load_bmp_8bpp: failed reading palette entry %d\n", i);
            free(info_hdr);
            fclose(fp);
            return 1;
        }
        palette_bgr[i][0] = entry[0]; /* B */
        palette_bgr[i][1] = entry[1]; /* G */
        palette_bgr[i][2] = entry[2]; /* R */
    }

    /* ------------------------------------------------------------------
     * Pixel data: 8bpp, bottom-up, rows padded to 4-byte boundary.
     * Row stride = ((width * 1) + 3) & ~3
     * ------------------------------------------------------------------ */
    {
        U32 row_stride = (width + 3u) & ~3u;
        U8 *row_buf = (U8 *)malloc(row_stride);
        if (!row_buf)
        {
            free(info_hdr);
            fclose(fp);
            return 1;
        }

        if (fseek(fp, (long)data_offset, SEEK_SET) != 0)
        {
            fprintf(stderr, "load_bmp_8bpp: seek to pixel data failed\n");
            free(row_buf);
            free(info_hdr);
            fclose(fp);
            return 1;
        }

        /* BMP row 0 (first in file) = bottom of image.
         * Read into pixels[] top-down by writing to row (height-1-r). */
        for (row = 0; row < (int)height; row++)
        {
            if (fread(row_buf, 1, row_stride, fp) != row_stride)
            {
                fprintf(stderr, "load_bmp_8bpp: failed reading pixel row %d\n", row);
                free(row_buf);
                free(info_hdr);
                fclose(fp);
                return 1;
            }

            int dst_row = (int)height - 1 - row; /* convert bottom-up -> top-down */
            for (col = 0; col < (int)width; col++)
                pixels[dst_row][col] = row_buf[col];
        }

        free(row_buf);
    }

    free(info_hdr);
    fclose(fp);
    return 0;
}

/* =========================================================================
 * get_tile_index_byte()
 *
 * Returns one byte (one pixel row, as 3-bit palette indices) for a tile,
 * built bit-by-bit isn't needed here -- this returns the row of palette
 * indices directly is not possible in one byte since indices are 3 bits.
 *
 * Instead: this helper extracts, for a given tile and pixel row, the
 * palette-index value (0..7) for each of the 8 pixels in that row.
 *
 * Parameters:
 *   pixels    - top-down IMG_HEIGHT x IMG_WIDTH palette-index array
 *   tile_col  - 0..31
 *   tile_row  - 0..23
 *   pixel_row - 0..7
 *   out_idx   - output array of 8 palette indices (0..7), left to right
 * ========================================================================= */
static void get_tile_row_indices(const U8 pixels[IMG_HEIGHT][IMG_WIDTH],
                                  int tile_col, int tile_row, int pixel_row,
                                  U8 out_idx[8])
{
    int base_x = tile_col * 8;
    int base_y = tile_row * 8 + pixel_row;
    int px;

    for (px = 0; px < 8; px++)
        out_idx[px] = pixels[base_y][base_x + px];
}

/* =========================================================================
 * convert_planes()
 *
 * Builds the three bitplane buffers from the source pixel array, applying
 * the same per-object tile shuffle as tiap_to_bin.c.
 *
 * For each object, each shuffled tile, each of the 8 pixel rows:
 *   For each of the 3 bit planes, pack 8 pixels' worth of that bit into
 *   one output byte (MSB = leftmost pixel), bit value taken directly from
 *   the palette index (no inversion).
 *
 * Parameters:
 *   pixels  - top-down IMG_HEIGHT x IMG_WIDTH palette-index array
 *   plane0  - output buffer, PLANE_BYTES bytes (bit 0 of each index)
 *   plane1  - output buffer, PLANE_BYTES bytes (bit 1 of each index)
 *   plane2  - output buffer, PLANE_BYTES bytes (bit 2 of each index)
 * ========================================================================= */
static void convert_planes(const U8 pixels[IMG_HEIGHT][IMG_WIDTH],
                            U8 *plane0, U8 *plane1, U8 *plane2)
{
    int obj, t, pr, px;
    int obj_col, obj_row;
    int src_tc0, src_tr0;

    for (obj = 0; obj < TOTAL_OBJS; obj++)
    {
        obj_col = obj % OBJS_ACROSS;
        obj_row = obj / OBJS_ACROSS;

        /* Top-left tile of this object in the source tilemap */
        src_tc0 = obj_col * OBJ_TILES_W;
        src_tr0 = obj_row * OBJ_TILES_H;

        for (t = 0; t < OBJ_TILE_COUNT; t++)
        {
            int tc = src_tc0 + TILE_ORDER[t][0];
            int tr = src_tr0 + TILE_ORDER[t][1];

            for (pr = 0; pr < 8; pr++)
            {
                U8 idx[8];
                U8 byte0 = 0, byte1 = 0, byte2 = 0;

                get_tile_row_indices(pixels, tc, tr, pr, idx);

                /* Pack 8 pixels into one byte per plane, MSB = leftmost */
                for (px = 0; px < 8; px++)
                {
                    int shift = 7 - px;
                    byte0 |= (U8)(((idx[px] >> 0) & 1) << shift);
                    byte1 |= (U8)(((idx[px] >> 1) & 1) << shift);
                    byte2 |= (U8)(((idx[px] >> 2) & 1) << shift);
                }

                {
                    int dst_offset = obj * OBJ_BYTES + t * TILE_BYTES + pr;
                    plane0[dst_offset] = byte0;
                    plane1[dst_offset] = byte1;
                    plane2[dst_offset] = byte2;
                }
            }
        }
    }
}

/* =========================================================================
 * build_palette()
 *
 * Converts 8 BMP palette entries (8-bit B,G,R) into 12-bit RGB values
 * packed as 0000RRRRGGGGBBBB, stored as 16-bit big-endian.
 *
 * Parameters:
 *   palette_bgr - PALETTE_COLORS x 3 input bytes (B,G,R), 8 bits each
 *   out         - output buffer, PALETTE_BYTES (16) bytes
 * ========================================================================= */
static void build_palette(const U8 palette_bgr[PALETTE_COLORS][3], U8 *out)
{
    int i;
    for (i = 0; i < PALETTE_COLORS; i++)
    {
        U8 b = palette_bgr[i][0];
        U8 g = palette_bgr[i][1];
        U8 r = palette_bgr[i][2];

        /* Take top 4 bits of each 8-bit channel */
        U16 r4 = (U16)(r >> 4);
        U16 g4 = (U16)(g >> 4);
        U16 b4 = (U16)(b >> 4);

        U16 val = (U16)((r4 << 8) | (g4 << 4) | b4);

        /* Store big-endian */
        out[i * 2 + 0] = (U8)((val >> 8) & 0xFF);
        out[i * 2 + 1] = (U8)(val & 0xFF);
    }
}

/* =========================================================================
 * write_file()
 *
 * Writes a buffer to disk, reporting errors.
 * ========================================================================= */
static int write_file(const char *filename, const U8 *data, size_t size)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "cannot open '%s' for writing\n", filename);
        return 1;
    }
    if (fwrite(data, 1, size, fp) != size)
    {
        fprintf(stderr, "short write to '%s'\n", filename);
        fclose(fp);
        return 1;
    }
    fclose(fp);
    return 0;
}

/* =========================================================================
 * main()
 *
 * Usage: bmp_to_bitplanes <input.bmp> <plane0> <plane1> <plane2> <palette>
 * ========================================================================= */
int main(int argc, char *argv[])
{
    U8 pixels[IMG_HEIGHT][IMG_WIDTH];
    U8 palette_bgr[PALETTE_COLORS][3];
    U8 plane0[PLANE_BYTES];
    U8 plane1[PLANE_BYTES];
    U8 plane2[PLANE_BYTES];
    U8 palette_out[PALETTE_BYTES];

    if (argc < 6)
    {
        fprintf(stderr,
            "Usage: %s <input.bmp> <plane0.bin> <plane1.bin> <plane2.bin> <palette.bin>\n",
            argv[0]);
        return 1;
    }

    if (load_bmp_8bpp(argv[1], pixels, palette_bgr) != 0)
        return 1;

    convert_planes(pixels, plane0, plane1, plane2);
    build_palette(palette_bgr, palette_out);

    if (write_file(argv[2], plane0, PLANE_BYTES) != 0) return 1;
    if (write_file(argv[3], plane1, PLANE_BYTES) != 0) return 1;
    if (write_file(argv[4], plane2, PLANE_BYTES) != 0) return 1;
    if (write_file(argv[5], palette_out, PALETTE_BYTES) != 0) return 1;

    printf("Wrote %s, %s, %s (%d bytes each), %s (%d bytes)\n",
           argv[2], argv[3], argv[4], PLANE_BYTES, argv[5], PALETTE_BYTES);
    return 0;
}
