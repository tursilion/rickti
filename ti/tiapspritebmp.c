/*
 * tiap_to_bmp.c
 *
 * Converts a TI bitmap (.TIAP) file into a 256x192 24-bit BMP showing
 * all sprite objects laid out in a grid.
 *
 * Source file format:
 *   Bytes 0..127   : Header -- ignored.
 *   Bytes 128..end : 768 tiles in row-major order (32 tiles across,
 *                    24 tiles down).  Each tile is 8 bytes; each byte
 *                    is one pixel row with MSB = leftmost pixel.
 *                    Bit 1 = black, bit 0 = white (inverted on output).
 *
 * Object structure (32x32 pixels = 4x4 tiles):
 *   Objects are assembled from four 16x16 sprites, each sprite from
 *   four 8x8 tiles.  The spatial arrangement in the source tilemap is:
 *
 *     TL-sprite occupies tile cols [oc+0..oc+1], tile rows [or+0..or+1]
 *     BL-sprite occupies tile cols [oc+0..oc+1], tile rows [or+2..or+3]
 *     TR-sprite occupies tile cols [oc+2..oc+3], tile rows [or+0..or+1]
 *     BR-sprite occupies tile cols [oc+2..oc+3], tile rows [or+2..or+3]
 *
 *   Within each sprite the tiles are ordered: TL, BL, TR, BR tile.
 *   This is the arrangement needed by the target sprite engine; in the
 *   source tilemap it maps to a contiguous 4x4 tile block, so we render
 *   each object by reading that block in normal raster order.
 *
 * Output BMP layout:
 *   8 objects across x 6 objects down = 48 objects per BMP.
 *   Each object cell is 32x32 pixels.  Total BMP: 256x192 pixels.
 *   Monochrome: bit 1 -> RGB(0,0,0) black; bit 0 -> RGB(255,255,255) white.
 *
 * Build:
 *   gcc -o tiap_to_bmp tiap_to_bmp.c
 *
 * Usage:
 *   tiap_to_bmp <input.TIAP> <output.bmp>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Type aliases
 * ------------------------------------------------------------------------- */
typedef uint8_t  U8;
typedef uint32_t U32;

/* -------------------------------------------------------------------------
 * Source file constants
 * ------------------------------------------------------------------------- */
#define HEADER_SIZE      128
#define TILE_BYTES       8        /* bytes per tile (one byte per pixel row) */
#define TILES_ACROSS     32       /* tiles per row in source image */
#define TILES_DOWN       24       /* tile rows in source image */
#define TOTAL_TILES      (TILES_ACROSS * TILES_DOWN)   /* 768 */
#define IMAGE_BYTES      (TOTAL_TILES * TILE_BYTES)    /* 6144 */
#define EXPECTED_FILESIZE (HEADER_SIZE + IMAGE_BYTES)  /* 6272 */

/* -------------------------------------------------------------------------
 * Object and output layout constants
 * ------------------------------------------------------------------------- */
#define OBJ_TILES_W      4        /* tiles wide per object */
#define OBJ_TILES_H      4        /* tiles tall per object */
#define OBJ_PIX_W        32       /* pixels wide per object */
#define OBJ_PIX_H        32       /* pixels tall per object */
#define OBJS_ACROSS      8        /* objects across in output BMP */
#define OBJS_DOWN        6        /* objects down in output BMP */
#define TOTAL_OBJS       (OBJS_ACROSS * OBJS_DOWN)    /* 48 */

/* -------------------------------------------------------------------------
 * BMP output constants (256x192 24-bit, same as other tools)
 * ------------------------------------------------------------------------- */
#define IMG_WIDTH        (OBJS_ACROSS * OBJ_PIX_W)    /* 256 */
#define IMG_HEIGHT       (OBJS_DOWN   * OBJ_PIX_H)    /* 192 */
#define BMP_FILE_HDR_SIZE  14
#define BMP_INFO_HDR_SIZE  40
#define BMP_PIXEL_OFFSET   (BMP_FILE_HDR_SIZE + BMP_INFO_HDR_SIZE)
/* 256px * 3 bytes = 768 bytes/row; DWORD-aligned, no row padding needed */
#define BMP_ROW_STRIDE     (IMG_WIDTH * 3)
#define BMP_PIXEL_DATA_SIZE (BMP_ROW_STRIDE * IMG_HEIGHT)
#define BMP_FILE_SIZE      (BMP_PIXEL_OFFSET + BMP_PIXEL_DATA_SIZE)

/* =========================================================================
 * put_u16_le() -- write 16-bit little-endian value into byte buffer
 * ========================================================================= */
static void put_u16_le(U8 *buf, uint16_t val)
{
    buf[0] = (U8)(val & 0xFF);
    buf[1] = (U8)((val >> 8) & 0xFF);
}

/* =========================================================================
 * put_u32_le() -- write 32-bit little-endian value into byte buffer
 * ========================================================================= */
static void put_u32_le(U8 *buf, U32 val)
{
    buf[0] = (U8)(val & 0xFF);
    buf[1] = (U8)((val >>  8) & 0xFF);
    buf[2] = (U8)((val >> 16) & 0xFF);
    buf[3] = (U8)((val >> 24) & 0xFF);
}

/* =========================================================================
 * load_tiap()
 *
 * Reads the TIAP file into a caller-supplied 6144-byte buffer, skipping
 * the 128-byte header.
 *
 * Parameters:
 *   filename  - path to the .TIAP file
 *   buf       - output buffer, must be IMAGE_BYTES (6144) bytes
 *
 * Returns:
 *   0 on success, non-zero on error.
 * ========================================================================= */
static int load_tiap(const char *filename, U8 *buf)
{
    FILE   *fp;
    size_t  n;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "load_tiap: cannot open '%s'\n", filename);
        return 1;
    }

    /* Skip the 128-byte header -- no useful image data there */
    if (fseek(fp, HEADER_SIZE, SEEK_SET) != 0)
    {
        fprintf(stderr, "load_tiap: seek failed on '%s'\n", filename);
        fclose(fp);
        return 1;
    }

    n = fread(buf, 1, IMAGE_BYTES, fp);
    fclose(fp);

    if (n < IMAGE_BYTES)
    {
        fprintf(stderr, "load_tiap: expected %d bytes after header, got %zu\n",
                IMAGE_BYTES, n);
        /* Not fatal -- remaining bytes will be zero if caller zeroed the buffer */
    }

    return 0;
}

/* =========================================================================
 * get_tile_byte()
 *
 * Returns one byte (one pixel row) from a tile in the image buffer.
 *
 * Parameters:
 *   img       - 6144-byte image buffer from load_tiap()
 *   tile_col  - tile column, 0..31
 *   tile_row  - tile row,    0..23
 *   pixel_row - pixel row within tile, 0..7
 * ========================================================================= */
static U8 get_tile_byte(const U8 *img, int tile_col, int tile_row, int pixel_row)
{
    int tile_idx = tile_row * TILES_ACROSS + tile_col;
    return img[tile_idx * TILE_BYTES + pixel_row];
}

/* =========================================================================
 * write_bmp()
 *
 * Renders the decoded tile image into a 256x192 24-bit BMP showing 48
 * objects (8 across x 6 down) each 32x32 pixels.
 *
 * Colour mapping (source bit -> output RGB):
 *   1 (black in TI convention) -> RGB(  0,   0,   0)  black
 *   0 (white in TI convention) -> RGB(255, 255, 255)  white
 *
 * Parameters:
 *   filename  - output BMP file path
 *   img       - 6144-byte image buffer from load_tiap()
 *
 * Returns:
 *   0 on success, non-zero on error.
 * ========================================================================= */
static int write_bmp(const char *filename, const U8 *img)
{
    FILE *fp;
    U8   file_hdr[BMP_FILE_HDR_SIZE];
    U8   info_hdr[BMP_INFO_HDR_SIZE];
    /* Top-down pixel buffer; zero-init = black for any untouched cells */
    U8   pixels[IMG_HEIGHT][BMP_ROW_STRIDE];
    int  obj;         /* object index 0..47 */
    int  obj_col;     /* object column in BMP, 0..7 */
    int  obj_row;     /* object row in BMP,    0..5 */
    int  tc;          /* tile column within object, 0..3 */
    int  tr;          /* tile row within object,    0..3 */
    int  pr;          /* pixel row within tile, 0..7 */
    int  px;          /* pixel (bit) within tile row, 0..7 */
    int  y;           /* absolute BMP scanline */
    int  x;           /* absolute BMP pixel X */

    memset(pixels, 0, sizeof(pixels));

    /* ------------------------------------------------------------------
     * Rasterize each object cell
     * Each object maps to a 4x4 tile block in the source image:
     *   object (obj_col, obj_row) -> source tile origin
     *     tile_col_origin = obj_col * OBJ_TILES_W
     *     tile_row_origin = obj_row * OBJ_TILES_H
     * ------------------------------------------------------------------ */
    for (obj = 0; obj < TOTAL_OBJS; obj++)
    {
        obj_col = obj % OBJS_ACROSS;   /* 0..7 */
        obj_row = obj / OBJS_ACROSS;   /* 0..5 */

        /* Tile-space origin of this object in the source tilemap */
        int src_tc0 = obj_col * OBJ_TILES_W;
        int src_tr0 = obj_row * OBJ_TILES_H;

        /* Pixel-space origin of this object cell in the output BMP */
        int dst_x0 = obj_col * OBJ_PIX_W;
        int dst_y0 = obj_row * OBJ_PIX_H;

        /* Walk the 4x4 tile grid of this object in normal raster order */
        for (tr = 0; tr < OBJ_TILES_H; tr++)
        {
            for (tc = 0; tc < OBJ_TILES_W; tc++)
            {
                /* Unpack all 8 pixel rows of this tile */
                for (pr = 0; pr < 8; pr++)
                {
                    U8 byte = get_tile_byte(img,
                                            src_tc0 + tc,
                                            src_tr0 + tr,
                                            pr);

                    y = dst_y0 + tr * 8 + pr;

                    /* Unpack 8 pixels from MSB to LSB */
                    for (px = 0; px < 8; px++)
                    {
                        /*
                         * MSB is leftmost pixel (px=0 -> bit 7).
                         * Bit=1 -> black (0x00), bit=0 -> white (0xFF).
                         * Invert with XOR 1 before expanding to 0x00/0xFF.
                         */
                        int bit = (byte >> (7 - px)) & 1;
                        U8  col = (bit ^ 1) ? 0xFF : 0x00;  /* invert: 1->black, 0->white */

                        x   = dst_x0 + tc * 8 + px;

                        /* Write BGR triplet (all channels equal for monochrome) */
                        int off = x * 3;
                        pixels[y][off + 0] = col;  /* B */
                        pixels[y][off + 1] = col;  /* G */
                        pixels[y][off + 2] = col;  /* R */
                    }
                }
            }
        }
    }

    /* ------------------------------------------------------------------
     * Build BMP file header (14 bytes)
     * ------------------------------------------------------------------ */
    memset(file_hdr, 0, sizeof(file_hdr));
    file_hdr[0] = 'B';
    file_hdr[1] = 'M';
    put_u32_le(file_hdr +  2, (U32)BMP_FILE_SIZE);
    put_u32_le(file_hdr + 10, (U32)BMP_PIXEL_OFFSET);

    /* ------------------------------------------------------------------
     * Build BITMAPINFOHEADER (40 bytes)
     * ------------------------------------------------------------------ */
    memset(info_hdr, 0, sizeof(info_hdr));
    put_u32_le(info_hdr +  0, 40);
    put_u32_le(info_hdr +  4, (U32)IMG_WIDTH);
    put_u32_le(info_hdr +  8, (U32)IMG_HEIGHT);
    put_u16_le(info_hdr + 12, 1);    /* color planes */
    put_u16_le(info_hdr + 14, 24);   /* bits per pixel */
    put_u32_le(info_hdr + 16, 0);    /* BI_RGB */
    put_u32_le(info_hdr + 20, (U32)BMP_PIXEL_DATA_SIZE);

    /* ------------------------------------------------------------------
     * Write file
     * ------------------------------------------------------------------ */
    fp = fopen(filename, "wb");
    if (!fp)
    {
        fprintf(stderr, "write_bmp: cannot open '%s'\n", filename);
        return 1;
    }

    if (fwrite(file_hdr, 1, BMP_FILE_HDR_SIZE, fp) != BMP_FILE_HDR_SIZE ||
        fwrite(info_hdr, 1, BMP_INFO_HDR_SIZE, fp) != BMP_INFO_HDR_SIZE)
    {
        fprintf(stderr, "write_bmp: failed writing headers\n");
        fclose(fp);
        return 1;
    }

    /* BMP rows are stored bottom-up */
    {
        int row;
        for (row = IMG_HEIGHT - 1; row >= 0; row--)
        {
            if (fwrite(pixels[row], 1, BMP_ROW_STRIDE, fp) != (size_t)BMP_ROW_STRIDE)
            {
                fprintf(stderr, "write_bmp: failed writing row %d\n", row);
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}

/* =========================================================================
 * main()
 * ========================================================================= */
int main(int argc, char *argv[])
{
    U8 img[IMAGE_BYTES];

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <input.TIAP> <output.bmp>\n", argv[0]);
        return 1;
    }

    memset(img, 0, sizeof(img));

    if (load_tiap(argv[1], img) != 0)
        return 1;

    if (write_bmp(argv[2], img) != 0)
        return 1;

    printf("Wrote %s (%dx%d, 48 objects, 24-bit BMP)\n",
           argv[2], IMG_WIDTH, IMG_HEIGHT);
    return 0;
}
