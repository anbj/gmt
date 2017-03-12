/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2017 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* x2sys_list will read the crossover data base and output a subset of
 * the crossovers in a format determined by the options.
 *
 * Author:	Paul Wessel
 * Date:	20-SEPT-2008
 * Version:	1.0, based on the spirit of the old x_system code x_list
 *		but completely rewritten from the ground up.
 *
 */

#define THIS_MODULE_NAME	"x2sys_list"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Extract subset from crossover data base"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""

#include "x2sys.h"

#define GMT_PROG_OPTIONS "->RV"

#define GMT_T	3	/* Just used to indicate abs time formatting */
#define LETTERS "acdhiInNtTvwxyz"

struct X2SYS_LIST_CTRL {
	struct X2SYS_LIST_In {
		bool active;
		char *file;
	} In;
	struct X2SYS_LIST_A {	/* -A */
		bool active;
		double value;
	} A;
	struct X2SYS_LIST_C {	/* -C */
		bool active;
		char *col;
	} C;
	struct X2SYS_LIST_E {	/* -E */
		bool active;
	} E;
	struct X2SYS_LIST_F {	/* -F */
		bool active;
		char *flags;
	} F;
	struct X2SYS_LIST_I {	/* -I */
		bool active;
		char *file;
	} I;
	struct X2SYS_LIST_L {	/* -L */
		bool active;
		char *file;
	} L;
	struct X2SYS_LIST_N {	/* -N */
		bool active;
		unsigned int min;
	} N;
	struct X2SYS_LIST_Q {	/* -Q */
		bool active;
		int mode;
	} Q;
	struct X2SYS_LIST_S {	/* -S */
		bool active;
		bool both;
		char *file;
	} S;
	struct X2SYS_LIST_T {	/* -T */
		bool active;
		char *TAG;
	} T;
	struct X2SYS_LIST_W {	/* -W */
		bool active;
		char *file;
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_LIST_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct X2SYS_LIST_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.value = 1.0;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_LIST_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.col);
	gmt_M_str_free (C->F.flags);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->L.file);
	gmt_M_str_free (C->S.file);
	gmt_M_str_free (C->T.TAG);
	gmt_M_str_free (C->W.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: x2sys_list -C<column> -T<TAG> [<COEdbase>] [-A<asymm_max] [-E] [-F<flags>] [-I<ignorelist>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-L[<corrtable.txt>]] [-N<nx_min>] [-Qe|i] [-S[+]<track>]\n\t[%s] [%s] [-W<weight>] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_bo_OPT, GMT_do_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-C <column> is the name of the data column whose crossovers we want.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the system tag for the data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<COEdbase> File with crossover error data base [stdin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Return only crossovers whose distribution in time [or dist if no time]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   are fairly symmetric about the mid-point. Specify max abs value for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   asymmetry = (n_right - n_left)/(n_right + n_left) [1, i.e., use all tracks].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Enhanced ASCII output: Add segment header with track names and number of crossovers [no segment headers].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify any combination of %s in the order of desired output:\n", LETTERS);
	GMT_Message (API, GMT_TIME_NONE, "\t   a Angle (<= 90) between the two tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c Crossover error in chosen observable (see -C).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d Distance along tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   h Heading along tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i Signed time interval between the two tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   I Unsigned time interval between the two tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n Names of the two tracks.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   N Id numbers of the two tracks.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t Absolute time along tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   T Time since start of track along tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   v Speed along tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   w weight assigned to the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x x-coordinate of the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   y y-coordinate of the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z Observed values (see -C) along tracks at the crossover.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Unless -S is specified, d,h,n,t,T,v,z will yield two columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I List of tracks to ignore [Use all tracks].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Subtract systematic corrections from the data. If no correction file is given,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the default file <TAG>_corrections.txt in $X2SYS_HOME/<TAG> is assumed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Output results for tracks with more than <nx_min> crossovers only [Use all tracks].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Append e or i for external or internal crossovers [Default is both].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default region is the entire data domain].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Return only crossovers involving this track [Use all tracks].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend a '+' to make it print info relative to both tracks [Default is selected track].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W If argument can be opened as a file then we expect a List of tracks and their\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   relative weights; otherwise the argument is the constant weight for all tracks [1].\n");
	GMT_Option (API, "bo,do,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct X2SYS_LIST_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, i, n_files[2] = {0, 0};
	bool mixed = false;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				else if (n_files[GMT_IN]++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;
			case '>':	/* Got named output file */
				n_files[GMT_OUT]++;
				break;

			/* Processes program-specific parameters */
			
			case 'A':
				Ctrl->A.active = true;
				Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = true;
				Ctrl->C.col = strdup (opt->arg);
				break;
			case 'E':
				Ctrl->E.active = true;
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.flags = strdup (opt->arg);
				break;
			case 'I':
				if ((Ctrl->I.active = gmt_check_filearg (GMT, 'I', opt->arg, GMT_IN, GMT_IS_TEXTSET)) != 0)
					Ctrl->I.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'L':	/* Crossover correction table */
				Ctrl->L.active = true;
				if (opt->arg[0]) {
					if (gmt_check_filearg (GMT, 'L', opt->arg, GMT_IN, GMT_IS_TEXTSET))
						Ctrl->L.file = strdup (opt->arg);
					else
						n_errors++;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.min = atoi (opt->arg);
				break;
			case 'Q':	/* Specify internal or external only */
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'e') Ctrl->Q.mode = 1;
				else if (opt->arg[0] == 'i') Ctrl->Q.mode = 2;
				else Ctrl->Q.mode = 3;
				break;
			case 'S':
				Ctrl->S.active = true;
				if (opt->arg[0] == '+' && opt->arg[1]) {
					Ctrl->S.both = true;
					Ctrl->S.file = strdup (&opt->arg[1]);
				}
				else if (opt->arg[0])
					Ctrl->S.file = strdup (opt->arg);
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S: Must supply a track name.\n");
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'W':
				Ctrl->W.active = true;
				Ctrl->W.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files[GMT_IN] > 1, "Syntax error: Only one COEdatabase can be given (or stdin)\n");
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: More than one output file given\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.mode == 3, "Syntax error: Only one of -Qe -Qi can be specified!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (Ctrl->A.value <= 0.0 || Ctrl->A.value > 1.0), "Syntax error option -A: Asymmetry must be in the range 0-1\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && GMT->common.b.active[GMT_OUT], "Syntax error: Cannot use -E with binary output.\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->F.flags, "Syntax error: Must use -F to specify output items.\n");
	for (i = 0; Ctrl->F.flags && i < strlen (Ctrl->F.flags); i++) {
		if (!strchr (LETTERS, (int)Ctrl->F.flags[i])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F: Unknown item %c.\n", Ctrl->F.flags[i]);
			n_errors++;			
		}
		if (Ctrl->F.flags[i] == 'n') mixed = true;		/* Both numbers and text - cannot use binary output */
	}
	/* GMT->parent->mode means we are calling from mex or Python and don't want to get textsets back */
	n_errors += gmt_M_check_condition (GMT, (mixed || GMT->parent->mode) && GMT->common.b.active[GMT_OUT], "Syntax error: Cannot use -Fn with binary output\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void dump_ascii_cols (struct GMT_CTRL *GMT, double *val, int col, int n, bool first, char *record) {
	/* Short-hand to dump n = 1 or 2 numerical values in chosen format.
	 * col is used to set the format, and first is true for first item per record.
	 */
	int i;
	char text[GMT_LEN64] = {""};
	if (first) record[0] = 0;
	for (i = 0; i < n; i++) {
		if (!first) strcat (record, GMT->current.setting.io_col_separator);
		gmt_ascii_format_col (GMT, text, val[i], GMT_OUT, col);
		strcat (record, text);
		first = false;
	}
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_list (void *V_API, int mode, void *args) {
	char **trk_name = NULL, **weight_name = NULL, *tofrom[2] = {"stdin", "stdout"}, *from = NULL;
	char record[GMT_BUFSIZ] = {""};
	struct X2SYS_INFO *s = NULL;
	struct X2SYS_BIX B;
	struct X2SYS_COE_PAIR *P = NULL;
	bool mixed = false, check_for_NaN = false, both, first;
	bool internal = true;	/* false if only external xovers are needed */
	bool external = true;	/* false if only internal xovers are needed */
	uint64_t i, j, k, one, two, n_items, n_tracks;
	uint64_t p, np_use = 0, nx_use = 0, np, m, nx, *trk_nx = NULL;
	unsigned int n_weights = 0, coe_kind, n_out, n_output, o_mode;
	int error = 0, id;
	double *wesn = NULL, val[2], out[128], corr[2] = {0.0, 0.0}, sec_2_unit = 1.0, w_k, w;
	double fixed_weight = 1.0, *weights = NULL, *trk_symm = NULL;
	struct MGD77_CORRTABLE **CORR = NULL;
	struct X2SYS_LIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_NEEDS, &options, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
 	/*---------------------------- This is the x2sys_list main code ----------------------------*/

	for (i = 0; i < strlen (Ctrl->F.flags); i++) {
		if (Ctrl->F.flags[i] == 'c' || Ctrl->F.flags[i] == 'z') check_for_NaN = true; /* Do not output records where the crossover or values are NaN */
		if (Ctrl->F.flags[i] == 'n') mixed = true;		/* Both numbers and text */
	}
	if (Ctrl->Q.active) {
		if (Ctrl->Q.mode == 1) internal = false;
		if (Ctrl->Q.mode == 2) external = false;
	}
	
	sec_2_unit = GMT->current.setting.time_system.i_scale;	/* Save conversion from secs to TIME_UNIT before MGD77_Init switches to UNIX time system (seconds) */
	
	/* Initialize system via the tag */
	
	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	/* Verify that the chosen column is known to the system */
	
	if (Ctrl->C.col) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->C.col, s), "-C");
	if (s->n_out_columns != 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: -C must specify a single column name\n");
		Return (GMT_RUNTIME_ERROR);
	}
	
	/* Select internal, external, or both */
	
	coe_kind = 0;
	if (internal) coe_kind |= 1;
	if (external) coe_kind |= 2;
	if (coe_kind == 0) coe_kind = 3;	/* Both */
	
	/* Read the entire data base; note the -I, R and -S options are applied during reading */
	
	from = (Ctrl->In.file) ? Ctrl->In.file : tofrom[GMT_IN];
	if (GMT->common.R.active) wesn = GMT->common.R.wesn;	/* Passed a sub region request */
	GMT_Report (API, GMT_MSG_VERBOSE, "Read crossover database from %s...\n", from);
	np = x2sys_read_coe_dbase (GMT, s, Ctrl->In.file, Ctrl->I.file, wesn, Ctrl->C.col, coe_kind, Ctrl->S.file, &P, &nx, &n_tracks);
	GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " pairs and a total of %" PRIu64 " crossover records.\n", np, nx);

	if (np == 0 && nx == 0) {	/* End here since nothing was allocated */
		x2sys_end (GMT, s);
		Return (GMT_NOERROR);
	}
	
	if (Ctrl->W.file) {	/* Got file with weights for each track, OR a fixed weight [1] */
		if (x2sys_read_weights (GMT, Ctrl->W.file, &weight_name, &weights, &n_weights) != X2SYS_NOERROR) fixed_weight = atof (Ctrl->W.file);
	}
	
	/* Must count to see total number of COE per track */
	
	trk_nx = gmt_M_memory (GMT, NULL, n_tracks, uint64_t);
	trk_name = gmt_M_memory (GMT, NULL, n_tracks, char *);
	for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
		for (k = 0; k < 2; k++) {
			trk_nx[P[p].id[k]] += P[p].nx;
			trk_name[P[p].id[k]] = P[p].trk[k];
		}
	}
	
	/* Initialize column output types */
	
	one = 0;	two = 1;	/* Normal track order */
	both = Ctrl->S.both;		/* Usually false unless -S+<track> is set */
	if (!both) both = !Ctrl->S.active;	/* Two columns for many output choices */
	n_out = 1 + both;		/* Number of column output for some cols if single is not specified */

	GMT->current.io.col_type[GMT_OUT][GMT_X] = (s->geographic) ? GMT_IS_LON : GMT_IS_FLOAT;
	GMT->current.io.col_type[GMT_OUT][GMT_Y] = (s->geographic) ? GMT_IS_LAT : GMT_IS_FLOAT;
	GMT->current.io.col_type[GMT_OUT][GMT_T] = GMT_IS_ABSTIME;

	n_items = strlen (Ctrl->F.flags); 
	for (i = j = 0; !mixed && i < n_items; i++, j++) {	/* Overwrite the above settings */
		switch (Ctrl->F.flags[i]) {	/* acdhintTvxyz */
			case 'a':	/* Angle between tracks */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				break;
			case 'c':	/* Crossover value */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				break;
			case 'd':	/* Distance along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'h':	/* Heading along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'I':	/* Time interval (unsigned) */
			case 'i':	/* Time interval (signed) */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				break;
			case 'n':	/* Names of the track(s) [need this case to fall through] */
			case 'N':	/* ID numbers of tracks */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 't':	/* Time along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_ABSTIME;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_ABSTIME;
				break;
			case 'T':	/* Time along track since beginning of the first year of the track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'v':	/* Speed along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'w':	/* Crossover composite weight */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				break;
			case 'x':	/* x coordinate of crossover */
				GMT->current.io.col_type[GMT_OUT][j] = (s->geographic) ? GMT_IS_LON : GMT_IS_FLOAT;
				break;
			case 'y':	/* y coordinate of crossover */
				GMT->current.io.col_type[GMT_OUT][j] = (s->geographic) ? GMT_IS_LAT : GMT_IS_FLOAT;
				break;
			case 'z':	/* Observed value along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
		}
	}

	if (Ctrl->L.active && !check_for_NaN) {	/* Correction table would not be needed for output */
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Correction table not needed for chosen output (corrections ignored).\n");
		Ctrl->L.active = false;
	}

	if (Ctrl->L.active) {	/* Load an ephemeral correction table */
		x2sys_get_corrtable (GMT, s, Ctrl->L.file, n_tracks, trk_name, Ctrl->C.col, NULL, NULL, &CORR);
	}
	
	if (Ctrl->A.active) {	/* Requested asymmetry estimates */
		int *x_side[2] = {NULL, NULL}, half;
		double mid[2];
		for (j = 0; j < 2; j++) x_side[j] = gmt_M_memory (GMT, NULL, n_tracks, int);
		trk_symm = gmt_M_memory (GMT, NULL, n_tracks, double);
		for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
			for (j = 0; j < 2; j++) {	/* Set mid-point for these two tracks */
				if (gmt_M_is_dnan (P[p].start[j]))
					mid[j] = P[p].dist[j];
				else
					mid[j] = 0.5 * (P[p].stop[j] + P[p].start[j]);
			}
			for (k = 0; k < P[p].nx; k++) {
				for (j = 0; j < 2; j++) {
					if (gmt_M_is_dnan (P[p].COE[k].data[j][COE_T]))
						half = (P[p].COE[k].data[j][COE_D] > mid[j]);
					else
						half = (P[p].COE[k].data[j][COE_T] > mid[j]);
					x_side[half][P[p].id[j]]++;
				}
			}
		}
		/* Compute symmetry */
		for (k = 0; k < n_tracks; k++) trk_symm[k] = (double)( (x_side[1][k] - x_side[0][k]) / (x_side[1][k] + x_side[0][k]) );
		for (j = 0; j < 2; j++) gmt_M_free (GMT, x_side[j]);
	}
	/* Time to issue output */

	n_output = (unsigned int)n_items;	/* But some items represent a pair */
	for (i = 0; i < n_items; i++) {
		switch (Ctrl->F.flags[i]) {	/* acdhintTvwxyz */
			case 'd':	/* Distance along track */
			case 'h':	/* Heading along track */
			case 'n':	/* Names of the track(s) */
			case 'N':	/* ID numbers of tracks */
			case 't':	/* Time along track */
			case 'T':	/* Time along track since beginning of the track */
			case 'v':	/* Speed along track */
			case 'z':	/* Observed value along track */
				if (both) ++n_output;
				break;
		}
	}

	o_mode = (mixed) ? GMT_IS_TEXTSET : GMT_IS_DATASET;
	if (GMT_Init_IO (API, o_mode, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	gmt_set_cols (GMT, GMT_OUT, n_output);
	if (GMT_Begin_IO (API, o_mode, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	gmt_set_tableheader (GMT, GMT_OUT, true);
	gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
	if (!GMT->common.b.active[GMT_OUT]) {	/* Write 3 header records */
		char *cmd = NULL;
		sprintf (record, " Tag: %s %s", Ctrl->T.TAG, Ctrl->C.col);
		GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
		cmd = GMT_Create_Cmd (API, options);
		sprintf (record, " Command: %s %s", THIS_MODULE_NAME, cmd);	/* Build command line argument string */
		GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
		gmt_M_free (GMT, cmd);
		GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, "");
		record[0] = 0;	/* Clean slate */
		for (i = j = 0; i < n_items; i++, j++) {	/* Overwrite the above settings */
			if (i > 0) strcat (record, GMT->current.setting.io_col_separator);
			switch (Ctrl->F.flags[i]) {	/* acdhintTvxyz */
				case 'a':	/* Angle between tracks */
					strcat (record, "angle");
					break;
				case 'c':	/* Crossover value */
					strcat (record, Ctrl->C.col); strcat (record, "_x");
					break;
				case 'd':	/* Distance along track */
					if (both) {
						strcat (record, "dist_1");	strcat (record, GMT->current.setting.io_col_separator);	strcat (record, "dist_2");
					}
					else
						strcat (record, "dist");
					break;
				case 'h':	/* Heading along track */
					if (both) {
						strcat (record, "head_1");	strcat (record, GMT->current.setting.io_col_separator);	strcat (record, "head_2");
					}
					else
						strcat (record, "head");
					break;
				case 'I':	/* Time interval (unsigned) */
					strcat (record, "u_tint");
					break;
				case 'i':	/* Time interval (signed) */
					strcat (record, "s_tint");
					break;
				case 'n':	/* Names of the track(s) [need this case to fall through] */
					if (both) {
						strcat (record, "track_1");	strcat (record, GMT->current.setting.io_col_separator);	strcat (record, "track_2");
					}
					else
						strcat (record, "track");
					break;
				case 'N':	/* ID numbers of tracks */
					if (both) {
						strcat (record, "ID_1");	strcat (record, GMT->current.setting.io_col_separator);	strcat (record, "ID_2");
					}
					else
						strcat (record, "ID");
					break;
				case 't':	/* Time along track */
					if (both) {
						strcat (record, "t_1");	strcat (record, GMT->current.setting.io_col_separator);	strcat (record, "t_2");
					}
					else
						strcat (record, "t");
					break;
				case 'T':	/* Time along track since beginning of the first year of the track */
					if (both) {
						strcat (record, "T_1");	strcat (record, GMT->current.setting.io_col_separator);	strcat (record, "T_2");
					}
					else
						strcat (record, "T");
					break;
				case 'v':	/* Speed along track */
					if (both) {
						strcat (record, "vel_1");	strcat (record, GMT->current.setting.io_col_separator);	strcat (record, "vel_2");
					}
					else
						strcat (record, "vel");
					break;
				case 'w':	/* Composite weight of crossover */
					strcat (record, "weight");
					break;
				case 'x':	/* x coordinate of crossover */
					(s->geographic) ? strcat (record, "lon") : strcat (record, "x");
					break;
				case 'y':	/* y coordinate of crossover */
					(s->geographic) ? strcat (record, "lat") : strcat (record, "y");
					break;
				case 'z':	/* Observed value along track */
					if (both) {
						strcat (record, Ctrl->C.col);	strcat (record, "_1");	strcat (record, GMT->current.setting.io_col_separator);
						strcat (record, Ctrl->C.col);	strcat (record, "_2");
					}
					else
						strcat (record, Ctrl->C.col);
					break;
			}
		}
		GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
	}
	
	for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
		if (Ctrl->N.active && (trk_nx[P[p].id[0]] < Ctrl->N.min || trk_nx[P[p].id[1]] < Ctrl->N.min)) continue;			/* Not enough COEs */
		if (Ctrl->A.active && (fabs (trk_symm[P[p].id[0]]) > Ctrl->A.value || fabs (trk_symm[P[p].id[1]]) > Ctrl->A.value)) continue;	/* COEs not distributed symmatrically */
		np_use++;
		nx_use += P[p].nx;
		if (Ctrl->E.active) {	/* Write segment header with information */
			sprintf (record, "%s - %s nx = %d", P[p].trk[0], P[p].trk[1], P[p].nx);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, record);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Crossovers from %s minus %s [%d].\n", P[p].trk[0], P[p].trk[1], P[p].nx);
		if (Ctrl->S.active) {	/* May have to flip which is track one and two */
			two = !strcmp (Ctrl->S.file, P[p].trk[0]);
			one = 1 - two;
		}
		
		for (k = 0; k < P[p].nx; k++) {	/* For each crossover */
			/* First check if this record has a non-NaN COE */
			if (check_for_NaN && (gmt_M_is_dnan (P[p].COE[k].data[one][COE_Z]) || gmt_M_is_dnan (P[p].COE[k].data[two][COE_Z]))) continue;
			record[0] = 0;	/* Clean slate */
			for (i = j = 0, first = true; i < n_items; i++, j++) {
				switch (Ctrl->F.flags[i]) {	/* acdhintTvwxyz */
					case 'a':	/* Angle between tracks */
						val[0] = fabs (P[p].COE[k].data[0][COE_H] - P[p].COE[k].data[1][COE_H]);
						while (val[0] >= 180.0) val[0] -= 180.0;
						out[j] = (val[0] > 90.0) ? 180.0 - val[0] : val[0];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first, record);
						break;
					case 'c':	/* Crossover value */
					 	if (Ctrl->L.active) {
							corr[one] = MGD77_Correction_Rec (GMT, CORR[P[p].id[one]][COE_Z].term, P[p].COE[k].data[one], NULL);
							corr[two] = MGD77_Correction_Rec (GMT, CORR[P[p].id[two]][COE_Z].term, P[p].COE[k].data[two], NULL);
						}
						out[j] = val[0] = (P[p].COE[k].data[one][COE_Z] - corr[one]) - (P[p].COE[k].data[two][COE_Z] - corr[two]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first, record);
						break;
					case 'd':	/* Distance along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_D];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_D];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first, record);
						break;
					case 'h':	/* Heading along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_H];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_H];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first, record);
						break;
					case 'i':	/* Time interval in current TIME_UNIT */
						out[j] = val[0] = sec_2_unit * (P[p].COE[k].data[one][COE_T] - P[p].COE[k].data[two][COE_T]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first, record);
						break;
					case 'I':	/* Time interval in current TIME_UNIT */
						out[j] = val[0] = sec_2_unit * fabs (P[p].COE[k].data[one][COE_T] - P[p].COE[k].data[two][COE_T]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first, record);
						break;
					case 'n':	/* Names of the track(s) */
						if (both) {
							if (!first) strcat (record, GMT->current.setting.io_col_separator);
							strcat (record, P[p].trk[0]);
							strcat (record, GMT->current.setting.io_col_separator);
							strcat (record, P[p].trk[1]);
						}
						else {
							if (!first) strcat (record, GMT->current.setting.io_col_separator);
							strcat (record, P[p].trk[two]);
						}
						break;
					case 'N':	/* ID numbers of tracks */
						out[j] = val[0] = (double)P[p].id[one];
						if (both) out[++j] = val[1] = (double)P[p].id[two];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first, record);
						break;
					case 't':	/* Time along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_T];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_T];
						if (mixed) dump_ascii_cols (GMT, val, GMT_T, n_out, first, record);
						break;
					case 'T':	/* Time along track since beginning of the track */
						out[j] = val[0] = sec_2_unit * (P[p].COE[k].data[one][COE_T] - P[p].start[one]);
						if (both) out[++j] = val[1] = sec_2_unit * (P[p].COE[k].data[two][COE_T] - P[p].start[two]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first, record);
						break;
					case 'v':	/* Speed along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_V];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_V];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first, record);
						break;
					case 'w':	/* Weight for this crossover */
						if (weights) {	/* Weightfile was given; compute composite weight for this COE */
							for (m = 0, w_k = 0.0; m < 2; m++) {
								if ((id = x2sys_find_track (GMT, P[p].trk[m], weight_name, n_weights)) == -1) {
									GMT_Report (API, GMT_MSG_VERBOSE, "No weights found for track %s - using weight = 1.\n", P[p].trk[m]);
									w = 1.0;
								}
								else
									w = weights[id];
								w_k += 1.0 / (w*w);
							}
							out[j] = sqrt (1.0/w_k);
						}
						else
							out[j] = fixed_weight;
						if (mixed) {val[0] = out[j];	dump_ascii_cols (GMT, val, GMT_Z, 1, first, record);}
						break;
					case 'x':	/* x coordinate of crossover */
						out[j] = val[0] = P[p].COE[k].data[0][COE_X];
						if (mixed) dump_ascii_cols (GMT, val, GMT_X, 1, first, record);
						break;
					case 'y':	/* y coordinate of crossover */
						out[j] = val[0] = P[p].COE[k].data[0][COE_Y];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Y, 1, first, record);
						break;
					case 'z':	/* Observed value along track */
						if (Ctrl->L.active) corr[one] = MGD77_Correction_Rec (GMT, CORR[P[p].id[one]][COE_Z].term, P[p].COE[k].data[one], NULL);
						out[j] = val[0] = P[p].COE[k].data[one][COE_Z] - corr[one];
						if (both) {
							if (Ctrl->L.active) corr[two] = MGD77_Correction_Rec (GMT, CORR[P[p].id[two]][COE_Z].term, P[p].COE[k].data[two], NULL);
							out[++j] = val[1] = P[p].COE[k].data[two][COE_Z] - corr[two];
						}
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first, record);
						break;
				}
				first = false;
			}
			if (mixed)
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			else
				GMT_Put_Record (API, GMT_WRITE_DATA, out);
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Output %" PRIu64 " pairs and a total of %" PRIu64 " crossover records.\n", np_use, nx_use);
	
	/* Done, free up data base array */
	
	x2sys_free_coe_dbase (GMT, P, np);
	gmt_M_free (GMT, trk_nx);
	if (Ctrl->A.active) gmt_M_free (GMT,  trk_symm);

	if (Ctrl->L.active) MGD77_Free_Correction (GMT, CORR, (unsigned int)n_tracks);
	gmt_M_free (GMT, trk_name);
	x2sys_end (GMT, s);

	Return (GMT_NOERROR);
}
