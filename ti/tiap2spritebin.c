/*
 * tiap_to_bin.c
 *
 * Converts a TI bitmap (.TIAP) file into a raw binary blob of sprite objects.
 *
 * Source file format:
 *   Bytes 0..127   : Header -- ignored.
 *   Bytes 128..end : 768 tiles in row-major order (32 tiles across,
 *                    24 tiles down).  Each tile is 8 bytes; each byte is
 *                    one pixel row with MSB = leftmost pixel.
 *                    Bit 1 = black, bit 0 = white (inverted on output).
 *
 * Object structure (32x32 pixels = 4x4 tiles = 16 tiles = 128 bytes):
 *   Each object is assembled from its 4x4 tile block in the source tilemap
 *   and emitted in sprite-engine order.  The 16 tiles are grouped into
 *   four 16x16 sprites (TL, BL, TR, BR), each sprite holding four 8x8
 *   tiles (TL, BL, TR, BR).  Output tile sequence per object:
 *
 *   Sprite  Tile  Source offset (col, row) relative to object top-left
 *   ------  ----  -------------------------------------------------------
 *   TL      TL    (col+0, row+0)
 *   TL      BL    (col+0, row+1)
 *   TL      TR    (col+1, row+0)
 *   TL      BR    (col+1, row+1)
 *   BL      TL    (col+0, row+2)
 *   BL      BL    (col+0, row+3)
 *   BL      TR    (col+1, row+2)
 *   BL      BR    (col+1, row+3)
 *   TR      TL    (col+2, row+0)
 *   TR      BL    (col+2, row+1)
 *   TR      TR    (col+3, row+0)
 *   TR      BR    (col+3, row+1)
 *   BR      TL    (col+2, row+2)
 *   BR      BL    (col+2, row+3)
 *   BR      TR    (col+3, row+2)
 *   BR      BR    (col+3, row+3)
 *
 * Output binary:
 *   48 objects x 16 tiles x 8 bytes = 6144 bytes total.
 *   All bytes are bitwise-inverted from the source (source bit 1 -> output 0,
 *   source bit 0 -> output 1).
 *   Objects are emitted in raster order: left-to-right, top-to-bottom.
 *
 * Build:
 *   gcc -o tiap_to_bin tiap_to_bin.c
 *
 * Usage:
 *   tiap_to_bin <input.TIAP> <output.bin>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint8_t U8;

/* -------------------------------------------------------------------------
 * Source file constants
 * ------------------------------------------------------------------------- */
#define HEADER_SIZE      128
#define TILE_BYTES       8
#define TILES_ACROSS     32
#define TILES_DOWN       24
#define TOTAL_TILES      (TILES_ACROSS * TILES_DOWN)     /* 768 */
#define IMAGE_BYTES      (TOTAL_TILES * TILE_BYTES)      /* 6144 */

/* -------------------------------------------------------------------------
 * Object layout constants
 * ------------------------------------------------------------------------- */
#define OBJ_TILES_W      4       /* tiles wide per object */
#define OBJ_TILES_H      4       /* tiles tall per object */
#define OBJ_TILE_COUNT   (OBJ_TILES_W * OBJ_TILES_H)    /* 16 */
#define OBJ_BYTES        (OBJ_TILE_COUNT * TILE_BYTES)   /* 128 */
#define OBJS_ACROSS      8
#define OBJS_DOWN        6
#define TOTAL_OBJS       (OBJS_ACROSS * OBJS_DOWN)       /* 48 */
#define OUTPUT_BYTES     (TOTAL_OBJS * OBJ_BYTES)        /* 6144 */

/*
 * Tile shuffle: (col_offset, row_offset) relative to the object's top-left
 * tile, in the required output order.
 *
 * Order: TL-sprite(TL,BL,TR,BR), BL-sprite(TL,BL,TR,BR),
 *        TR-sprite(TL,BL,TR,BR), BR-sprite(TL,BL,TR,BR)
 */
static const int TILE_ORDER[OBJ_TILE_COUNT][2] = {
    /* TL sprite */
    {0, 0},  /* TL tile */
    {0, 1},  /* BL tile */
    {1, 0},  /* TR tile */
    {1, 1},  /* BR tile */
    /* BL sprite */
    {0, 2},  /* TL tile */
    {0, 3},  /* BL tile */
    {1, 2},  /* TR tile */
    {1, 3},  /* BR tile */
    /* TR sprite */
    {2, 0},  /* TL tile */
    {2, 1},  /* BL tile */
    {3, 0},  /* TR tile */
    {3, 1},  /* BR tile */
    /* BR sprite */
    {2, 2},  /* TL tile */
    {2, 3},  /* BL tile */
    {3, 2},  /* TR tile */
    {3, 3},  /* BR tile */
};

/* =========================================================================
 * load_tiap()
 *
 * Reads image bytes from a TIAP file into caller-supplied buffer,
 * skipping the 128-byte header.
 *
 * Parameters:
 *   filename  - path to .TIAP file
 *   buf       - output buffer, IMAGE_BYTES (6144) bytes
 *
 * Returns 0 on success, non-zero on error.
 * ========================================================================= */
static int load_tiap(const char *filename, U8 *buf)
{
    FILE  *fp;
    size_t n;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "load_tiap: cannot open '%s'\n", filename);
        return 1;
    }

    if (fseek(fp, HEADER_SIZE, SEEK_SET) != 0)
    {
        fprintf(stderr, "load_tiap: seek failed\n");
        fclose(fp);
        return 1;
    }

    n = fread(buf, 1, IMAGE_BYTES, fp);
    fclose(fp);

    if (n < IMAGE_BYTES)
    {
        fprintf(stderr, "load_tiap: short read: %zu of %d bytes\n", n, IMAGE_BYTES);
        /* Not fatal; caller zero-initialised the buffer */
    }

    return 0;
}

/* =========================================================================
 * get_tile()
 *
 * Returns a pointer to the 8 source bytes of a tile.
 *
 * Parameters:
 *   img       - IMAGE_BYTES source buffer
 *   tile_col  - 0..31
 *   tile_row  - 0..23
 * ========================================================================= */
static const U8 *get_tile(const U8 *img, int tile_col, int tile_row)
{
    int idx = tile_row * TILES_ACROSS + tile_col;
    return img + idx * TILE_BYTES;
}

/* =========================================================================
 * convert()
 *
 * Shuffles and inverts all 48 objects into the output buffer.
 *
 * For each object:
 *   1. Determine its tile origin in the source tilemap.
 *   2. Walk TILE_ORDER[] to select the 16 tiles in sprite-engine sequence.
 *   3. Copy each tile's 8 bytes, inverting every byte (~byte).
 *
 * Parameters:
 *   img  - IMAGE_BYTES source buffer from load_tiap()
 *   out  - OUTPUT_BYTES output buffer (caller-allocated)
 * ========================================================================= */
static void convert(const U8 *img, U8 *out)
{
    int obj;       /* 0..47 */
    int t;         /* tile slot within object, 0..15 */
    int b;         /* byte within tile, 0..7 */
    int obj_col;   /* object column, 0..7 */
    int obj_row;   /* object row,    0..5 */
    int src_tc0;   /* source tile-col origin of this object */
    int src_tr0;   /* source tile-row origin of this object */

    for (obj = 0; obj < TOTAL_OBJS; obj++)
    {
        obj_col = obj % OBJS_ACROSS;
        obj_row = obj / OBJS_ACROSS;

        /* Top-left tile of this object in the source tilemap */
        src_tc0 = obj_col * OBJ_TILES_W;
        src_tr0 = obj_row * OBJ_TILES_H;

        /* Emit 16 tiles in shuffle order */
        for (t = 0; t < OBJ_TILE_COUNT; t++)
        {
            /* TILE_ORDER gives (col_offset, row_offset) within the object */
            int tc = src_tc0 + TILE_ORDER[t][0];
            int tr = src_tr0 + TILE_ORDER[t][1];

            const U8 *src_tile = get_tile(img, tc, tr);

            /* Destination: object base + tile slot * 8 bytes */
            U8 *dst = out + obj * OBJ_BYTES + t * TILE_BYTES;

            /* Copy tile bytes with bitwise inversion:
             * source bit 1 (TI black) -> output 0
             * source bit 0 (TI white) -> output 1 */
            for (b = 0; b < TILE_BYTES; b++)
                dst[b] = (U8)(~src_tile[b]);
        }
    }
}

/* =========================================================================
 * main()
 * ========================================================================= */
int main(int argc, char *argv[])
{
    U8    img[IMAGE_BYTES];
    U8    out[OUTPUT_BYTES];
    FILE *fp;
    int   written;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <input.TIAP> <output.bin>\n", argv[0]);
        return 1;
    }

    memset(img, 0, sizeof(img));

    if (load_tiap(argv[1], img) != 0)
        return 1;

    convert(img, out);

    fp = fopen(argv[2], "wb");
    if (!fp)
    {
        fprintf(stderr, "cannot open '%s' for writing\n", argv[2]);
        return 1;
    }

    written = (int)fwrite(out, 1, OUTPUT_BYTES, fp);
    fclose(fp);

    if (written != OUTPUT_BYTES)
    {
        fprintf(stderr, "short write: %d of %d bytes\n", written, OUTPUT_BYTES);
        return 1;
    }

    printf("Wrote %s (%d objects x %d bytes = %d bytes)\n",
           argv[2], TOTAL_OBJS, OBJ_BYTES, OUTPUT_BYTES);
    return 0;
}
