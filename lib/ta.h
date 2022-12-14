/* ta.h */

/*
 * Copyright (C) 2011-2019 by Werner Lemberg.
 *
 * This file is part of the ttfautohint library, and may only be used,
 * modified, and distributed under the terms given in `COPYING'.  By
 * continuing to use, modify, or distribute this file you indicate that you
 * have read `COPYING' and understand and accept it fully.
 *
 * The file `COPYING' mentioned in the previous paragraph is distributed
 * with the ttfautohint library.
 */


#ifndef TA_H_
#define TA_H_

#include <config.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#include <ttfautohint.h>
#include <sds.h>
#include <numberset.h>

#include "taloader.h"
#include "taglobal.h"
#include "tadummy.h"
#include "talatin.h"


#define TTFAUTOHINT_GLYPH ".ttfautohint"
#define TTFAUTOHINT_GLYPH_FIRST_BYTE "\x0C" /* first byte is string length */
#define TTFAUTOHINT_GLYPH_LEN 13


/* these macros convert 16bit and 32bit numbers into single bytes */
/* using the byte order needed within SFNT files */

#define HIGH(x) (FT_Byte)(((x) & 0xFF00) >> 8)
#define LOW(x) ((x) & 0x00FF)

#define BYTE1(x) (FT_Byte)(((FT_ULong)(x) & 0xFF000000UL) >> 24);
#define BYTE2(x) (FT_Byte)(((FT_ULong)(x) & 0x00FF0000UL) >> 16);
#define BYTE3(x) (FT_Byte)(((FT_ULong)(x) & 0x0000FF00UL) >> 8);
#define BYTE4(x) ((FT_ULong)(x) & 0x000000FFUL);

/* utility macros to get data from a pointer (with auto-increment) */

#define NEXT_USHORT(buffer) \
          ((FT_UShort)(buffer += 2, (FT_UShort)buffer[-2] << 8 \
                                    | (FT_UShort)buffer[-1]))
#define NEXT_ULONG(buffer) \
          ((FT_ULong)(buffer += 4, (FT_ULong)buffer[-4] << 24 \
                                   | (FT_ULong)buffer[-3] << 16 \
                                   | (FT_ULong)buffer[-2] << 8 \
                                   | (FT_ULong)buffer[-1]))


/* an SFNT tag for our information table */
#define TTAG_TTFA FT_MAKE_TAG('T', 'T', 'F', 'A')

/* the length of a dummy `DSIG' table */
#define DSIG_LEN 8

/* the length of our `gasp' table */
#define GASP_LEN 8

/* an empty slot in the table info array */
#define MISSING ~0U

/* the offset to the loca table format in the `head' table */
#define LOCA_FORMAT_OFFSET 51

/* various offsets within the `maxp' table */
#define MAXP_NUM_GLYPHS 4
#define MAXP_MAX_COMPOSITE_POINTS 10
#define MAXP_MAX_COMPOSITE_CONTOURS 12
#define MAXP_MAX_ZONES_OFFSET 14
#define MAXP_MAX_TWILIGHT_POINTS_OFFSET 16
#define MAXP_MAX_STORAGE_OFFSET 18
#define MAXP_MAX_FUNCTION_DEFS_OFFSET 20
#define MAXP_MAX_INSTRUCTION_DEFS_OFFSET 22
#define MAXP_MAX_STACK_ELEMENTS_OFFSET 24
#define MAXP_MAX_INSTRUCTIONS_OFFSET 26
#define MAXP_MAX_COMPONENTS_OFFSET 28

#define MAXP_LEN 32

/* the offset of the type flags field in the `OS/2' table */
#define OS2_FSTYPE_OFFSET 8


/* flags in composite glyph records */
#define ARGS_ARE_WORDS 0x0001
#define ARGS_ARE_XY_VALUES 0x0002
#define WE_HAVE_A_SCALE 0x0008
#define MORE_COMPONENTS 0x0020
#define WE_HAVE_AN_XY_SCALE 0x0040
#define WE_HAVE_A_2X2 0x0080
#define WE_HAVE_INSTR 0x0100

/* flags in simple glyph records */
#define ON_CURVE 0x01
#define X_SHORT_VECTOR 0x02
#define Y_SHORT_VECTOR 0x04
#define REPEAT 0x08
#define SAME_X 0x10
#define SAME_Y 0x20


/* a single glyph */
typedef struct GLYPH_
{
  FT_ULong len1; /* number of bytes before instruction related data */
  FT_ULong len2; /* number of bytes after instruction related data; */
                 /* if zero, this indicates a composite glyph */
  FT_Byte* buf; /* extracted glyph data (without instruction related data) */
  FT_ULong flags_offset; /* offset to last flag in a composite glyph */

  FT_Byte ins_extra_len; /* number of extra instructions */
  FT_Byte* ins_extra_buf; /* extra instructions data */
  FT_ULong ins_len; /* number of new instructions */
  FT_Byte* ins_buf; /* new instruction data */

  FT_Short num_contours; /* >= 0 for simple glyphs */
  FT_UShort num_points; /* number of points in a simple glyph */

  FT_UShort num_components;
  FT_UShort* components; /* the subglyph indices of a composite glyph */

  FT_UShort num_pointsums;
  FT_UShort* pointsums; /* the pointsums of all composite elements */
                        /* (after walking recursively over all subglyphs) */

  FT_UShort num_composite_contours; /* after recursion */
} GLYPH;

/* a representation of the data in the `glyf' table */
typedef struct glyf_Data_
{
  FT_UShort num_glyphs;
  GLYPH* glyphs;

  /* this field gives the `master' globals for a `glyf' table; */
  /* see function `TA_sfnt_handle_coverage' */
  TA_FaceGlobals master_globals;
  /* for coverage bookkeeping */
  FT_Bool adjusted;

  /* if a `glyf' table gets used in more than one subfont, */
  /* so do `cvt', `fpgm', and `prep' tables: */
  /* these four tables are always handled in parallel */
  FT_ULong cvt_idx;
  FT_ULong fpgm_idx;
  FT_ULong prep_idx;

  /* styles present in a font get a running number; */
  /* unavailable styles get value 0xFFFF */
  FT_UInt style_ids[TA_STYLE_MAX];
  FT_UInt num_used_styles;

  /* we have separate CVT data for each style */
  FT_UInt cvt_offsets[TA_STYLE_MAX];
  FT_UInt cvt_horz_width_sizes[TA_STYLE_MAX];
  FT_UInt cvt_vert_width_sizes[TA_STYLE_MAX];
  FT_UInt cvt_blue_zone_sizes[TA_STYLE_MAX];
  FT_UInt cvt_blue_adjustment_offsets[TA_STYLE_MAX];
} glyf_Data;

/* an SFNT table */
typedef struct SFNT_Table_
{
  FT_ULong tag;
  FT_ULong len;
  FT_Byte* buf; /* the table data */
  FT_ULong offset; /* from beginning of file */
  FT_ULong checksum;
  void* data; /* used e.g. for `glyf' table data */
  FT_Bool processed;
} SFNT_Table;

/* we use indices into the SFNT table array to */
/* represent table info records of the TTF header */
typedef FT_ULong SFNT_Table_Info;

/* this structure is used to model a TTF or a subfont within a TTC */
typedef struct SFNT_
{
  FT_Face face;

  SFNT_Table_Info* table_infos;
  FT_ULong num_table_infos;

  /* various SFNT table indices */
  FT_ULong glyf_idx;
  FT_ULong loca_idx;
  FT_ULong head_idx;
  FT_ULong hmtx_idx;
  FT_ULong maxp_idx;
  FT_ULong name_idx;
  FT_ULong post_idx;
  FT_ULong OS2_idx;
  FT_ULong GPOS_idx;

  /* values necessary to update the `maxp' table */
  FT_UShort max_composite_points;
  FT_UShort max_composite_contours;
  FT_UShort max_storage;
  FT_UShort max_stack_elements;
  FT_UShort max_twilight_points;
  FT_UShort max_instructions;
  FT_UShort max_components;
} SFNT;

typedef struct Control_ Control;

/* our font object; the `FONT' typedef is in `taloader.h' */
struct FONT_
{
  FT_Library lib;

  FT_Byte* in_buf;
  size_t in_len;

  FT_Byte* out_buf;
  size_t out_len;

  char* control_buf;
  size_t control_len;

  FT_Byte* reference_buf;
  size_t reference_len;

  FT_Face reference;
  FT_Long reference_index;
  const char* reference_name;

  SFNT* sfnts;
  FT_Long num_sfnts;

  SFNT_Table* tables;
  FT_ULong num_tables;

  FT_Bool have_DSIG;

  /* we have a single `gasp' table for all subfonts */
  FT_ULong gasp_idx;

  /* the control instructions */
  Control* control;

  /* two generic pointers into the control instructions tree */
  void* control_data_head;
  void* control_data_cur;

  /* two fields for handling one-point segment directions */
  /* of the current glyph */
  void* control_segment_dirs_head;
  void* control_segment_dirs_cur;

  TA_LoaderRec loader[1]; /* the interface to the autohinter */

  /* configuration options */
  TA_Progress_Func progress;
  void* progress_data;
  TA_Info_Func info;
  TA_Info_Post_Func info_post;
  void* info_data;
  TA_Alloc_Func allocate;
  TA_Free_Func deallocate;
  FT_UInt hinting_range_min;
  FT_UInt hinting_range_max;
  FT_UInt hinting_limit;
  FT_UInt increase_x_height;
  number_range* x_height_snapping_exceptions;
  FT_UInt fallback_stem_width;
  FT_Int gray_stem_width_mode;
  FT_Int gdi_cleartype_stem_width_mode;
  FT_Int dw_cleartype_stem_width_mode;
  FT_Bool windows_compatibility;
  FT_Bool adjust_subglyphs;
  FT_Bool hint_composites;
  FT_Bool ignore_restrictions;
  TA_Style fallback_style;
  FT_Bool fallback_scaling;
  TA_Script default_script;
  FT_Bool symbol;
  FT_Bool dehint;
  FT_Bool debug;
  FT_Bool TTFA_info;
  unsigned long long epoch;
};


#include "tatables.h"
#include "tabytecode.h"
#include "tacontrol.h"


/* in file `tascript.c' */
extern const char* script_names[];
extern size_t script_names_size;


const char*
TA_get_error_message(FT_Error error);

char*
TA_font_dump_parameters(FONT* font,
                        FT_Bool format);

void
TA_get_current_time(FONT* font,
                    FT_ULong* high,
                    FT_ULong* low);

FT_Byte*
TA_build_push(FT_Byte* bufp,
              FT_UInt* args,
              FT_UInt num_args,
              FT_Bool need_words,
              FT_Bool optimize);

FT_Error
TA_font_init(FONT* font);
void
TA_font_unload(FONT* font,
               const char* in_buf,
               char** out_bufp,
               const char* control_buf,
               const char* reference_buf);

FT_Error
TA_font_file_read(FILE* file,
                  FT_Byte** buffer,
                  size_t* length);
FT_Error
TA_font_file_write(FONT* font,
                   FILE* out_file);
FT_Error
TA_control_file_read(FONT* font,
                     FILE* control_file);

FT_Error
TA_sfnt_compute_composite_pointsums(SFNT* sfnt,
                                    FONT* font);

FT_Error
TA_sfnt_build_glyph_instructions(SFNT* sfnt,
                                 FONT* font,
                                 FT_Long idx);

FT_Error
TA_sfnt_split_into_SFNT_tables(SFNT* sfnt,
                               FONT* font);

FT_Error
TA_sfnt_build_cvt_table(SFNT* sfnt,
                        FONT* font);

FT_Error
TA_table_build_TTFA(FT_Byte** TTFA,
                    FT_ULong* TTFA_len,
                    FONT* font);

FT_Error
TA_table_build_DSIG(FT_Byte** DSIG);

FT_Error
TA_sfnt_build_fpgm_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_build_gasp_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_split_glyf_table(SFNT* sfnt,
                         FONT* font);
FT_Error
TA_sfnt_build_glyf_table(SFNT* sfnt,
                         FONT* font);
FT_Error
TA_sfnt_create_glyf_data(SFNT* sfnt,
                         FONT* font);
FT_Error
TA_sfnt_handle_coverage(SFNT* sfnt,
                        FONT* font);
void
TA_sfnt_adjust_coverage(SFNT* sfnt,
                        FONT* font);
#if 0
void
TA_sfnt_copy_master_coverage(SFNT* sfnt,
                             FONT* font);
#endif

FT_Error
TA_sfnt_update_GPOS_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_update_hmtx_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_build_loca_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_update_maxp_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_update_post_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_update_name_table(SFNT* sfnt,
                          FONT* font);

FT_Error
TA_sfnt_build_prep_table(SFNT* sfnt,
                         FONT* font);

FT_Error
TA_sfnt_build_TTF_header(SFNT* sfnt,
                         FONT* font,
                         FT_Byte** header_buf,
                         FT_ULong* header_len,
                         FT_Int do_complete);
FT_Error
TA_font_build_TTF(FONT* font);

FT_Error
TA_font_build_TTC(FONT* font);

#endif /* TA_H_ */

/* end of ta.h */
