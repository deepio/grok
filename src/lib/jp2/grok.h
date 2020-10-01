/*
 *    Copyright (C) 2016-2020 Grok Image Compression Inc.
 *
 *    This source code is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This source code is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *    This source code incorporates work covered by the BSD 2-clause license.
 *    Please see the LICENSE file in the root directory for details.
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "grk_config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#ifdef __GNUC__
#define GRK_DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define GRK_DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define GRK_DEPRECATED(func) func
#endif

#if defined(GRK_STATIC) || !defined(_WIN32)
#		if defined(GRK_STATIC) /* static library uses "hidden" */
#			define GRK_API    __attribute__ ((visibility ("hidden")))
#		else
#			define GRK_API    __attribute__ ((visibility ("default")))
#		endif
#	define GRK_CALLCONV
#else
#	define GRK_CALLCONV __stdcall
#	if defined(GRK_EXPORTS) || defined(DLL_EXPORT)
#		define GRK_API __declspec(dllexport)
#	else
#		define GRK_API __declspec(dllimport)
#	endif
#endif

enum GRK_SUPPORTED_FILE_FMT {
	GRK_UNK_FMT,
	GRK_J2K_FMT,
	GRK_JP2_FMT,
	GRK_PXM_FMT,
	GRK_PGX_FMT,
	GRK_PAM_FMT,
	GRK_BMP_FMT,
	GRK_TIF_FMT,
	GRK_RAW_FMT, /* MSB / Big Endian */
	GRK_PNG_FMT,
	GRK_RAWL_FMT, /* LSB / Little Endian */
	GRK_JPG_FMT
};

#define GRK_PATH_LEN 4096 /**< Maximum allowed size for filenames */

/*
 * note: range for number of decomposition levels is 0-32
 * So, accordingly, range for number of resolutions is 1-33
 * */

#define GRK_J2K_MAXRLVLS 33						/**< Maximum number of resolution levels authorized */
#define GRK_J2K_MAXBANDS (3*GRK_J2K_MAXRLVLS-2)	/**< Maximum number of sub-bands */

// used by dump
#define GRK_IMG_INFO		1	/**< Basic image information provided to the user */
#define GRK_J2K_MH_INFO		2	/**< Codestream information based only on the main header */
#define GRK_J2K_TH_INFO		4	/**< Tile information based on the current tile header */
#define GRK_J2K_TCH_INFO	8	/**< Tile/Component information of all tiles */
#define GRK_J2K_MH_IND		16	/**< Codestream index based only on the main header */
#define GRK_J2K_TH_IND		32	/**< Tile index based on the current tile */
#define GRK_JP2_INFO		128	/**< JP2 file information */
#define GRK_JP2_IND			256	/**< JP2 file index */

#define GRK_CBLKSTY_LAZY    0x01  /**< Selective arithmetic coding bypass */
#define GRK_CBLKSTY_RESET   0x02  /**< Reset context probabilities on coding pass boundaries */
#define GRK_CBLKSTY_TERMALL 0x04  /**< Termination on each coding pass */
#define GRK_CBLKSTY_VSC     0x08  /**< Vertically stripe causal context */
#define GRK_CBLKSTY_PTERM   0x10  /**< Predictable termination */
#define GRK_CBLKSTY_SEGSYM  0x20  /**< Segmentation symbols are used */
#define GRK_CBLKSTY_HT     	0x40  /**< high throughput block coding */
#define GRK_JPH_RSIZ_FLAG   0x4000 /**<for JPH, bit 14 of RSIZ must be set to 1 */

/**
 * JPEG 2000 Profiles, see Table A.10 from 15444-1 (updated in various AMDs)
 *
 * These values help choose the RSIZ value for the JPEG 2000 code stream.
 * The RSIZ value forces various encoding options, as detailed in Table A.10.
 * If GRK_PROFILE_PART2 is chosen, it must be combined with one or more extensions
 * described below.
 *
 *   Example: rsiz = GRK_PROFILE_PART2 | GRK_EXTENSION_MCT;
 *
 * For broadcast profiles, the GRK_PROFILE_X value has to be combined with the target
 * level (3-0 LSB, value between 0 and 11):
 *   Example: rsiz = GRK_PROFILE_BC_MULTI | 0x0005; //level equals 5
 *
 * For IMF profiles, the GRK_PROFILE_X value has to be combined with the target main-level
 * (3-0 LSB, value between 0 and 11) and sub-level (7-4 LSB, value between 0 and 9):
 *   Example: rsiz = GRK_PROFILE_IMF_2K | 0x0040 | 0x0005; // main-level equals 5 and sub-level equals 4
 * 
 * */
#define GRK_PROFILE_NONE        0x0000 /** no profile, conform to 15444-1 */
#define GRK_PROFILE_0           0x0001 /** Profile 0 as described in 15444-1,Table A.45 */
#define GRK_PROFILE_1           0x0002 /** Profile 1 as described in 15444-1,Table A.45 */
#define GRK_PROFILE_CINEMA_2K   0x0003 /** 2K cinema profile defined in 15444-1 AMD1 */
#define GRK_PROFILE_CINEMA_4K   0x0004 /** 4K cinema profile defined in 15444-1 AMD1 */
#define GRK_PROFILE_CINEMA_S2K  0x0005 /** Scalable 2K cinema profile defined in 15444-1 AMD2 */
#define GRK_PROFILE_CINEMA_S4K  0x0006 /** Scalable 4K cinema profile defined in 15444-1 AMD2 */
#define GRK_PROFILE_CINEMA_LTS  0x0007 /** Long term storage cinema profile defined in 15444-1 AMD2 */
#define GRK_PROFILE_BC_SINGLE   0x0100 /** Single Tile Broadcast profile defined in 15444-1 AMD3 */
#define GRK_PROFILE_BC_MULTI    0x0200 /** Multi Tile Broadcast profile defined in 15444-1 AMD3 */
#define GRK_PROFILE_BC_MULTI_R  0x0300 /** Multi Tile Reversible Broadcast profile defined in 15444-1 AMD3 */
#define GRK_PROFILE_BC_MASK		0x030F /** Mask for broadcast profile including main level */
#define GRK_PROFILE_IMF_2K      0x0400 /** 2K Single Tile Lossy IMF profile defined in 15444-1 AMD8 */
#define GRK_PROFILE_IMF_4K      0x0500 /** 4K Single Tile Lossy IMF profile defined in 15444-1 AMD8 */
#define GRK_PROFILE_IMF_8K      0x0600 /** 8K Single Tile Lossy IMF profile defined in 15444-1 AMD8 */
#define GRK_PROFILE_IMF_2K_R    0x0700 /** 2K Single/Multi Tile Reversible IMF profile defined in 15444-1 AMD8 */
#define GRK_PROFILE_IMF_4K_R    0x0800 /** 4K Single/Multi Tile Reversible IMF profile defined in 15444-1 AMD8 */
#define GRK_PROFILE_IMF_8K_R    0x0900  /** 8K Single/Multi Tile Reversible IMF profile defined in 15444-1 AMD8 */
#define GRK_PROFILE_MASK		0x0FFF  /** Mask for profile bits */
#define GRK_PROFILE_PART2						0x8000 /** At least 1 extension defined in 15444-2 (Part-2) */
#define GRK_PROFILE_PART2_EXTENSIONS_MASK       0x3FFF // Mask for Part-2 extension bits

/**
 * JPEG 2000 Part-2 extensions
 * */
#define GRK_EXTENSION_NONE      0x0000 /** No Part-2 extension */
#define GRK_EXTENSION_MCT       0x0100  /** Custom MCT support */
#define GRK_IS_PART2(v)      ((v) & GRK_PROFILE_PART2)

#define GRK_IS_CINEMA(v)     (((v) >= GRK_PROFILE_CINEMA_2K) && ((v) <= GRK_PROFILE_CINEMA_S4K))
#define GRK_IS_STORAGE(v)    ((v) == GRK_PROFILE_CINEMA_LTS)

/*
 *
 * *********************************************
 * Broadcast level (3-0 LSB) (15444-1 AMD4,AMD8)
 * *********************************************
 *
 * indicates maximum bit rate and sample rate for a code stream
 *
 * Note: Mbit/s == 10^6 bits/s;  Msamples/s == 10^6 samples/s
 *
 * 0:		no maximum rate
 * 1:		200 Mbits/s, 65  Msamples/s
 * 2:		200 Mbits/s, 130 Msamples/s
 * 3:		200 Mbits/s, 195 Msamples/s
 * 4:		400 Mbits/s, 260 Msamples/s
 * 5:		800Mbits/s,  520 Msamples/s
 * >= 6:	2^(level-6) * 1600 Mbits/s, 2^(level-6) * 1200 Msamples/s
 *
 * Note: level cannot be greater than 11
 *
 * ****************
 * Broadcast tiling
 * ****************
 *
 * Either single-tile or multi-tile. Multi-tile only permits
 * 1 or 4 tiles per frame, where multiple tiles have identical
 * sizes, and are configured in either 2x2 or 1x4 layout.
 *
 *************************************************************
 *
 * ***************************************
 * IMF main-level (3-0) LSB (15444-1 AMD8)
 * ***************************************
 *
 * main-level indicates maximum number of samples per second,
 * as listed above.
 *
 *
 * **************************************
 * IMF sub-level (7-4) LSB (15444-1 AMD8)
 * **************************************
 *
 * sub-level indicates maximum bit rate for a code stream:
 *
 * 0:	no maximum rate
 * >0:	2^sub-level * 100 Mbits/second
 *
 * Note: sub-level cannot be greater than 9, and cannot be larger
 * then maximum of (main-level -2) and 1.
 *
 */
/** Extract profile without mainlevel/sublevel */
#define GRK_GET_IMF_OR_BROADCAST_PROFILE(v)   ((v) & 0x0f00)

#define GRK_LEVEL_MAX    11U   			/** Maximum (main) level */
#define GRK_GET_LEVEL(v) ((v) & 0xf)    /** Extract (main) level */

/******* BROADCAST **********/

#define GRK_IS_BROADCAST(v)  (  ((v) >= GRK_PROFILE_BC_SINGLE) && \
								((v) <= (GRK_PROFILE_BC_MULTI_R | 0x000b) )  && \
							  ( ((v) & 0xf) <= 0xb )    )

/* Maximum component sampling Rate (Mbits/sec) per level */
#define GRK_BROADCAST_LEVEL_1_MBITSSEC   200U     /** Mbits/sec for level 1 */
#define GRK_BROADCAST_LEVEL_2_MBITSSEC   200U     /** Mbits/sec for level 2 */
#define GRK_BROADCAST_LEVEL_3_MBITSSEC   200U     /** Mbits/sec for level 3 */
#define GRK_BROADCAST_LEVEL_4_MBITSSEC   400U     /** Mbits/sec for level 4 */
#define GRK_BROADCAST_LEVEL_5_MBITSSEC   800U     /** Mbits/sec for level 5 */
#define GRK_BROADCAST_LEVEL_6_MBITSSEC   1600U    /** Mbits/sec for level 6 */
#define GRK_BROADCAST_LEVEL_7_MBITSSEC   3200U    /** Mbits/sec for level 7 */
#define GRK_BROADCAST_LEVEL_8_MBITSSEC   6400U    /** Mbits/sec for level 8 */
#define GRK_BROADCAST_LEVEL_9_MBITSSEC   12800U   /** Mbits/sec for level 9 */
#define GRK_BROADCAST_LEVEL_10_MBITSSEC  25600U   /** Mbits/sec for level 10 */
#define GRK_BROADCAST_LEVEL_11_MBITSSEC  51200U   /** Mbits/sec for level 11 */

#define GRK_BROADCAST_LEVEL_1_MSAMPLESSEC   64U    /** MSamples/sec for level 1 */
#define GRK_BROADCAST_LEVEL_2_MSAMPLESSEC   130U   /** MSamples/sec for level 2 */
#define GRK_BROADCAST_LEVEL_3_MSAMPLESSEC   195U   /** MSamples/sec for level 3 */
#define GRK_BROADCAST_LEVEL_4_MSAMPLESSEC   260U   /** MSamples/sec for level 4 */
#define GRK_BROADCAST_LEVEL_5_MSAMPLESSEC   520U   /** MSamples/sec for level 5 */
#define GRK_BROADCAST_LEVEL_6_MSAMPLESSEC   1200U  /** MSamples/sec for level 6 */
#define GRK_BROADCAST_LEVEL_7_MSAMPLESSEC   2400U  /** MSamples/sec for level 7 */
#define GRK_BROADCAST_LEVEL_8_MSAMPLESSEC   4800U  /** MSamples/sec for level 8 */
#define GRK_BROADCAST_LEVEL_9_MSAMPLESSEC   9600U  /** MSamples/sec for level 9 */
#define GRK_BROADCAST_LEVEL_10_MSAMPLESSEC  19200U /** MSamples/sec for level 10 */
#define GRK_BROADCAST_LEVEL_11_MSAMPLESSEC  38400U /** MSamples/sec for level 11 */

/********IMF ***********************************************************************/

#define GRK_IS_IMF(v)        (  ((v) >= GRK_PROFILE_IMF_2K) && \
		                        ((v) <= (GRK_PROFILE_IMF_8K_R | 0x009b) )  && \
							  ( ((v) & 0xf) <= 0xb ) && \
							  ( ((v) & 0xf0) <= 0x90 )    )

/* Maximum component sampling rate (MSamples/sec) per main level */
#define GRK_IMF_MAINLEVEL_1_MSAMPLESSEC   65U      /** MSamples/sec for main level 1 */
#define GRK_IMF_MAINLEVEL_2_MSAMPLESSEC   130U     /** MSamples/sec for main level 2 */
#define GRK_IMF_MAINLEVEL_3_MSAMPLESSEC   195U     /** MSamples/sec for main level 3 */
#define GRK_IMF_MAINLEVEL_4_MSAMPLESSEC   260U     /** MSamples/sec for main level 4 */
#define GRK_IMF_MAINLEVEL_5_MSAMPLESSEC   520U     /** MSamples/sec for main level 5 */
#define GRK_IMF_MAINLEVEL_6_MSAMPLESSEC   1200U    /** MSamples/sec for main level 6 */
#define GRK_IMF_MAINLEVEL_7_MSAMPLESSEC   2400U    /** MSamples/sec for main level 7 */
#define GRK_IMF_MAINLEVEL_8_MSAMPLESSEC   4800U    /** MSamples/sec for main level 8 */
#define GRK_IMF_MAINLEVEL_9_MSAMPLESSEC   9600U    /** MSamples/sec for main level 9 */
#define GRK_IMF_MAINLEVEL_10_MSAMPLESSEC  19200U   /** MSamples/sec for main level 10 */
#define GRK_IMF_MAINLEVEL_11_MSAMPLESSEC  38400U   /** MSamples/sec for main level 11 */

#define GRK_IMF_SUBLEVEL_MAX    	9U   			 /** Maximum IMF sublevel */
#define GRK_GET_IMF_SUBLEVEL(v)  (((v) >> 4) & 0xf)  /** Extract IMF sub level */

/** Maximum compressed bit rate (Mbits/s) per IMF sub level */
#define GRK_IMF_SUBLEVEL_1_MBITSSEC      200U     /** Mbits/s for IMF sub level 1 */
#define GRK_IMF_SUBLEVEL_2_MBITSSEC      400U     /** Mbits/s for IMF sub level 2 */
#define GRK_IMF_SUBLEVEL_3_MBITSSEC      800U     /** Mbits/s for IMF sub level 3 */
#define GRK_IMF_SUBLEVEL_4_MBITSSEC     1600U     /** Mbits/s for IMF sub level 4 */
#define GRK_IMF_SUBLEVEL_5_MBITSSEC     3200U     /** Mbits/s for IMF sub level 5 */
#define GRK_IMF_SUBLEVEL_6_MBITSSEC     6400U     /** Mbits/s for IMF sub level 6 */
#define GRK_IMF_SUBLEVEL_7_MBITSSEC    12800U     /** Mbits/s for IMF sub level 7 */
#define GRK_IMF_SUBLEVEL_8_MBITSSEC    25600U     /** Mbits/s for IMF sub level 8 */
#define GRK_IMF_SUBLEVEL_9_MBITSSEC    51200U     /** Mbits/s for IMF sub level 9 */
/**********************************************************************************/

/**
 * JPEG 2000 cinema profile code stream and component size limits
 * */
#define GRK_CINEMA_24_CS     1302083U    /** Maximum code stream length @ 24fps */
#define GRK_CINEMA_48_CS     651041U     /** Maximum code stream length @ 48fps */
#define GRK_CINEMA_24_COMP   1041666U    /** Maximum size per color component @ 24fps */
#define GRK_CINEMA_48_COMP   520833U	 /** Maximum size per color component @ 48fps */

/**
 * Progression order
 * */
typedef enum _GRK_PROG_ORDER {
	GRK_PROG_UNKNOWN = -1, /**< place-holder */
	GRK_LRCP = 0, /**< layer-resolution-component-precinct order */
	GRK_RLCP = 1, /**< resolution-layer-component-precinct order */
	GRK_RPCL = 2, /**< resolution-precinct-component-layer order */
	GRK_PCRL = 3, /**< precinct-component-resolution-layer order */
	GRK_CPRL = 4 /**< component-precinct-resolution-layer order */
} GRK_PROG_ORDER;

/*
 *
 * CIE Lab #defines
 */
#define GRK_CUSTOM_CIELAB_SPACE 0x0
#define GRK_DEFAULT_CIELAB_SPACE 0x44454600  //'DEF'
#define GRK_CIE_DAY   ((((uint32_t) 'C')<<24) + (((uint32_t) 'T')<<16))
#define GRK_CIE_D50   ((uint32_t) 0x00443530)
#define GRK_CIE_D65   ((uint32_t) 0x00443635)
#define GRK_CIE_D75   ((uint32_t) 0x00443735)
#define GRK_CIE_SA   ((uint32_t) 0x00005341)
#define GRK_CIE_SC   ((uint32_t) 0x00005343)
#define GRK_CIE_F2   ((uint32_t) 0x00004632)
#define GRK_CIE_F7   ((uint32_t) 0x00004637)
#define GRK_CIE_F11   ((uint32_t) 0x00463131)

/**
 * Supported image color spaces
 * */
typedef enum _GRK_COLOR_SPACE {
	GRK_CLRSPC_UNKNOWN = 0, 		/**< unknown */
	GRK_CLRSPC_SRGB = 2, 			/**< sRGB */
	GRK_CLRSPC_GRAY = 3, 			/**< grayscale */
	GRK_CLRSPC_SYCC = 4, 			/**< standard YCC (YUV) */
	GRK_CLRSPC_EYCC = 5, 			/**< extended YCC */
	GRK_CLRSPC_CMYK = 6, 			/**< CMYK */
	GRK_CLRSPC_DEFAULT_CIE = 7, 	/**< default CIE LAB */
	GRK_CLRSPC_CUSTOM_CIE = 8, 		/**< custom CIE LAB */
	GRK_CLRSPC_ICC = 9 				/**< ICC profile */
} GRK_COLOR_SPACE;

/* JPEG 2000 standard values for colour spaces */
enum GRK_ENUM_COLOUR_SPACE{
	GRK_ENUM_CLRSPC_UNKNOWN = 0,
	GRK_ENUM_CLRSPC_CMYK = 12,
	GRK_ENUM_CLRSPC_CIE = 14,
	GRK_ENUM_CLRSPC_SRGB = 16,
	GRK_ENUM_CLRSPC_GRAY = 17,
	GRK_ENUM_CLRSPC_SYCC = 18,
	GRK_ENUM_CLRSPC_EYCC = 24
};

/**
 * Supported codecs
 */
typedef enum _GRK_CODEC_FORMAT {
	GRK_CODEC_UNKNOWN = -1, /**< place-holder */
	GRK_CODEC_J2K = 0, /**< JPEG 2000 code stream : read/write */
	GRK_CODEC_JP2 = 2 /**< JP2 file format : read/write */
} GRK_CODEC_FORMAT;

#define  GRK_NUM_COMMENTS_SUPPORTED 256
#define GRK_MAX_COMMENT_LENGTH (UINT16_MAX-2)

/**
 * Callback function prototype for logging
 *
 * @param msg               Event message
 * @param client_data       Client object where will be return the event message
 * */
typedef void (*grk_msg_callback)(const char *msg, void *client_data);

/*
 ==========================================================
 codec structures
 ==========================================================
 */

/**
 * Progression order changes
 *
 */
typedef struct _grk_poc {
	/** Resolution num start, component num start, given by POC */
	uint32_t resno0, compno0;
	/** Layer num end, resolution num end, component num end, given by POC */
	uint16_t layno1;
	uint32_t resno1, compno1;
	/** Layer num start,precinct num start, precinct num end */
	uint16_t layno0;
	uint64_t precno0, precno1;
	/** Progression order enum*/
	GRK_PROG_ORDER prg1, prg;
	/** Progression order string*/
	char progorder[5];
	/** Tile number */
	uint32_t tile;
	/** Start and end values for tile width and height*/
	uint32_t tx0, tx1, ty0, ty1;
	/** Start value, initialized in pi_initialise_encode*/
	uint32_t resS, compS;
	/** End value, initialized in pi_initialise_encode */
	uint16_t layE;
	uint32_t resE, compE;
	uint64_t prcE;
	/** Start and end values of tile width and height, initialized in pi_initialise_encode*/
	uint32_t txS, txE, tyS, tyE, dx, dy;
	/** Temporary values for Tile parts, initialized in pi_create_encode */
	uint16_t lay_t;
	uint32_t res_t, comp_t, tx0_t, ty0_t;
	uint64_t prc_t;
} grk_poc;

/**@name RAW component compress parameters */
/*@{*/
typedef struct _grk_raw_comp_cparameters {
	/** subsampling in X direction */
	uint32_t dx;
	/** subsampling in Y direction */
	uint32_t dy;
	/*@}*/
} grk_raw_comp_cparameters;

/**@name RAW image compress parameters */
/*@{*/
typedef struct _grk_raw_cparameters {
	/** width of the raw image */
	uint32_t width;
	/** height of the raw image */
	uint32_t height;
	/** number of components of the raw image */
	uint16_t numcomps;
	/** bit depth of the raw image */
	uint32_t prec;
	/** signed/unsigned raw image */
	bool sgnd;
	/** raw components parameters */
	grk_raw_comp_cparameters *comps;
	/*@}*/
} grk_raw_cparameters;

/**
 * Compress parameters
 * */
typedef struct _grk_cparameters {
	/** size of tile: tile_size_on = false (not in argument) or = true (in argument) */
	bool tile_size_on;
	/** XTOsiz */
	uint32_t tx0;
	/** YTOsiz */
	uint32_t ty0;
	/** XTsiz */
	uint32_t t_width;
	/** YTsiz */
	uint32_t t_height;
	/** allocation by rate/distortion */
	bool cp_disto_alloc;
	/** allocation by fixed_quality */
	bool cp_fixed_quality;
	/** comment for coding */
	char *cp_comment[GRK_NUM_COMMENTS_SUPPORTED];
	uint16_t cp_comment_len[GRK_NUM_COMMENTS_SUPPORTED];
	bool cp_is_binary_comment[GRK_NUM_COMMENTS_SUPPORTED];
	size_t cp_num_comments;
	/** csty : coding style */
	uint8_t csty;
	/** progression order (default GRK_LRCP) */
	GRK_PROG_ORDER prog_order;
	/** progression order changes */
	grk_poc POC[32];
	/** number of progression order changes (POCs), default to 0 */
	uint32_t numpocs;
	/** number of layers */
	uint16_t tcp_numlayers;
	/** rates of layers, expressed as compression ratios.
	 *  They might be subsequently limited by the max_cs_size field */
	double tcp_rates[100];
	/** different psnr for successive layers */
	double tcp_distoratio[100];
	/** number of resolutions */
	uint32_t numresolution;
	/** initial code block width  (default to 64) */
	uint32_t cblockw_init;
	/** initial code block height (default to 64) */
	uint32_t cblockh_init;
	/** code block style */
	uint8_t cblk_sty;
	/* flag for high throughput */
	bool isHT;
	/** 1 : use the irreversible DWT 9-7, 0 :
	 *  use lossless compression (default) */
	bool irreversible;
	/** region of interest: affected component in [0..3];
	 *  -1 indicates no ROI */
	int32_t roi_compno;
	/** region of interest: upshift value */
	uint32_t roi_shift;
	/* number of precinct size specifications */
	uint32_t res_spec;
	/** initial precinct width */
	uint32_t prcw_init[GRK_J2K_MAXRLVLS];
	/** initial precinct height */
	uint32_t prch_init[GRK_J2K_MAXRLVLS];
	/** input file name */
	char infile[GRK_PATH_LEN];
	/** output file name */
	char outfile[GRK_PATH_LEN];
	/** subimage encoding: origin image offset in x direction */
	uint32_t image_offset_x0;
	/** subimage encoding: origin image offset in y direction */
	uint32_t image_offset_y0;
	/** subsampling value for dx */
	uint32_t subsampling_dx;
	/** subsampling value for dy */
	uint32_t subsampling_dy;
	/** input file format*/
	GRK_SUPPORTED_FILE_FMT decod_format;
	/** output file format*/
	GRK_SUPPORTED_FILE_FMT cod_format;
	grk_raw_cparameters raw_cp;
	/**
	 * Maximum size (in bytes) for each component.
	 * If == 0, component size limitation is not considered
	 * */
	uint32_t max_comp_size;
	/** Tile part generation*/
	uint8_t tp_on;
	/** Flag for tile part generation*/
	uint8_t tp_flag;
	/** MCT (multiple component transform) */
	uint8_t tcp_mct;
	/** Naive implementation of MCT restricted to a single reversible array based
	 encoding without offset concerning all the components. */
	void *mct_data;
	/**
	 * Maximum size (in bytes) for the whole code stream.
	 * If equal to zero, code stream size limitation is not considered
	 * If it does not comply with tcp_rates, max_cs_size prevails
	 * and a warning is issued.
	 * */
	uint64_t max_cs_size;
	/** RSIZ value
	 To be used to combine GRK_PROFILE_*, GRK_EXTENSION_* and (sub)levels values. */
	uint16_t rsiz;
	int framerate;

	// set to true if input file stores capture resolution
	bool write_capture_resolution_from_file;
	double capture_resolution_from_file[2];

	bool write_capture_resolution;
	double capture_resolution[2];

	bool write_display_resolution;
	double display_resolution[2];

	// 0: bisect with all truncation points,  1: bisect with only feasible truncation points
	uint32_t rateControlAlgorithm;
	uint32_t numThreads;
	int32_t deviceId;
	uint32_t duration; //seconds
	uint32_t kernelBuildOptions;
	uint32_t repeats;
	bool writePLT;
	bool writeTLM;
	bool verbose;
} grk_cparameters;

/**
 Channel description: channel index, type, association
 */
typedef struct _grk_jp2_cdef_info {
	uint16_t cn;
	uint16_t typ;
	uint16_t asoc;
} grk_jp2_cdef_info;

/**
 Channel descriptions and number of descriptions
 */
typedef struct _grk_jp2_cdef {
	grk_jp2_cdef_info *info;
	uint16_t n;
} grk_jp2_cdef;

/**
 Component mappings: channel index, mapping type, palette index
 */
typedef struct _grk_jp2_cmap_comp {
	uint16_t cmp;
	uint8_t mtyp, pcol;
} grk_jp2_cmap_comp;

/**
 Palette data: table entries, palette columns
 */
typedef struct _grk_jp2_pclr {
	uint32_t *entries;
	uint8_t *channel_sign;
	uint8_t *channel_size;
	grk_jp2_cmap_comp *cmap;
	uint16_t nr_entries;
	uint8_t nr_channels;
} grk_jp2_pclr;

/**
 ICC profile, palette, component mapping, channel description
 */
typedef struct _grk_jp2_color {
	uint8_t *icc_profile_buf;
	uint32_t icc_profile_len;
	grk_jp2_cdef *jp2_cdef;
	grk_jp2_pclr *jp2_pclr;
	uint8_t jp2_has_colour_specification_box;
} grk_jp2_color;

/**
 * Header info
 */
typedef struct _grk_header_info {
	/** initial code block width, default to 64 */
	uint32_t cblockw_init;
	/** initial code block height, default to 64 */
	uint32_t cblockh_init;
	/** 1 : use the irreversible DWT 9-7, 0 : use lossless compression (default) */
	bool irreversible;
	/** multi-component transform identifier */
	uint32_t mct;
	/** RSIZ value
	 To be used to combine GRK_PROFILE_*, GRK_EXTENSION_* and (sub)levels values. */
	uint16_t rsiz;
	/** number of resolutions */
	uint32_t numresolutions;
	// coding style can be specified in main header COD segment,
	// tile header COD segment, and tile component COC segment.
	// !!! Assume that coding style does not vary across tile components !!!
	uint8_t csty;
	// code block style is specified in main header COD segment, and can
	// be overridden in a tile header. !!! Assume that style does
	// not vary across tiles !!!
	uint8_t cblk_sty;
	/** initial precinct width */
	uint32_t prcw_init[GRK_J2K_MAXRLVLS];
	/** initial precinct height */
	uint32_t prch_init[GRK_J2K_MAXRLVLS];
	/** XTOsiz */
	uint32_t tx0;
	/** YTOsiz */
	uint32_t ty0;
	/** XTsiz */
	uint32_t t_width;
	/** YTsiz */
	uint32_t t_height;
	/** tile grid width */
	uint32_t t_grid_width;
	/** tile grid height  */
	uint32_t t_grid_height;
	/** number of layers */
	uint16_t tcp_numlayers;

	GRK_ENUM_COLOUR_SPACE enumcs;
	// ICC profile information
	// note: the contents of this struct will remain valid
	// until the codec is destroyed
	grk_jp2_color color;
	// note: xml_data will remain valid
	// until codec is destroyed
	uint8_t *xml_data;
	size_t xml_data_len;
	size_t num_comments;
	char *comment[GRK_NUM_COMMENTS_SUPPORTED];
	uint16_t comment_len[GRK_NUM_COMMENTS_SUPPORTED];
	bool isBinaryComment[GRK_NUM_COMMENTS_SUPPORTED];

	bool has_capture_resolution;
	double capture_resolution[2];

	bool has_display_resolution;
	double display_resolution[2];

} grk_header_info;

/**
 * Core decompress parameters
 * */
typedef struct _grk_dparameters {
	/**
	 Set the number of highest resolution levels to be discarded.
	 The image resolution is effectively divided by 2 to the power of the number of discarded levels.
	 The reduce factor is limited by the smallest total number of decomposition levels among tiles.
	 if greater than zero, then image is deocded to original dimension divided by 2^(cp_reduce);
	 if equal to zero or not used, image is decoded to full resolution
	 */
	uint32_t cp_reduce;
	/**
	 Set the maximum number of quality layers to decompress.
	 If there are fewer quality layers than the specified number, all quality layers will be decoded.
	 if != 0, then only the first "layer" layers are decoded;
	 if == 0 or not used, all the quality layers are decoded
	 */
	uint16_t cp_layer;
	/** input file name */
	char infile[GRK_PATH_LEN];
	/** output file name */
	char outfile[GRK_PATH_LEN];
	/** input file format*/
	GRK_SUPPORTED_FILE_FMT decod_format;
	/** output file format*/
	GRK_SUPPORTED_FILE_FMT cod_format;
	/** Decoding area left boundary */
	uint32_t DA_x0;
	/** Decoding area right boundary */
	uint32_t DA_x1;
	/** Decoding area up boundary */
	uint32_t DA_y0;
	/** Decoding area bottom boundary */
	uint32_t DA_y1;
	/** Verbose mode */
	bool m_verbose;
	/** tile number of the decompressed tile*/
	uint16_t tile_index;
	/** Number of tiles to decompress */
	uint32_t nb_tile_to_decode;
	uint32_t flags;
} grk_dparameters;

/**
 * Precision mode
 */
typedef enum grk_prec_mode {
	GRK_PREC_MODE_CLIP, GRK_PREC_MODE_SCALE
} grk_precision_mode;

/**
 * Precision
 */
typedef struct _grk_prec {
	uint32_t prec;
	grk_precision_mode mode;
} grk_precision;

#define GRK_DECOMPRESS_COMPRESSION_LEVEL_DEFAULT (UINT_MAX)

/**
 * Decompress parameters
 */
typedef struct _grk_decompress_params {
	/** core library parameters */
	grk_dparameters core;
	/** input file name */
	char infile[GRK_PATH_LEN];
	/** output file name */
	char outfile[GRK_PATH_LEN];
	/** input file format 0: J2K, 1: JP2*/
	GRK_SUPPORTED_FILE_FMT decod_format;
	/** output file format 0: PGX, 1: PxM, 2: BMP */
	GRK_SUPPORTED_FILE_FMT cod_format;
	/** index file name */
	char indexfilename[GRK_PATH_LEN];
	/** Decoding area left boundary */
	uint32_t DA_x0;
	/** Decoding area right boundary */
	uint32_t DA_x1;
	/** Decoding area up boundary */
	uint32_t DA_y0;
	/** Decoding area bottom boundary */
	uint32_t DA_y1;
	/** Verbose mode */
	bool m_verbose;
	/** tile number of the decoded tile*/
	uint16_t tile_index;
	/** Number of tiles to decompress */
	uint32_t nb_tile_to_decode;
	grk_precision *precision;
	uint32_t nb_precision;
	/* force output colorspace to RGB */
	bool force_rgb;
	/* upsample components according to their dx/dy values */
	bool upsample;
	/* split output components to different files */
	bool split_pnm;
	/* serialize XML metadata to disk */
	bool serialize_xml;
	uint32_t compression;
	// compression "quality". Meaning of "quality" depends
	// on file format we are writing to
	uint32_t compressionLevel;
	int32_t deviceId;
	uint32_t duration; //seconds
	uint32_t kernelBuildOptions;
	uint32_t repeats;
	bool verbose;
	uint32_t numThreads;
} grk_decompress_parameters;

typedef void *grk_codec;

/*
 ==========================================================
 I/O stream typedef definitions
 ==========================================================
 */

/*
 * Callback function prototype for read function
 *
 */
typedef size_t (*grk_stream_read_fn)(void *p_buffer, size_t nb_bytes,
		void *user_data);



/*
 * Callback function prototype for write function
 */
typedef size_t (*grk_stream_write_fn)(void *p_buffer, size_t nb_bytes,
		void *user_data);

/*
 * Callback function prototype for (absolute) seek function.
 */
typedef bool (*grk_stream_seek_fn)(uint64_t nb_bytes, void *user_data);

/*
 * Callback function prototype for free user data function
 */
typedef void (*grk_stream_free_user_data_fn)(void *user_data);

/*
 * JPEG 2000 stream.
 */
typedef void *grk_stream;

/*
 ==============================
 image typedef definitions
 ==============================
 */

// component type
enum GRK_COMPONENT_TYPE{

	GRK_COMPONENT_TYPE_COLOUR =	0,
	GRK_COMPONENT_TYPE_OPACITY = 	1,
	GRK_COMPONENT_TYPE_PREMULTIPLIED_OPACITY =  2,
	GRK_COMPONENT_TYPE_UNSPECIFIED = 65535U

};

// component association
enum GRK_COMPONENT_ASSOC{

	GRK_COMPONENT_ASSOC_WHOLE_IMAGE =	0,
	GRK_COMPONENT_ASSOC_COLOUR_1 = 	1,
	GRK_COMPONENT_ASSOC_COLOUR_2 =  2,
	GRK_COMPONENT_ASSOC_COLOUR_3 =  3,
	GRK_COMPONENT_ASSOC_UNASSOCIATED = 65535U

};

/**
 * Image component
 * */
typedef struct _grk_image_comp {
	/** XRsiz: horizontal separation of a sample of with component with respect to the reference grid */
	uint32_t dx;
	/** YRsiz: vertical separation of a sample of with component with respect to the reference grid */
	uint32_t dy;
	/** data width */
	uint32_t w;
	/** data stride */
	uint32_t stride;
	/** data height */
	uint32_t h;
	/** x component offset compared to the whole image */
	uint32_t x0;
	/** y component offset compared to the whole image */
	uint32_t y0;
	/** precision */
	uint32_t prec;
	bool sgnd;
	/** image component data */
	int32_t *data;
	// if true, then image will manage data, otherwise up to caller
	bool owns_data;
	GRK_COMPONENT_TYPE type;
	GRK_COMPONENT_ASSOC association;
} grk_image_comp;

/**
 * Image
 * */
typedef struct _grk_image {
	/** XOsiz: horizontal offset from the origin of the reference grid
	 *  to the left side of the image area */
	uint32_t x0;
	/** YOsiz: vertical offset from the origin of the reference grid
	 *  to the top side of the image area */
	uint32_t y0;
	/** Xsiz: width of the reference grid */
	uint32_t x1;
	/** Ysiz: height of the reference grid */
	uint32_t y1;
	/** number of components in the image */
	uint32_t numcomps;
	/** color space: sRGB, Greyscale or YUV */
	GRK_COLOR_SPACE color_space;
	/** image components */
	grk_image_comp *comps;
	/** 'restricted' ICC profile */
	uint8_t *icc_profile_buf;
	/** size of ICC profile */
	uint32_t icc_profile_len;
	double capture_resolution[2];
	double display_resolution[2];
	uint8_t *iptc_buf;
	size_t iptc_len;
	uint8_t *xmp_buf;
	size_t xmp_len;
} grk_image;

/**
 * Image component parameters
 * */
typedef struct _grk_image_comptparm {
	/** XRsiz: horizontal separation of a sample of component
	 *  with respect to the reference grid */
	uint32_t dx;
	/** YRsiz: vertical separation of a sample of component
	 *  with respect to the reference grid */
	uint32_t dy;
	/** data width */
	uint32_t w;
	/* data stride */
	uint32_t stride;
	/** data height */
	uint32_t h;
	/** x component offset compared to the whole image */
	uint32_t x0;
	/** y component offset compared to the whole image */
	uint32_t y0;
	/** precision */
	uint32_t prec;
	/** true if data is signed */
	bool sgnd;
} grk_image_cmptparm;

/*
 ==========================================================
 Information about the JPEG 2000 code stream
 ==========================================================
 */

/**
 * Packet info
 * */
typedef struct _grk_packet_info {
	/** packet start position (including SOP marker if it exists) */
	uint64_t start_pos;
	/** end of packet header position (including EPH marker if it exists)*/
	uint64_t end_ph_pos;
	/** packet end position */
	uint64_t end_pos;
	/** packet distortion */
	double disto;
} grk_packet_info;

/**
 * Marker info
 * */
typedef struct _grk_marker_info {
	/** marker type */
	uint16_t type;
	/** position in code stream */
	uint64_t pos;
	/** length, marker value included */
	uint32_t len;
} grk_marker_info;


/**
 * Tile info
 */
typedef struct _grk_tile_info {
	/** value of thresh for each layer by tile cfr. Marcela   */
	double *thresh;
	/** number of tile */
	uint16_t tileno;
	/** start position */
	uint64_t start_pos;
	/** end position of the header */
	uint64_t end_header;
	/** end position */
	uint64_t end_pos;
	/** precinct number for each resolution level (width) */
	uint32_t pw[GRK_J2K_MAXRLVLS];
	/** precinct number for each resolution level (height) */
	uint32_t ph[GRK_J2K_MAXRLVLS];
	/** precinct size (in power of 2), in X for each resolution level */
	uint32_t pdx[GRK_J2K_MAXRLVLS];
	/** precinct size (in power of 2), in Y for each resolution level */
	uint32_t pdy[GRK_J2K_MAXRLVLS];
	/** information concerning packets inside tile */
	grk_packet_info *packet;
	int64_t numpix;
	double distotile;
	/** number of markers */
	uint32_t marknum;
	/** list of markers */
	grk_marker_info *marker;
	/** actual size of markers array */
	uint32_t maxmarknum;
	/** number of tile parts */
	uint32_t num_tps;
} grk_tile_info;

/**
 * Code stream info
 */
typedef struct _grk_codestream_info {
	/** maximum distortion reduction on the whole image (add for Marcela) */
	double D_max;
	/** packet number */
	uint32_t packno;
	/** writing the packet in the index with t2_encode_packets */
	uint32_t index_write;
	/** component numbers */
	uint32_t numcomps;
	/** number of layer */
	uint32_t numlayers;
	/** number of decomposition for each component */
	uint32_t *numdecompos;
	/** number of markers */
	uint32_t marknum;
	/** list of markers */
	grk_marker_info *marker;
	/** actual size of markers array */
	uint32_t maxmarknum;
	/** main header position */
	uint64_t main_head_start;
	/** main header position */
	uint64_t main_head_end;
	/** code stream's size */
	uint64_t codestream_size;
	/** information regarding tiles inside image */
	grk_tile_info *tile;
} grk_codestream_info;

/**
 * Component coding parameters info
 */
typedef struct _grk_tccp_info {
	/** component index */
	uint32_t compno;
	/** coding style */
	uint8_t csty;
	/** number of resolutions */
	uint32_t numresolutions;
	/** code-blocks width */
	uint32_t cblkw;
	/** code-blocks height */
	uint32_t cblkh;
	/** code block mode */
	uint8_t cblk_sty;
	/** discrete wavelet transform identifier */
	uint8_t qmfbid;
	/** quantisation style */
	uint8_t qntsty;
	/** stepsizes used for quantization */
	uint32_t stepsizes_mant[GRK_J2K_MAXBANDS];
	/** stepsizes used for quantization */
	uint32_t stepsizes_expn[GRK_J2K_MAXBANDS];
	/** number of guard bits */
	uint8_t numgbits;
	/** Region Of Interest shift */
	uint32_t roishift;
	/** precinct width */
	uint32_t prcw[GRK_J2K_MAXRLVLS];
	/** precinct height */
	uint32_t prch[GRK_J2K_MAXRLVLS];
} grk_tccp_info;

/**
 * Tile coding parameter info
 */
typedef struct _grk_tile_v2_info {
	/** number (index) of tile */
	uint16_t tileno;
	/** coding style */
	uint32_t csty;
	/** progression order */
	GRK_PROG_ORDER prg;
	/** number of layers */
	uint16_t numlayers;
	/** multi-component transform identifier */
	uint32_t mct;
	/** information concerning tile component parameters*/
	grk_tccp_info *tccp_info;

} grk_tile_info_v2;

/**
 * Code stream info v2
 */
typedef struct _grk_codestream_info_v2 {
	/* Tile info */
	/** tile origin in x = XTOsiz */
	uint32_t tx0;
	/** tile origin in y = YTOsiz */
	uint32_t ty0;
	/** tile size in x = XTsiz */
	uint32_t t_width;
	/** tile size in y = YTsiz */
	uint32_t t_height;
	/** number of tiles in X */
	uint32_t t_grid_width;
	/** number of tiles in Y */
	uint32_t t_grid_height;
	/** number of components*/
	uint32_t nbcomps;
	/** Default information regarding tiles inside image */
	grk_tile_info_v2 m_default_tile_info;
} grk_codestream_info_v2;

/**
 * Tile part index info
 */
typedef struct _grk_tp_index {
	/** start position */
	uint64_t start_pos;
	/** end position of the header */
	uint64_t end_header;
	/** end position */
	uint64_t end_pos;
} grk_tp_index;

/**
 * Tile index info
 */
typedef struct _grk_tile_index {
	/** tile index */
	uint16_t tileno;
	/** number of tile parts */
	uint32_t nb_tps;
	/** current nb of tile part (allocated)*/
	uint32_t current_nb_tps;
	/** current tile-part index */
	uint32_t current_tpsno;
	/** information concerning tile parts */
	grk_tp_index *tp_index;
	/** number of markers */
	uint32_t marknum;
	/** list of markers */
	grk_marker_info *marker;
	/** actual size of markers array */
	uint32_t maxmarknum;
} grk_tile_index;

/**
 * Code stream index info
 */
typedef struct _grk_codestream_index {
	/** main header start position (SOC position) */
	uint64_t main_head_start;
	/** main header end position (first SOT position) */
	uint64_t main_head_end;
	/** code stream's size */
	uint64_t codestream_size;
	/** number of markers */
	uint32_t marknum;
	/** list of markers */
	grk_marker_info *marker;
	/** actual size of markers array */
	uint32_t maxmarknum;
	uint32_t nb_of_tiles;
	grk_tile_index *tile_index;
} grk_codestream_index;

////////////////////////////////////////////////
// Structs to pass data between grok and plugin
/////////////////////////////////////////////////

/**
 * Plugin pass
 */
typedef struct _grk_plugin_pass {
	double distortionDecrease; //distortion decrease up to and including this pass
	size_t rate;    // rate up to and including this pass
	size_t length;	//stream length for this pass
} grk_plugin_pass;

/**
 * Plugin code block
 */
typedef struct _grk_plugin_code_block {
	/////////////////////////
	// debug info
	uint32_t x0, y0, x1, y1;
	unsigned int *contextStream;
	///////////////////////////
	uint32_t numPix;
	uint8_t *compressedData;
	uint32_t compressedDataLength;
	size_t numBitPlanes;
	size_t numPasses;
	grk_plugin_pass passes[67];
	unsigned int sortedIndex;
} grk_plugin_code_block;

/**
 * Plugin precinct
 */
typedef struct _grk_plugin_precinct {
	uint64_t numBlocks;
	grk_plugin_code_block **blocks;
} grk_plugin_precinct;

/**
 * Plugin band
 */
typedef struct _grk_plugin_band {
	size_t orient;
	uint64_t numPrecincts;
	grk_plugin_precinct **precincts;
	float stepsize;
} grk_plugin_band;

/**
 * Plugin resolution
 */
typedef struct _grk_plugin_resolution {
	size_t level;
	size_t numBands;
	grk_plugin_band **bands;
} grk_plugin_resolution;

/**
 * Plugin tile component
 */
typedef struct grk_plugin_tile_component {
	size_t numResolutions;
	grk_plugin_resolution **resolutions;
} grk_plugin_tile_component;

#define GRK_DECODE_HEADER	(1 << 0)
#define GRK_DECODE_T2		(1 << 1)
#define GRK_DECODE_T1		(1 << 2)
#define GRK_DECODE_POST_T1	(1 << 3)
#define GRK_PLUGIN_DECODE_CLEAN  (1 << 4)
#define GRK_DECODE_ALL		(GRK_PLUGIN_DECODE_CLEAN | GRK_DECODE_HEADER | GRK_DECODE_T2 | GRK_DECODE_T1 | GRK_DECODE_POST_T1)

/**
 * Plugin tile
 */
typedef struct _grk_plugin_tile {
	uint32_t decode_flags;
	size_t numComponents;
	grk_plugin_tile_component **tileComponents;
} grk_plugin_tile;

/**
 * library version
 */
GRK_API const char* GRK_CALLCONV grk_version(void);

/**
 * Initialize library
 *
 * @param plugin_path 	path to plugin
 * @param numthreads 	number of threads to use for compress/decompress
 */
GRK_API bool GRK_CALLCONV grk_initialize(const char *plugin_path,
		uint32_t numthreads);

/**
 * De-initialize library
 */
GRK_API void GRK_CALLCONV grk_deinitialize();

/*
 ============================
 image function definitions
 ============================
 */

/**
 * Create image
 *
 * @param numcmpts      number of components
 * @param cmptparms     component parameters
 * @param clrspc        image color space
 * @param allocData		true if data is to be allocated, otherwise false
 *
 * @return returns      a new image if successful, otherwise nullptr
 * */
GRK_API grk_image* GRK_CALLCONV grk_image_create(uint32_t numcmpts,
		grk_image_cmptparm *cmptparms, GRK_COLOR_SPACE clrspc, bool allocData);

/**
 * Deallocate all resources associated with an image
 *
 * @param image         image
 */
GRK_API void GRK_CALLCONV grk_image_destroy(grk_image *image);


/**
 * Deallocate all component data for an image
 *
 * @param image         image
 */
GRK_API void GRK_CALLCONV grk_image_all_components_data_free(grk_image *image);

/**
 * Deallocate data for single image component
 *
 * @param image         image
 */
GRK_API void GRK_CALLCONV grk_image_single_component_data_free(
		grk_image_comp *image);

/*
 =================================
 stream function definitions
 =================================
 */

/**
 * Create an abstract stream. This function does nothing except
 * allocate memory and initialize abstract stream.
 *
 * @param	buffer_size   size of stream's double-buffer
 * @param	is_input	  if set to true then the stream will be
 *  an input stream, an output stream else.
 *
 * @return	stream object.
 */
GRK_API grk_stream* GRK_CALLCONV grk_stream_create(size_t buffer_size,
		bool is_input);

/**
 * Destroy a stream created by grk_create_stream. This function does NOT
 * close the abstract stream. If needed the user must
 * close their own implementation of the stream.
 *
 * @param	stream	the stream to destroy.
 */
GRK_API void GRK_CALLCONV grk_stream_destroy(grk_stream *stream);

/**
 * Set the given function to be used as a read function.
 *
 * @param		stream	the stream to modify
 * @param		p_function	the function to use a read function.
 */
GRK_API void GRK_CALLCONV grk_stream_set_read_function(grk_stream *stream,
		grk_stream_read_fn p_function);

/**
 * Set the given function to be used as a write function.
 *
 * @param		stream	the stream to modify
 * @param		p_function	the function to use a write function.
 */
GRK_API void GRK_CALLCONV grk_stream_set_write_function(grk_stream *stream,
		grk_stream_write_fn p_function);

/**
 * Set the given function to be used as a seek function, the stream is then seekable.
 *
 * @param		stream	the stream to modify
 * @param		p_function	the function to use a skip function.
 */
GRK_API void GRK_CALLCONV grk_stream_set_seek_function(grk_stream *stream,
		grk_stream_seek_fn p_function);

/**
 * Set the given data to be used as a user data for the stream.
 *
 * @param		stream	the stream to modify
 * @param		data		the data to set.
 * @param		p_function	the function to free data when grk_stream_destroy() is called.
 */
GRK_API void GRK_CALLCONV grk_stream_set_user_data(grk_stream *stream,
		void *data, grk_stream_free_user_data_fn p_function);

/**
 * Set the length of the user data for the stream.
 *
 * @param stream    the stream to modify
 * @param data_length length of the user_data.
 */
GRK_API void GRK_CALLCONV grk_stream_set_user_data_length(grk_stream *stream,
		uint64_t data_length);

/** Create stream from a file identified with its filename with a specific buffer size
 *
 * @param fname             the filename of the file to stream
 * @param buffer_size     size of the chunk used to stream
 * @param is_read_stream  whether the stream is a read stream (true) or not (false)
 */
GRK_API grk_stream* GRK_CALLCONV grk_stream_create_file_stream(
		const char *fname, size_t buffer_size, bool is_read_stream);

/** Create stream from buffer
 *
 * @param buf			buffer
 * @param buffer_len    length of buffer
 * @param ownsBuffer	if true, library will delete[] buffer. Otherwise, it is the caller's
 *						responsibility to delete the buffer
 * @param is_read_stream  whether the stream is a read stream (true) or not (false)
 */
GRK_API grk_stream* GRK_CALLCONV grk_stream_create_mem_stream(uint8_t *buf,
		size_t buffer_len, bool ownsBuffer, bool is_read_stream);

/**
 * Get length of memory stream
 *
 * @param stream memory stream
 */
GRK_API size_t GRK_CALLCONV grk_stream_get_write_mem_stream_length(
		grk_stream *stream);

/**
 * Create mapped file stream
 *
 * @param fname			file name
 * @param read_stream 	true if this is a read stream, otherwise false
 */
GRK_API grk_stream* GRK_CALLCONV grk_stream_create_mapped_file_stream(
		const char *fname, bool read_stream);

/*
 ========================================
 logger function definitions
 ========================================
 */
/**
 * Set info handler
 *
 * @param p_callback    the callback function which will be used
 * @param user_data   client object where will be returned the message
 */
GRK_API bool GRK_CALLCONV grk_set_info_handler(grk_msg_callback p_callback,
		void *user_data);
/**
 * Set warning handler
 *
 * @param p_callback    the callback function which will be used
 * @param user_data   client object where will be returned the message
 */
GRK_API bool GRK_CALLCONV grk_set_warning_handler(grk_msg_callback p_callback,
		void *user_data);
/**
 * Set error handler
 *
 * @param p_callback    the callback function which will be used
 * @param user_data   client object where will be returned the message
 */
GRK_API bool GRK_CALLCONV grk_set_error_handler(grk_msg_callback p_callback,
		void *user_data);


/**
 * Create J2K/JP2 decompression structure
 *
 * @param format 		JPEG 2000 format
 * @param	stream	JPEG 2000 stream.
 *
 * @return a handle to a decompressor if successful, otherwise nullptr
 * */
GRK_API grk_codec GRK_CALLCONV grk_create_decompress(GRK_CODEC_FORMAT format,
		grk_stream *stream);

/**
 * Initialize decompress parameters with default values
 *
 * @param parameters decompression parameters
 */
GRK_API void GRK_CALLCONV grk_set_default_decompress_params(
		grk_dparameters *parameters);

/**
 * Set up the decompressor with decompress parameters
 *
 * @param codec 		decompressor handler
 * @param parameters 	decompression parameters
 *
 * @return true			if the decoder is correctly set
 */
GRK_API bool GRK_CALLCONV grk_init_decompress(grk_codec codec,
		grk_dparameters *parameters);

/**
 * Decode JPEG 2000 header
 *
 * @param	codec				JPEG 2000 code stream to read.
 * @param	header_info			information read from JPEG 2000 header.
 * @param	image				the image structure initialized with the characteristics
 *								of encoded image.
 * @return true					if the main header of the code stream and the JP2 header
 * 							 	is correctly read.
 */
GRK_API bool GRK_CALLCONV grk_read_header(grk_codec codec,
		grk_header_info *header_info, grk_image **image);

/**
 * Set the given area to be decompressed. This function should be called
 *  right after grk_read_header is called, and before any tile header is read.
 *
 * @param	codec			JPEG 2000 code stream.
 * @param	image         	image created by grk_read_header
 * @param	start_x		    left position of the rectangle to decompress (in image coordinates).
 * @param	end_x			the right position of the rectangle to decompress (in image coordinates).
 * @param	start_y		    up position of the rectangle to decompress (in image coordinates).
 * @param	end_y			bottom position of the rectangle to decompress (in image coordinates).
 *
 * @return	true			if the area could be set.
 */
GRK_API bool GRK_CALLCONV grk_set_decompress_area(grk_codec codec,
		grk_image *image, uint32_t start_x, uint32_t start_y, uint32_t end_x,
		uint32_t end_y);

/**
 * Decompress image from a JPEG 2000 code stream
 *
 * @param p_decompressor 	decompressor handle
 * @param tile			 	tile struct from plugin
 * @param image 			the decoded image
 * @return 					true if success, otherwise false
 * */
GRK_API bool GRK_CALLCONV grk_decompress(grk_codec p_decompressor,
		grk_plugin_tile *tile, grk_image *image);

/**
 * Decompress a specific tile
 *
 * @param	codec			JPEG 2000 code stream
 * @param	image			output image
 * @param	tile_index		index of the tile to be decompressed
 *
 * @return					true if success, otherwise false
 */
GRK_API bool GRK_CALLCONV grk_decompress_tile(grk_codec codec,
		grk_image *image, uint16_t tile_index);

/**
 * End decompression
 *
 * @param	codec			JPEG 2000 code stream
 */
GRK_API bool GRK_CALLCONV grk_end_decompress(grk_codec codec);


/* COMPRESSION FUNCTIONS*/

/**
 * Creates a J2K/JP2 compression structure
 * @param 	format 		Coder to select
 * @param	stream	the JPEG 2000 stream.
 * @return 				Returns a handle to a compressor if successful,
 * 						returns nullptr otherwise
 */
GRK_API grk_codec GRK_CALLCONV grk_create_compress(GRK_CODEC_FORMAT format,
		grk_stream *stream);

/**
 Set encoding parameters to default values, that means :

 Lossless
 Single tile
 Size of precinct : 2^15 x 2^15 (i.e. single precinct)
 Size of code block : 64 x 64
 Number of resolutions: 6
 No SOP marker in the code stream
 No EPH marker in the code stream
 No sub-sampling in x or y direction
 No mode switch activated
 Progression order: LRCP
 No index file
 No ROI upshifted
 Image origin lies at (0,0)
 Tile origin lies at (0,0)
 Reversible DWT 5-3 transform

 @param parameters Compression parameters
 */
GRK_API void GRK_CALLCONV grk_set_default_compress_params(
		grk_cparameters *parameters);

/**
 * Set up the encoder parameters using the current image and user parameters.
 *
 * @param codec 		JPEG 2000 code stream
 * @param parameters 	compression parameters
 * @param image 		input image
 */
GRK_API bool GRK_CALLCONV grk_init_compress(grk_codec codec,
		grk_cparameters *parameters, grk_image *image);

/**
 * Start compressing current image.
 *
 * @param codec 		Compressor handle
 *
 */
GRK_API bool GRK_CALLCONV grk_start_compress(grk_codec codec);

/**
 * Encode an image into a JPEG 2000 code stream
 * @param codec 		compressor handle
 *
 * @return 				Returns true if successful, returns false otherwise
 */
GRK_API bool GRK_CALLCONV grk_compress(grk_codec codec);

/**
 * Compress uncompressed data stored in a buffer.
 * This method should be called right after grk_start_compress,
 * and before grk_end_compress.
 *
 * @param	codec		    JPEG 2000 code stream
 * @param	tile_index		the index of the tile to write. At the moment,
 * 							the tiles must be written from 0 to n-1 in sequence.
 * @param	data			pointer to the data to write. Data is arranged in planar
 *  						sequence, data_comp0, data_comp1 etc,
 *  						The data should NOT BE INTERLEAVED.
 * @param	data_size		this value is used to ensure the data
 * 							being written is correct. The size must be
 * 							equal to the sum for each component of
 *                          tile_width * tile_height * component_size.
 *                          component_size can be 1 or 2 bytes, depending on
 *                          the precision of the given component.
 *
 * @return	true if the data could be written.
 */
GRK_API bool GRK_CALLCONV grk_compress_tile(grk_codec codec,
		uint16_t tile_index, uint8_t *data, uint64_t data_size);


/**
 * Encode an image into a JPEG 2000 code stream using plugin
 * @param codec 		compressor handle
 * @param tile			plugin tile
 *
 * @return 				Returns true if successful, returns false otherwise
 */
GRK_API bool GRK_CALLCONV grk_compress_with_plugin(grk_codec codec,
		grk_plugin_tile *tile);


/**
 * End to compress the current image.
 * @param codec 		Compressor handle
 */
GRK_API bool GRK_CALLCONV grk_end_compress(grk_codec codec);


/**
 * Destroy codec
 *
 * @param	codec			JPEG 2000 code stream
 */
GRK_API void GRK_CALLCONV grk_destroy_codec(grk_codec codec);


/**
 Destroy Codestream information after compression or decompression
 @param cstr_info Codestream information structure
 */
GRK_API void GRK_CALLCONV grk_destroy_cstr_info(
		grk_codestream_info_v2 **cstr_info);

/**
 * Dump codec information to file
 *
 * @param	codec			JPEG 2000 code stream.
 * @param	info_flag		type of information dump.
 * @param	output_stream	output stream where dump the information get from the codec.
 *
 */
GRK_API void GRK_CALLCONV grk_dump_codec(grk_codec codec, uint32_t info_flag,
		FILE *output_stream);

/**
 * Get code stream information from codec
 *
 * @param	codec			JPEG 2000 code stream
 *
 * @return					pointer to a code stream information structure.
 *
 */
GRK_API grk_codestream_info_v2* GRK_CALLCONV grk_get_cstr_info(
		grk_codec codec);

/**
 * Get the code stream index from the codec
 *
 * @param	codec			JPEG 2000 code stream.
 *
 * @return					pointer to a code stream index structure.
 *
 */
GRK_API grk_codestream_index* GRK_CALLCONV grk_get_cstr_index(grk_codec codec);

/**
 * Destroy code stream index
 *
 * @param p_cstr_index		code stream index
 */
GRK_API void GRK_CALLCONV grk_destroy_cstr_index(
		grk_codestream_index **p_cstr_index);

/**
 * Set the MCT matrix to use.
 *
 * @param	parameters		the parameters to change.
 * @param	pEncodingMatrix	the encoding matrix.
 * @param	p_dc_shift		the dc shift coefficients to use.
 * @param	pNbComp			the number of components of the image.
 *
 * @return	true if the parameters could be set.
 */
GRK_API bool GRK_CALLCONV grk_set_MCT(grk_cparameters *parameters,
		float *pEncodingMatrix, int32_t *p_dc_shift, uint32_t pNbComp);

/*****************
 Plugin Interface
 ******************/

/*
 Plugin management
 */

typedef struct _grk_plugin_load_info {
	const char *plugin_path;
} grk_plugin_load_info;

/**
 * Load plugin
 *
 * @param info		plugin loading info
 */
GRK_API bool GRK_CALLCONV grk_plugin_load(grk_plugin_load_info info);

/**
 * Release plugin resources
 */
GRK_API void GRK_CALLCONV grk_plugin_cleanup(void);

// No debug is done on plugin. Production setting.
#define GRK_PLUGIN_STATE_NO_DEBUG			0x0

/*
 For compress debugging, the plugin first performs a T1 compress.
 Then:
 1. perform host DWT on plugin MCT data, and write to host image
 This way, both plugin and host start from same point
 (assume MCT is equivalent for both host and plugin)
 2. map plugin DWT data, compare with host DWT, and then write to plugin image
 At this point in the code, the plugin image holds plugin DWT data. And if no warnings are triggered,
 then we can safely say that host and plugin DWT data are identical.
 3. Perform host compress, skipping MCT and DWT (they have already been performed)
 4. during host compress, each context that is formed is compared against context stream from plugin
 5. rate control - synch with plugin code stream, and compare
 6. T2 and store to disk
 */

#define GRK_PLUGIN_STATE_DEBUG				0x1
#define GRK_PLUGIN_STATE_PRE_TR1			0x2
#define GRK_PLUGIN_STATE_DWT_QUANTIZATION	0x4
#define GRK_PLUGIN_STATE_MCT_ONLY			0x8

/**
 * Get debug state of plugin
 */
GRK_API uint32_t GRK_CALLCONV grk_plugin_get_debug_state();

/*
 Plugin encoding
 */
typedef struct _grk_plugin_init_info {
	int32_t deviceId;
	bool verbose;
} grk_plugin_init_info;

/**
 * Initialize plugin
 */
GRK_API bool GRK_CALLCONV grk_plugin_init(grk_plugin_init_info initInfo);

typedef struct grk_plugin_encode_user_callback_info {
	const char *input_file_name;
	bool outputFileNameIsRelative;
	const char *output_file_name;
	grk_cparameters *encoder_parameters;
	grk_image *image;
	grk_plugin_tile *tile;
	uint8_t *compressBuffer;
	size_t compressBufferLen;
	unsigned int error_code;
} grk_plugin_encode_user_callback_info;

typedef bool (*GRK_PLUGIN_ENCODE_USER_CALLBACK)(
		grk_plugin_encode_user_callback_info *info);

/**
 * Compress with plugin
 *
 * @param encode_parameters 	compress parameters
 * @param callback				callback
 */
GRK_API int32_t GRK_CALLCONV grk_plugin_compress(
		grk_cparameters *encode_parameters,
		GRK_PLUGIN_ENCODE_USER_CALLBACK callback);

/**
 * Batch compress with plugin
 *
 * @param input_dir				directory holding input images
 * @param output_dir			directory holding compressed output images
 * @param encode_parameters 	compress parameters
 * @param callback				callback
 *
 * @return 0 if successful
 *
 */
GRK_API int32_t GRK_CALLCONV grk_plugin_batch_compress(const char *input_dir,
		const char *output_dir, grk_cparameters *encode_parameters,
		GRK_PLUGIN_ENCODE_USER_CALLBACK callback);

/**
 * Check if batch job is complete
 */
GRK_API bool GRK_CALLCONV grk_plugin_is_batch_complete(void);

/**
 * Stop batch compress
 */
GRK_API void GRK_CALLCONV grk_plugin_stop_batch_compress(void);

/*
 Plugin decompression
 */

typedef int (*GROK_INIT_DECODERS)(grk_header_info *header_info,
		grk_image *image);

typedef struct _grk_plugin_decode_callback_info {
	size_t deviceId;
	GROK_INIT_DECODERS init_decoders_func;
	const char *input_file_name;
	const char *output_file_name;
	// input file format 0: J2K, 1: JP2
	GRK_SUPPORTED_FILE_FMT decod_format;
	// output file format 0: PGX, 1: PxM, 2: BMP etc 
	GRK_SUPPORTED_FILE_FMT cod_format;
	grk_stream *l_stream;
	grk_codec l_codec;
	grk_header_info header_info;
	grk_decompress_parameters *decoder_parameters;
	grk_image *image;
	bool plugin_owns_image;
	grk_plugin_tile *tile;
	unsigned int error_code;
	uint32_t decode_flags;
	uint32_t full_image_x0;
	uint32_t full_image_y0;
	void* user_data;
} grk_plugin_decode_callback_info;

typedef int32_t (*grk_plugin_decode_callback)(
		grk_plugin_decode_callback_info *info);

/**
 * Decompress with plugin
 *
 * @param decompress_parameters  decompress parameters
 * @param callback  			 callback
 */
GRK_API int32_t GRK_CALLCONV grk_plugin_decompress(
		grk_decompress_parameters *decompress_parameters,
		grk_plugin_decode_callback callback);

/**
 * Initialize batch decompress
 *
 * @param input_dir input directory holding compressed images
 * @param output_dir output directory holding decompressed images
 * @param decompress_parameters  decompress parameters
 * @param callback  			 callback
 *
 * @return 0 if successful
 */
GRK_API int32_t GRK_CALLCONV grk_plugin_init_batch_decompress(
		const char *input_dir, const char *output_dir,
		grk_decompress_parameters *decompress_parameters,
		grk_plugin_decode_callback callback);

/**
 * Batch decompress
 */
GRK_API int32_t GRK_CALLCONV grk_plugin_batch_decompress(void);

/**
 * Stop batch decompress
 */
GRK_API void GRK_CALLCONV grk_plugin_stop_batch_decompress(void);

#ifdef __cplusplus
}
#endif

