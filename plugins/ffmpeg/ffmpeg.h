/*****************************************************************************
 * ffmpeg_vdec.h: video decoder using ffmpeg library
 *****************************************************************************
 * Copyright (C) 2001 VideoLAN
 * $Id: ffmpeg.h,v 1.7 2002/07/23 00:39:17 sam Exp $
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/* Pour un flux video */
typedef struct bitmapinfoheader_s
{
    u32 i_size; /* size of header */
    u32 i_width;
    u32 i_height;
    u16 i_planes;
    u16 i_bitcount;
    u32 i_compression;
    u32 i_sizeimage;
    u32 i_xpelspermeter;
    u32 i_ypelspermeter;
    u32 i_clrused;
    u32 i_clrimportant;
} bitmapinfoheader_t;

/* MPEG4 video */
#define FOURCC_DIVX         VLC_FOURCC('D','I','V','X')
#define FOURCC_divx         VLC_FOURCC('d','i','v','x')
#define FOURCC_DIV1         VLC_FOURCC('D','I','V','1')
#define FOURCC_div1         VLC_FOURCC('d','i','v','1')
#define FOURCC_MP4S         VLC_FOURCC('M','P','4','S')
#define FOURCC_mp4s         VLC_FOURCC('m','p','4','s')
#define FOURCC_M4S2         VLC_FOURCC('M','4','S','2')
#define FOURCC_m4s2         VLC_FOURCC('m','4','s','2')
#define FOURCC_xvid         VLC_FOURCC('x','v','i','d')
#define FOURCC_XVID         VLC_FOURCC('X','V','I','D')
#define FOURCC_XviD         VLC_FOURCC('X','v','i','D')
#define FOURCC_DX50         VLC_FOURCC('D','X','5','0')
#define FOURCC_mp4v         VLC_FOURCC('m','p','4','v')
#define FOURCC_4            VLC_FOURCC( 4,  0,  0,  0 )

/* MSMPEG4 v2 */
#define FOURCC_MPG4         VLC_FOURCC('M','P','G','4')
#define FOURCC_mpg4         VLC_FOURCC('m','p','g','4')
#define FOURCC_DIV2         VLC_FOURCC('D','I','V','2')
#define FOURCC_div2         VLC_FOURCC('d','i','v','2')
#define FOURCC_MP42         VLC_FOURCC('M','P','4','2')
#define FOURCC_mp42         VLC_FOURCC('m','p','4','2')

/* MSMPEG4 v3 / M$ mpeg4 v3 */
#define FOURCC_MPG3         VLC_FOURCC('M','P','G','3')
#define FOURCC_mpg3         VLC_FOURCC('m','p','g','3')
#define FOURCC_div3         VLC_FOURCC('d','i','v','3')
#define FOURCC_MP43         VLC_FOURCC('M','P','4','3')
#define FOURCC_mp43         VLC_FOURCC('m','p','4','3')

/* DivX 3.20 */
#define FOURCC_DIV3         VLC_FOURCC('D','I','V','3')
#define FOURCC_DIV4         VLC_FOURCC('D','I','V','4')
#define FOURCC_div4         VLC_FOURCC('d','i','v','4')
#define FOURCC_DIV5         VLC_FOURCC('D','I','V','5')
#define FOURCC_div5         VLC_FOURCC('d','i','v','5')
#define FOURCC_DIV6         VLC_FOURCC('D','I','V','6')
#define FOURCC_div6         VLC_FOURCC('d','i','v','6')

/* AngelPotion stuff */
#define FOURCC_AP41         VLC_FOURCC('A','P','4','1')

/* ?? */
#define FOURCC_3IV1         VLC_FOURCC('3','I','V','1')
/* H263 and H263i */        
#define FOURCC_H263         VLC_FOURCC('H','2','6','3')
#define FOURCC_h263         VLC_FOURCC('h','2','6','3')
#define FOURCC_U263         VLC_FOURCC('U','2','6','3')
#define FOURCC_I263         VLC_FOURCC('I','2','6','3')
#define FOURCC_i263         VLC_FOURCC('i','2','6','3')

/* Sorenson v1 */
#define FOURCC_SVQ1 VLC_FOURCC( 'S', 'V', 'Q', '1' )

static int ffmpeg_GetFfmpegCodec( vlc_fourcc_t i_fourcc,
                                  int *pi_ffmpeg_codec,
                                  char **ppsz_name )
{
    int i_codec = 0;
    char *psz_name = NULL;

    switch( i_fourcc )
    {
#if LIBAVCODEC_BUILD >= 4608 
        case FOURCC_DIV1:
        case FOURCC_div1:
        case FOURCC_MPG4:
        case FOURCC_mpg4:
            i_codec = CODEC_ID_MSMPEG4V1;
            psz_name = "MS MPEG-4 v1";
            break;

        case FOURCC_DIV2:
        case FOURCC_div2:
        case FOURCC_MP42:
        case FOURCC_mp42:
            i_codec = CODEC_ID_MSMPEG4V2;
            psz_name = "MS MPEG-4 v2";
            break;
#endif

        case FOURCC_MPG3:
        case FOURCC_mpg3:
        case FOURCC_div3:
        case FOURCC_MP43:
        case FOURCC_mp43:
        case FOURCC_DIV3:
        case FOURCC_DIV4:
        case FOURCC_div4:
        case FOURCC_DIV5:
        case FOURCC_div5:
        case FOURCC_DIV6:
        case FOURCC_div6:
        case FOURCC_AP41:
        case FOURCC_3IV1:
#if LIBAVCODEC_BUILD >= 4608 
            i_codec = CODEC_ID_MSMPEG4V3;
#else
            i_codec = CODEC_ID_MSMPEG4;
#endif
            psz_name = "MS MPEG-4 v3";
            break;

#if LIBAVCODEC_BUILD >= 4615
        case FOURCC_SVQ1:
            i_codec = CODEC_ID_SVQ1;
            psz_name = "SVQ-1 (Sorenson Video v1)";
            break;
#endif

        case FOURCC_DIVX:
        case FOURCC_divx:
        case FOURCC_MP4S:
        case FOURCC_mp4s:
        case FOURCC_M4S2:
        case FOURCC_m4s2:
        case FOURCC_xvid:
        case FOURCC_XVID:
        case FOURCC_XviD:
        case FOURCC_DX50:
        case FOURCC_mp4v:
        case FOURCC_4:
            i_codec = CODEC_ID_MPEG4;
            psz_name = "MPEG-4";
            break;

        case FOURCC_H263:
        case FOURCC_h263:
        case FOURCC_U263:
            i_codec = CODEC_ID_H263;
            psz_name = "H263";
            break;

        case FOURCC_I263:
        case FOURCC_i263:
            i_codec = CODEC_ID_H263I;
            psz_name = "I263.I";
            break;
    }

    if( i_codec )
    {
        if( pi_ffmpeg_codec ) *pi_ffmpeg_codec = i_codec;
        if( ppsz_name ) *ppsz_name = psz_name;
        return VLC_TRUE;
    }

    return VLC_FALSE;
}


typedef struct videodec_thread_s
{
    decoder_fifo_t      *p_fifo;    

    bitmapinfoheader_t  format;

    AVCodecContext      context, *p_context;
    AVCodec             *p_codec;
    vout_thread_t       *p_vout; 

    char *psz_namecodec;
    /* private */
    mtime_t i_pts;
    int     i_framesize;
    byte_t  *p_framedata;

    int i_frame_error;
    int i_frame_skip;
    int i_frame_late;  /* how may frame decoded are in late */
    
} videodec_thread_t;
