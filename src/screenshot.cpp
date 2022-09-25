#include <xsystem/xsystem.h>
#if _WIN32
#include <stdio.h>
#include <stdlib.h>



#define FILE_TYPE_ERR "file type error", 1
#define FILE_OPEN_ERR "fopen: open file error", 2
#define FILE_READ_ERR "fread: read file error", 3
#define BUFFER_ALLOC_ERR "malloc: alloc buffer error", 4
#define BUFFER_READ_ERR "fread: read buffer error", 5
#define BUFFER_WRITE_ERR "fwrite: write buffer error", 6

#define REVERSED /* regularly, BMP image is stored reversely */

#define MEM_OUT_SIZE 1 << 17 /* alloc output memory with 128 KB */
#define BMP_HEAD_LEN 54      /* file head length of BMP image */
#define COMP_NUM 3           /* number of components */
#define PRECISION 8          /* data precision */
#define DCTSIZE 8            /* data unit size */
#define DCTSIZE2 64          /* data unit value number */
typedef enum
{ /* JPEG marker codes */
  M_SOF0 = 0xc0,
  M_SOF1 = 0xc1,
  M_SOF2 = 0xc2,
  M_SOF3 = 0xc3,

  M_SOF5 = 0xc5,
  M_SOF6 = 0xc6,
  M_SOF7 = 0xc7,

  M_JPG = 0xc8,
  M_SOF9 = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,

  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,

  M_DHT = 0xc4,

  M_DAC = 0xcc,

  M_RST0 = 0xd0,
  M_RST1 = 0xd1,
  M_RST2 = 0xd2,
  M_RST3 = 0xd3,
  M_RST4 = 0xd4,
  M_RST5 = 0xd5,
  M_RST6 = 0xd6,
  M_RST7 = 0xd7,

  M_SOI = 0xd8,
  M_EOI = 0xd9,
  M_SOS = 0xda,
  M_DQT = 0xdb,
  M_DNL = 0xdc,
  M_DRI = 0xdd,
  M_DHP = 0xde,
  M_EXP = 0xdf,

  M_APP0 = 0xe0,
  M_APP1 = 0xe1,
  M_APP2 = 0xe2,
  M_APP3 = 0xe3,
  M_APP4 = 0xe4,
  M_APP5 = 0xe5,
  M_APP6 = 0xe6,
  M_APP7 = 0xe7,
  M_APP8 = 0xe8,
  M_APP9 = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,

  M_JPG0 = 0xf0,
  M_JPG13 = 0xfd,
  M_COM = 0xfe,

  M_TEM = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;

typedef struct
{
    UINT8 len;
    UINT16 val;
} BITS;

/* zigzag table */

static UINT8 ZIGZAG[DCTSIZE2] = {
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63};

/* RGB to YCbCr table */

typedef struct
{
    INT32 r2y[256];
    INT32 r2cb[256];
    INT32 r2cr[256];
    INT32 g2y[256];
    INT32 g2cb[256];
    INT32 g2cr[256];
    INT32 b2y[256];
    INT32 b2cb[256];
    INT32 b2cr[256];
} ycbcr_tables;

extern ycbcr_tables ycc_tables;

/* store color unit in YCbCr */

typedef struct
{
    float y[DCTSIZE2];
    float cb[DCTSIZE2];
    float cr[DCTSIZE2];
} ycbcr_unit;

/* standard quantization tables */

static UINT8 STD_LU_QTABLE[DCTSIZE2] = {/* luminance */
                                        16, 11, 10, 16, 24, 40, 51, 61,
                                        12, 12, 14, 19, 26, 58, 60, 55,
                                        14, 13, 16, 24, 40, 57, 69, 56,
                                        14, 17, 22, 29, 51, 87, 80, 62,
                                        18, 22, 37, 56, 68, 109, 103, 77,
                                        24, 35, 55, 64, 81, 104, 113, 92,
                                        49, 64, 78, 87, 103, 121, 120, 101,
                                        72, 92, 95, 98, 112, 100, 103, 99};

static UINT8 STD_CH_QTABLE[DCTSIZE2] = {/* chrominance */
                                        17, 18, 24, 47, 99, 99, 99, 99,
                                        18, 21, 26, 66, 99, 99, 99, 99,
                                        24, 26, 56, 99, 99, 99, 99, 99,
                                        47, 66, 99, 99, 99, 99, 99, 99,
                                        99, 99, 99, 99, 99, 99, 99, 99,
                                        99, 99, 99, 99, 99, 99, 99, 99,
                                        99, 99, 99, 99, 99, 99, 99, 99,
                                        99, 99, 99, 99, 99, 99, 99, 99};

static double AAN_SCALE_FACTOR[DCTSIZE] = {
    1.0,
    1.387039845,
    1.306562965,
    1.175875602,
    1.0,
    0.785694958,
    0.541196100,
    0.275899379};

/* store scaled quantization tables */

typedef struct
{
    UINT8 lu[DCTSIZE2];
    UINT8 ch[DCTSIZE2];
} quant_tables;

extern quant_tables q_tables;

/* store color unit after quantizing operation */

typedef struct
{
    INT16 y[DCTSIZE2];
    INT16 cb[DCTSIZE2];
    INT16 cr[DCTSIZE2];
} quant_unit;

/* standard huffman tables */

/* luminance DC */

static UINT8 STD_LU_DC_NRCODES[17] = {/* code No. */
                                      0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};

static UINT8 STD_LU_DC_VALUES[12] = {/* code value */
                                     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

/* chrominance DC */

static UINT8 STD_CH_DC_NRCODES[17] = {
    0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};

static UINT8 STD_CH_DC_VALUES[12] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

/* luminance AC */

static UINT8 STD_LU_AC_NRCODES[17] = {
    0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d};

static UINT8 STD_LU_AC_VALUES[162] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa};

/* chrominance AC */

static UINT8 STD_CH_AC_NRCODES[17] = {
    0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77};

static UINT8 STD_CH_AC_VALUES[162] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa};

/* store precalculated huffman tables */

typedef struct
{
    BITS lu_dc[12];
    BITS lu_ac[256];
    BITS ch_dc[12];
    BITS ch_ac[256];
} huff_tables;

extern huff_tables h_tables;

/* store BMP image informations */

typedef struct
{
    UINT32 size;     /* bmp file size:                        2- 5 */
    UINT32 offset;   /* offset between file start and data:  10-13 */
    UINT32 width;    /* pixel width of bmp image:            18-21 */
    UINT32 height;   /* pixel height of bmp image:           22-25 */
    UINT16 bitppx;   /* bit number per pixel:                28-29 */
    UINT32 datasize; /* image data size:                     34-37 */
} bmp_info;

extern void err_exit(const char *error_string, int exit_num);
typedef unsigned char (*CIO_METHOD)(void *);

typedef struct
{
    UINT8 *set;
    UINT8 *pos;
    UINT8 *end;
    CIO_METHOD flush_buffer;
    FILE *fp;
} mem_mgr;

typedef struct
{
    mem_mgr *in;
    mem_mgr *out;
    BITS temp_bits;
} compress_io;

unsigned char flush_cin_buffer(void *cio);
unsigned char flush_cout_buffer(void *cio);

void init_mem(compress_io *cio,
              FILE *in_fp, int in_size, FILE *out_fp, int out_size);
void free_mem(compress_io *cio);

void write_byte(compress_io *cio, UINT8 val);
void write_word(compress_io *cio, UINT16 val);
void write_marker(compress_io *cio, JPEG_MARKER mark);
void write_bits(compress_io *cio, BITS bits);
void write_align_bits(compress_io *cio);
void write_app0(compress_io *cio)
{
    write_marker(cio, M_APP0);
    write_word(cio, 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1); /* length */
    write_byte(cio, 0x4A);                              /* Identifier: ASCII "JFIF" */
    write_byte(cio, 0x46);
    write_byte(cio, 0x49);
    write_byte(cio, 0x46);
    write_byte(cio, 0);
    write_byte(cio, 1); /* Version fields */
    write_byte(cio, 1);
    write_byte(cio, 0); /* Pixel size information */
    write_word(cio, 1);
    write_word(cio, 1);
    write_byte(cio, 0); /* No thumbnail image */
    write_byte(cio, 0);
}

void write_sof0(compress_io *cio, bmp_info *binfo)
{
    write_marker(cio, M_SOF0);
    write_word(cio, 3 * COMP_NUM + 2 + 5 + 1); /* length */
    write_byte(cio, PRECISION);

    write_word(cio, binfo->height);
    write_word(cio, binfo->width);

    write_byte(cio, COMP_NUM);

    /*
     * Component:
     *  Component ID
     *  Sampling factors:   bit 0-3 vert., 4-7 hor.
     *  Quantization table No.
     */
    /* component Y */
    write_byte(cio, 1);
    write_byte(cio, 0x11);
    write_byte(cio, 0);
    /* component Cb */
    write_byte(cio, 2);
    write_byte(cio, 0x11);
    write_byte(cio, 1);
    /* component Cr */
    write_byte(cio, 3);
    write_byte(cio, 0x11);
    write_byte(cio, 1);
}

void write_sos(compress_io *cio)
{
    write_marker(cio, M_SOS);
    write_word(cio, 2 + 1 + COMP_NUM * 2 + 3); /* length */

    write_byte(cio, COMP_NUM);

    /*
     * Component:
     *  Component ID
     *  DC & AC table No.   bits 0..3: AC table (0..3),
     *                      bits 4..7: DC table (0..3)
     */
    /* component Y */
    write_byte(cio, 1);
    write_byte(cio, 0x00);
    /* component Cb */
    write_byte(cio, 2);
    write_byte(cio, 0x11);
    /* component Cr */
    write_byte(cio, 3);
    write_byte(cio, 0x11);

    write_byte(cio, 0);    /* Ss */
    write_byte(cio, 0x3F); /* Se */
    write_byte(cio, 0);    /* Bf */
}

void write_dqt(compress_io *cio)
{
    /* index:
     *  bit 0..3: number of QT, Y = 0
     *  bit 4..7: precision of QT, 0 = 8 bit
     */
    int index;
    int i;
    write_marker(cio, M_DQT);
    write_word(cio, 2 + (DCTSIZE2 + 1) * 2);

    index = 0; /* table for Y */
    write_byte(cio, index);
    for (i = 0; i < DCTSIZE2; i++)
        write_byte(cio, q_tables.lu[i]);

    index = 1; /* table for Cb,Cr */
    write_byte(cio, index);
    for (i = 0; i < DCTSIZE2; i++)
        write_byte(cio, q_tables.ch[i]);
}

int get_ht_length(UINT8 *nrcodes)
{
    int length = 0;
    int i;
    for (i = 1; i <= 16; i++)
        length += nrcodes[i];
    return length;
}

void write_htable(compress_io *cio,
                  UINT8 *nrcodes, UINT8 *values, int len, UINT8 index)
{
    /*
     * index:
     *  bit 0..3: number of HT (0..3), for Y = 0
     *  bit 4   : type of HT, 0 = DC table, 1 = AC table
     *  bit 5..7: not used, must be 0
     */
    write_byte(cio, index);

    int i;
    for (i = 1; i <= 16; i++)
        write_byte(cio, nrcodes[i]);
    for (i = 0; i < len; i++)
        write_byte(cio, values[i]);
}

void write_dht(compress_io *cio)
{
    int len1, len2, len3, len4;

    write_marker(cio, M_DHT);

    len1 = get_ht_length(STD_LU_DC_NRCODES);
    len2 = get_ht_length(STD_LU_AC_NRCODES);
    len3 = get_ht_length(STD_CH_DC_NRCODES);
    len4 = get_ht_length(STD_CH_AC_NRCODES);
    write_word(cio, 2 + (1 + 16) * 4 + len1 + len2 + len3 + len4);

    write_htable(cio, STD_LU_DC_NRCODES, STD_LU_DC_VALUES, len1, 0x00);
    write_htable(cio, STD_LU_AC_NRCODES, STD_LU_AC_VALUES, len2, 0x10);
    write_htable(cio, STD_CH_DC_NRCODES, STD_CH_DC_VALUES, len3, 0x01);
    write_htable(cio, STD_CH_AC_NRCODES, STD_CH_AC_VALUES, len4, 0x11);
}

/*
 * Write datastream header.
 * This consists of an SOI and optional APPn markers.
 */
void write_file_header(compress_io *cio)
{
    write_marker(cio, M_SOI);
    write_app0(cio);
}

/*
 * Write frame header.
 * This consists of DQT and SOFn markers.
 * Note that we do not emit the SOF until we have emitted the DQT(s).
 * This avoids compatibility problems with incorrect implementations that
 * try to error-check the quant table numbers as soon as they see the SOF.
 */
void write_frame_header(compress_io *cio, bmp_info *binfo)
{
    write_dqt(cio);
    write_sof0(cio, binfo);
}

/*
 * Write scan header.
 * This consists of DHT or DAC markers, optional DRI, and SOS.
 * Compressed data will be written following the SOS.
 */
void write_scan_header(compress_io *cio)
{
    write_dht(cio);
    write_sos(cio);
}

/*
 * Write datastream trailer.
 */
void write_file_trailer(compress_io *cio)
{
    write_marker(cio, M_EOI);
}

void jpeg_fdct(float *data)
{
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13;
    float z1, z2, z3, z4, z5, z11, z13;
    float *dataptr;
    int ctr;

    /* Pass 1: process rows. */

    dataptr = data;
    for (ctr = 0; ctr < DCTSIZE; ctr++)
    {
        /* Load data into workspace */
        tmp0 = dataptr[0] + dataptr[7];
        tmp7 = dataptr[0] - dataptr[7];
        tmp1 = dataptr[1] + dataptr[6];
        tmp6 = dataptr[1] - dataptr[6];
        tmp2 = dataptr[2] + dataptr[5];
        tmp5 = dataptr[2] - dataptr[5];
        tmp3 = dataptr[3] + dataptr[4];
        tmp4 = dataptr[3] - dataptr[4];

        /* Even part */

        tmp10 = tmp0 + tmp3; /* phase 2 */
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        /* Apply unsigned->signed conversion */
        dataptr[0] = tmp10 + tmp11; /* phase 3 */
        dataptr[4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * ((float)0.707106781); /* c4 */
        dataptr[2] = tmp13 + z1;                     /* phase 5 */
        dataptr[6] = tmp13 - z1;

        /* Odd part */

        tmp10 = tmp4 + tmp5; /* phase 2 */
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        z5 = (tmp10 - tmp12) * ((float)0.382683433); /* c6 */
        z2 = ((float)0.541196100) * tmp10 + z5;      /* c2-c6 */
        z4 = ((float)1.306562965) * tmp12 + z5;      /* c2+c6 */
        z3 = tmp11 * ((float)0.707106781);           /* c4 */

        z11 = tmp7 + z3; /* phase 5 */
        z13 = tmp7 - z3;

        dataptr[5] = z13 + z2; /* phase 6 */
        dataptr[3] = z13 - z2;
        dataptr[1] = z11 + z4;
        dataptr[7] = z11 - z4;

        dataptr += DCTSIZE; /* advance pointer to next row */
    }

    /* Pass 2: process columns. */

    dataptr = data;
    for (ctr = DCTSIZE - 1; ctr >= 0; ctr--)
    {
        tmp0 = dataptr[DCTSIZE * 0] + dataptr[DCTSIZE * 7];
        tmp7 = dataptr[DCTSIZE * 0] - dataptr[DCTSIZE * 7];
        tmp1 = dataptr[DCTSIZE * 1] + dataptr[DCTSIZE * 6];
        tmp6 = dataptr[DCTSIZE * 1] - dataptr[DCTSIZE * 6];
        tmp2 = dataptr[DCTSIZE * 2] + dataptr[DCTSIZE * 5];
        tmp5 = dataptr[DCTSIZE * 2] - dataptr[DCTSIZE * 5];
        tmp3 = dataptr[DCTSIZE * 3] + dataptr[DCTSIZE * 4];
        tmp4 = dataptr[DCTSIZE * 3] - dataptr[DCTSIZE * 4];

        /* Even part */

        tmp10 = tmp0 + tmp3; /* phase 2 */
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        dataptr[DCTSIZE * 0] = tmp10 + tmp11; /* phase 3 */
        dataptr[DCTSIZE * 4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13) * ((float)0.707106781); /* c4 */
        dataptr[DCTSIZE * 2] = tmp13 + z1;           /* phase 5 */
        dataptr[DCTSIZE * 6] = tmp13 - z1;

        /* Odd part */

        tmp10 = tmp4 + tmp5; /* phase 2 */
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        /* The rotator is modified from fig 4-8 to avoid extra negations. */
        z5 = (tmp10 - tmp12) * ((float)0.382683433); /* c6 */
        z2 = ((float)0.541196100) * tmp10 + z5;      /* c2-c6 */
        z4 = ((float)1.306562965) * tmp12 + z5;      /* c2+c6 */
        z3 = tmp11 * ((float)0.707106781);           /* c4 */

        z11 = tmp7 + z3; /* phase 5 */
        z13 = tmp7 - z3;

        dataptr[DCTSIZE * 5] = z13 + z2; /* phase 6 */
        dataptr[DCTSIZE * 3] = z13 - z2;
        dataptr[DCTSIZE * 1] = z11 + z4;
        dataptr[DCTSIZE * 7] = z11 - z4;

        dataptr++; /* advance pointer to next column */
    }
}

UINT32
extract_uint(const UINT8 *dataptr, UINT32 start, UINT32 len)
{
    UINT32 uint = 0;
    if (len <= 0 || len > 4)
        return uint;
    if (len > 0)
        uint += dataptr[start];
    if (len > 1)
        uint += dataptr[start + 1] * 1 << 8;
    if (len > 2)
        uint += dataptr[start + 2] * 1 << 16;
    if (len > 3)
        uint += dataptr[start + 3] * 1 << 24;
    return uint;
}

int get_file_size(FILE *bmp_fp)
{
    fpos_t fpos;
    int len;
    fgetpos(bmp_fp, &fpos);
    fseek(bmp_fp, 0, SEEK_END);
    len = ftell(bmp_fp);
    fsetpos(bmp_fp, &fpos);
    return len;
}

void read_bmp(FILE *bmp_fp, bmp_info *binfo)
{
    size_t len = BMP_HEAD_LEN;
    UINT8 bmp_head[len];
    if (fread(bmp_head, sizeof(UINT8), len, bmp_fp) != len)
        err_exit(FILE_READ_ERR);

    binfo->size = extract_uint(bmp_head, 2, 4);
    binfo->offset = extract_uint(bmp_head, 10, 4);
    binfo->width = extract_uint(bmp_head, 18, 4);
    binfo->height = extract_uint(bmp_head, 22, 4);
    binfo->bitppx = extract_uint(bmp_head, 28, 2);
    binfo->datasize = extract_uint(bmp_head, 34, 4);
    if (binfo->datasize == 0) /* data size not included in some BMP */
        binfo->datasize = get_file_size(bmp_fp) - binfo->offset;
}

#include <string.h>
unsigned char flush_cin_buffer(void *cio)
{
    mem_mgr *in = ((compress_io *)cio)->in;
    size_t len = in->end - in->set;
    memset(in->set, 0, len);
    if (fread(in->set, sizeof(UINT8), len, in->fp) != len)
        return false;
#ifdef REVERSED
    fseek(in->fp, -len * 2, SEEK_CUR);
#endif
    in->pos = in->set;
    return true;
}
unsigned char flush_cout_buffer(void *cio)
{
    mem_mgr *out = ((compress_io *)cio)->out;
    size_t len = out->pos - out->set;
    if (fwrite(out->set, sizeof(UINT8), len, out->fp) != len)
        return false;
    memset(out->set, 0, len);
    out->pos = out->set;
    return true;
}

/*
 * init memory manager.
 */

void init_mem(compress_io *cio,
              FILE *in_fp, int in_size, FILE *out_fp, int out_size)
{
    cio->in = (mem_mgr *)malloc(sizeof(mem_mgr));
    if (!cio->in)
        err_exit(BUFFER_ALLOC_ERR);
    cio->in->set = (UINT8 *)malloc(sizeof(UINT8) * in_size);
    if (!cio->in->set)
        err_exit(BUFFER_ALLOC_ERR);
    cio->in->pos = cio->in->set;
    cio->in->end = cio->in->set + in_size;
    cio->in->flush_buffer = flush_cin_buffer;
    cio->in->fp = in_fp;

    cio->out = (mem_mgr *)malloc(sizeof(mem_mgr));
    if (!cio->out)
        err_exit(BUFFER_ALLOC_ERR);
    cio->out->set = (UINT8 *)malloc(sizeof(UINT8) * out_size);
    if (!cio->out->set)
        err_exit(BUFFER_ALLOC_ERR);
    cio->out->pos = cio->out->set;
    cio->out->end = cio->out->set + out_size;
    cio->out->flush_buffer = flush_cout_buffer;
    cio->out->fp = out_fp;

    cio->temp_bits.len = 0;
    cio->temp_bits.val = 0;
}

void free_mem(compress_io *cio)
{
    fflush(cio->out->fp);
    free(cio->in->set);
    free(cio->out->set);
    free(cio->in);
    free(cio->out);
}

/*
 * write operations.
 */

void write_byte(compress_io *cio, UINT8 val)
{
    mem_mgr *out = cio->out;
    *(out->pos)++ = val & 0xFF;
    if (out->pos == out->end)
    {
        if (!(out->flush_buffer)(cio))
            err_exit(BUFFER_WRITE_ERR);
    }
}

void write_word(compress_io *cio, UINT16 val)
{
    write_byte(cio, (val >> 8) & 0xFF);
    write_byte(cio, val & 0xFF);
}

void write_marker(compress_io *cio, JPEG_MARKER mark)
{
    write_byte(cio, 0xFF);
    write_byte(cio, (int)mark);
}

void write_bits(compress_io *cio, BITS bits)
{
    BITS *temp = &(cio->temp_bits);
    UINT16 word;
    UINT8 byte1, byte2;
    int len = bits.len + temp->len - 16;
    if (len >= 0)
    {
        word = temp->val | bits.val >> len;
        byte1 = word >> 8;
        write_byte(cio, byte1);
        if (byte1 == 0xFF)
            write_byte(cio, 0);
        byte2 = word & 0xFF;
        write_byte(cio, byte2);
        if (byte2 == 0xFF)
            write_byte(cio, 0);
        temp->len = len;
        temp->val = bits.val << (16 - len);
    }
    else
    {
        temp->len = 16 + len;
        temp->val |= bits.val << -len;
    }
}

void write_align_bits(compress_io *cio)
{
    BITS *temp = &(cio->temp_bits);
    BITS align_bits;
    align_bits.len = 8 - temp->len % 8;
    align_bits.val = (UINT16)~0x0 >> temp->len % 8;
    UINT8 byte;
    write_bits(cio, align_bits);
    if (temp->len == 8)
    {
        byte = temp->val >> 8;
        write_byte(cio, byte);
        if (byte == 0xFF)
            write_byte(cio, 0);
    }
}

/*
 * precalculated tables for a faster YCbCr->RGB transformation.
 * use a INT32 table because we'll scale values by 2^16 and
 * work with integers.
 */

ycbcr_tables ycc_tables;

void init_ycbcr_tables()
{
    UINT16 i;
    for (i = 0; i < 256; i++)
    {
        ycc_tables.r2y[i] = (INT32)(65536 * 0.299 + 0.5) * i;
        ycc_tables.r2cb[i] = (INT32)(65536 * -0.16874 + 0.5) * i;
        ycc_tables.r2cr[i] = (INT32)(32768) * i;
        ycc_tables.g2y[i] = (INT32)(65536 * 0.587 + 0.5) * i;
        ycc_tables.g2cb[i] = (INT32)(65536 * -0.33126 + 0.5) * i;
        ycc_tables.g2cr[i] = (INT32)(65536 * -0.41869 + 0.5) * i;
        ycc_tables.b2y[i] = (INT32)(65536 * 0.114 + 0.5) * i;
        ycc_tables.b2cb[i] = (INT32)(32768) * i;
        ycc_tables.b2cr[i] = (INT32)(65536 * -0.08131 + 0.5) * i;
    }
}

void rgb_to_ycbcr(UINT8 *rgb_unit, ycbcr_unit *ycc_unit, int x, int w)
{
    ycbcr_tables *tbl = &ycc_tables;
    UINT8 r, g, b;
#ifdef REVERSED
    int src_pos = (x + w * (DCTSIZE - 1)) * 3;
#else
    int src_pos = x * 3;
#endif
    int dst_pos = 0;
    int i, j;
    for (j = 0; j < DCTSIZE; j++)
    {
        for (i = 0; i < DCTSIZE; i++)
        {
            b = rgb_unit[src_pos];
            g = rgb_unit[src_pos + 1];
            r = rgb_unit[src_pos + 2];
            ycc_unit->y[dst_pos] = (INT8)((UINT8)((tbl->r2y[r] + tbl->g2y[g] + tbl->b2y[b]) >> 16) - 128);
            ycc_unit->cb[dst_pos] = (INT8)((UINT8)((tbl->r2cb[r] + tbl->g2cb[g] + tbl->b2cb[b]) >> 16));
            ycc_unit->cr[dst_pos] = (INT8)((UINT8)((tbl->r2cr[r] + tbl->g2cr[g] + tbl->b2cr[b]) >> 16));
            src_pos += 3;
            dst_pos++;
        }
#ifdef REVERSED
        src_pos -= (w + DCTSIZE) * 3;
#else
        src_pos += (w - DCTSIZE) * 3;
#endif
    }
}

/* quantization */

quant_tables q_tables;

void init_quant_tables(UINT32 scale_factor)
{
    quant_tables *tbl = &q_tables;
    int temp1, temp2;
    int i;
    for (i = 0; i < DCTSIZE2; i++)
    {
        temp1 = ((UINT32)STD_LU_QTABLE[i] * scale_factor + 50) / 100;
        if (temp1 < 1)
            temp1 = 1;
        if (temp1 > 255)
            temp1 = 255;
        tbl->lu[ZIGZAG[i]] = (UINT8)temp1;

        temp2 = ((UINT32)STD_CH_QTABLE[i] * scale_factor + 50) / 100;
        if (temp2 < 1)
            temp2 = 1;
        if (temp2 > 255)
            temp2 = 255;
        tbl->ch[ZIGZAG[i]] = (UINT8)temp2;
    }
}

void jpeg_quant(ycbcr_unit *ycc_unit, quant_unit *q_unit)
{
    quant_tables *tbl = &q_tables;
    float q_lu, q_ch;
    int x, y, i = 0;
    for (x = 0; x < DCTSIZE; x++)
    {
        for (y = 0; y < DCTSIZE; y++)
        {
            q_lu = 1.0 / ((double)tbl->lu[ZIGZAG[i]] *
                          AAN_SCALE_FACTOR[x] * AAN_SCALE_FACTOR[y] * 8.0);
            q_ch = 1.0 / ((double)tbl->ch[ZIGZAG[i]] *
                          AAN_SCALE_FACTOR[x] * AAN_SCALE_FACTOR[y] * 8.0);

            q_unit->y[i] = (INT16)(ycc_unit->y[i] * q_lu + 16384.5) - 16384;
            q_unit->cb[i] = (INT16)(ycc_unit->cb[i] * q_ch + 16384.5) - 16384;
            q_unit->cr[i] = (INT16)(ycc_unit->cr[i] * q_ch + 16384.5) - 16384;

            i++;
        }
    }
}

/* huffman compression */

huff_tables h_tables;

void set_huff_table(UINT8 *nrcodes, UINT8 *values, BITS *h_table)
{
    int i, j, k;
    j = 0;
    UINT16 value = 0;
    for (i = 1; i <= 16; i++)
    {
        for (k = 0; k < nrcodes[i]; k++)
        {
            h_table[values[j]].len = i;
            h_table[values[j]].val = value;
            j++;
            value++;
        }
        value <<= 1;
    }
}

void init_huff_tables()
{
    huff_tables *tbl = &h_tables;
    set_huff_table(STD_LU_DC_NRCODES, STD_LU_DC_VALUES, tbl->lu_dc);
    set_huff_table(STD_LU_AC_NRCODES, STD_LU_AC_VALUES, tbl->lu_ac);
    set_huff_table(STD_CH_DC_NRCODES, STD_CH_DC_VALUES, tbl->ch_dc);
    set_huff_table(STD_CH_AC_NRCODES, STD_CH_AC_VALUES, tbl->ch_ac);
}

void set_bits(BITS *bits, INT16 data)
{
    UINT16 pos_data;
    int i;
    pos_data = data < 0 ? ~data + 1 : data;
    for (i = 15; i >= 0; i--)
        if ((pos_data & (1 << i)) != 0)
            break;
    bits->len = i + 1;
    bits->val = data < 0 ? data + (1 << bits->len) - 1 : data;
}

#ifdef DEBUG
void print_bits(BITS bits)
{
    printf("%hu %hu\t", bits.len, bits.val);
}
#endif

/*
 * compress JPEG
 */
void jpeg_compress(compress_io *cio,
                   INT16 *data, INT16 *dc, BITS *dc_htable, BITS *ac_htable)
{
    INT16 zigzag_data[DCTSIZE2];
    BITS bits;
    INT16 diff;
    int i, j;
    int zero_num;
    int mark;

    /* zigzag encode */
    for (i = 0; i < DCTSIZE2; i++)
        zigzag_data[ZIGZAG[i]] = data[i];

    /* write DC */
    diff = zigzag_data[0] - *dc;
    *dc = zigzag_data[0];

    if (diff == 0)
        write_bits(cio, dc_htable[0]);
    else
    {
        set_bits(&bits, diff);
        write_bits(cio, dc_htable[bits.len]);
        write_bits(cio, bits);
    }

    /* write AC */
    int end = DCTSIZE2 - 1;
    while (zigzag_data[end] == 0 && end > 0)
        end--;
    for (i = 1; i <= end; i++)
    {
        j = i;
        while (zigzag_data[j] == 0 && j <= end)
            j++;
        zero_num = j - i;
        for (mark = 0; mark < zero_num / 16; mark++)
            write_bits(cio, ac_htable[0xF0]);
        zero_num = zero_num % 16;
        set_bits(&bits, zigzag_data[j]);
        write_bits(cio, ac_htable[zero_num * 16 + bits.len]);
        write_bits(cio, bits);
        i = j;
    }

    /* write end of unit */
    if (end != DCTSIZE2 - 1)
        write_bits(cio, ac_htable[0]);
}

/*
 * main JPEG encoding
 */
void jpeg_encode(compress_io *cio, bmp_info *binfo)
{
    /* init tables */
    UINT32 scale = 50;
    init_ycbcr_tables();
    init_quant_tables(scale);
    init_huff_tables();

    /* write info */
    write_file_header(cio);
    write_frame_header(cio, binfo);
    write_scan_header(cio);

    /* encode */
    mem_mgr *in = cio->in;
    ycbcr_unit ycc_unit;
    quant_unit q_unit;
    INT16 dc_y = 0,
          dc_cb = 0,
          dc_cr = 0;
    int x, y;

#ifdef REVERSED
    int in_size = in->end - in->set;
    int offset = binfo->offset + (binfo->datasize / in_size - 1) * in_size;
#else
    int offset = binfo->offset;
#endif
    fseek(in->fp, offset, SEEK_SET);

    for (y = 0; y < binfo->height; y += 8)
    {

        /* flush input buffer */
        if (!(in->flush_buffer)(cio))
            err_exit(BUFFER_READ_ERR);

        for (x = 0; x < binfo->width; x += 8)
        {

            /* convert RGB unit to YCbCr unit */
            rgb_to_ycbcr(in->set, &ycc_unit, x, binfo->width);

            /* forward DCT on YCbCr unit */
            jpeg_fdct(ycc_unit.y);
            jpeg_fdct(ycc_unit.cb);
            jpeg_fdct(ycc_unit.cr);

            /* quantization, store in quant unit */
            jpeg_quant(&ycc_unit, &q_unit);

            /* huffman compression, write */
            jpeg_compress(cio, q_unit.y, &dc_y,
                          h_tables.lu_dc, h_tables.lu_ac);
            jpeg_compress(cio, q_unit.cb, &dc_cb,
                          h_tables.ch_dc, h_tables.ch_ac);
            jpeg_compress(cio, q_unit.cr, &dc_cr,
                          h_tables.ch_dc, h_tables.ch_ac);
        }
    }
    write_align_bits(cio);

    /* write file end */
    write_file_trailer(cio);
}

unsigned char is_bmp(FILE *fp)
{
    UINT8 marker[3];
    if (fread(marker, sizeof(UINT16), 2, fp) != 2)
        err_exit(FILE_READ_ERR);
    if (marker[0] != 0x42 || marker[1] != 0x4D)
        return false;
    rewind(fp);
    return true;
}

void err_exit(const char *error_string, int exit_num)
{
    printf(error_string);
    exit(exit_num);
}

void bmp_to_jpeg(char *bmp_path, char *jpeg_path)
{

    /* open bmp file */
    FILE *bmp_fp = fopen(bmp_path, "rb");
    if (!bmp_fp)
        err_exit(FILE_OPEN_ERR);
    if (!is_bmp(bmp_fp))
        err_exit(FILE_TYPE_ERR);

    /* open jpeg file */
    FILE *jpeg_fp = fopen(jpeg_path, "wb");
    if (!jpeg_fp)
        err_exit(FILE_OPEN_ERR);

    /* get bmp info */
    bmp_info binfo;
    read_bmp(bmp_fp, &binfo);

    /* init memory for input and output */
    compress_io cio;
    int in_size = (binfo.width * 3 + 3) / 4 * 4 * DCTSIZE;
    int out_size = MEM_OUT_SIZE;
    init_mem(&cio, bmp_fp, in_size, jpeg_fp, out_size);

    /* main encode process */
    jpeg_encode(&cio, &binfo);

    /* flush and free memory, close files */
    if (!(cio.out->flush_buffer)(&cio))
    {
        err_exit(BUFFER_WRITE_ERR);
    }
    free_mem(&cio);
    fclose(bmp_fp);
    fclose(jpeg_fp);
}
#include <gdiplus.h>
/************************************************************************/
/* hBitmap    为刚才的屏幕位图句柄                                                                   
/* lpFileName 为需要保存的位图文件名
/************************************************************************/
double getZoom();
HBITMAP   GetCaptureBmp();
int SaveBitmapToFile(LPSTR lpFileName);
double getZoom()
{
    // 获取窗口当前显示的监视器
    HWND hWnd = GetDesktopWindow();
    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    // 获取监视器逻辑宽度
    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    GetMonitorInfo(hMonitor, &monitorInfo);
    int cxLogical = (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);

    // 获取监视器物理宽度
    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &dm);
    int cxPhysical = dm.dmPelsWidth;

    return cxPhysical * 1.0 / cxLogical;
}
int SaveBitmapToFile(LPSTR lpFileName) 
{         
    HBITMAP   hBitmap;    
    hBitmap   =   GetCaptureBmp(); 
    HDC            hDC; //设备描述表
    int            iBits;//当前显示分辨率下每个像素所占字节数
    WORD           wBitCount;//位图中每个像素所占字节数    
    DWORD          dwPaletteSize=0;//定义调色板大小
    DWORD          dwBmBitsSize;//位图中像素字节大小
    DWORD          dwDIBSize;// 位图文件大小
    DWORD          dwWritten;//写入文件字节数
    BITMAP         Bitmap;//位图结构
    BITMAPFILEHEADER   bmfHdr;   //位图属性结构   
    BITMAPINFOHEADER   bi;       //位图文件头结构
    LPBITMAPINFOHEADER lpbi;     //位图信息头结构     指向位图信息头结构
    HANDLE          fh;//定义文件句柄
    HANDLE            hDib;//分配内存句柄
    HANDLE            hPal;//分配内存句柄
    HANDLE          hOldPal=NULL;//调色板句柄  
     
    //计算位图文件每个像素所占字节数   
    hDC = CreateDC("DISPLAY",NULL,NULL,NULL);   
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);   
    DeleteDC(hDC);
     
    if (iBits <= 1)       
        wBitCount = 1;   
    else if (iBits <= 4)       
        wBitCount = 4;   
    else if (iBits <= 8)       
        wBitCount = 8;   
    else if (iBits <= 24)
        wBitCount = 24;
    else if (iBits<=32)
        wBitCount = 24;
     
     
    //计算调色板大小   
    if (wBitCount <= 8)       
        dwPaletteSize = (1 << wBitCount) *sizeof(RGBQUAD);
     
     
     
    //设置位图信息头结构   
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);   
    bi.biSize            = sizeof(BITMAPINFOHEADER);   
    bi.biWidth           = Bitmap.bmWidth;   
    bi.biHeight          = Bitmap.bmHeight;   
    bi.biPlanes          = 1;   
    bi.biBitCount         = wBitCount;       
    bi.biCompression      = BI_RGB;   
    bi.biSizeImage        = 0;   
    bi.biXPelsPerMeter     = 0;   
    bi.biYPelsPerMeter     = 0;   
    bi.biClrUsed         = 0;   
    bi.biClrImportant      = 0;   
    dwBmBitsSize = ((Bitmap.bmWidth *wBitCount+31)/32)* 4*Bitmap.bmHeight ;
     
    //为位图内容分配内存   
    hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));   
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (lpbi==NULL)
    {
        return 0;
    }
     
    *lpbi = bi;   
    // 处理调色板
    hPal = GetStockObject(DEFAULT_PALETTE);   
    if (hPal)
    {       
        hDC  = GetDC(NULL);       
        hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);       
        RealizePalette(hDC);       
    }   
    // 获取该调色板下新的像素值   
    GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight,       
        (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSize,
        (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);   
    //恢复调色板      
    if (hOldPal)       
    {       
        SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);       
        RealizePalette(hDC);       
        ReleaseDC(NULL, hDC);       
    }   
    //创建位图文件       
    fh = CreateFile(lpFileName, GENERIC_WRITE,        
        0, NULL, CREATE_ALWAYS,       
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);   
 
    if (fh == INVALID_HANDLE_VALUE)       
        return FALSE;
     
    // 设置位图文件头   
    bmfHdr.bfType = 0x4D42;  // "BM"   
    dwDIBSize    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)+ dwPaletteSize + dwBmBitsSize;     
    bmfHdr.bfSize = dwDIBSize;   
    bmfHdr.bfReserved1 = 0;   
    bmfHdr.bfReserved2 = 0;   
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER)+ dwPaletteSize;
     
    // 写入位图文件头   
    WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
     
    // 写入位图文件其余内容   
    WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
     
    //清除      
    GlobalUnlock(hDib);   
    GlobalFree(hDib);   
    CloseHandle(fh);
     
    return 1;
}
 
HBITMAP   GetCaptureBmp()   
{
    HDC     hDC;   
    HDC     MemDC;   
    BYTE*   Data;   
    HBITMAP   hBmp;   
    BITMAPINFO   bi;   
     
    memset(&bi,   0,   sizeof(bi));   
    double c= getZoom();
    bi.bmiHeader.biSize   =   sizeof(BITMAPINFO);
    bi.bmiHeader.biWidth   =  GetSystemMetrics(SM_CXSCREEN)*c;   
    bi.bmiHeader.biHeight   = GetSystemMetrics(SM_CYSCREEN)*c;   
    bi.bmiHeader.biPlanes   =   1;   
    bi.bmiHeader.biBitCount   =   24;   
     
    hDC   =   GetDC(NULL);   
    MemDC   =   CreateCompatibleDC(hDC);   
    hBmp   =   CreateDIBSection(MemDC,   &bi, DIB_RGB_COLORS,   (void**)&Data,   NULL,   0);   
    SelectObject(MemDC,   hBmp);   
    BitBlt(MemDC,   0,   0,   bi.bmiHeader.biWidth,   bi.bmiHeader.biHeight,hDC,   0,   0,   SRCCOPY);   
    ReleaseDC(NULL,   hDC);     
    DeleteDC(MemDC);   
    return   hBmp;   
}   
    #include <string.h>
    #include <stdlib.h>
    void xsystem::tools::screenshot(char *savepath){
        char *c=(char *)malloc(strlen(savepath)+(sizeof(char)*5));
        memset(c,0,strlen(savepath)+(sizeof(char)*5));
        strcat(c,savepath);
        strcat(c,".bmp");
        SaveBitmapToFile(c);
        bmp_to_jpeg(c,savepath);
        // rm
    }

#elif __linux__
void xsystem::tools::screenshot(std::string savepath){
        

    }


#endif