/*******************************************************************************

License: 
This software and/or related materials was developed at the National Institute
of Standards and Technology (NIST) by employees of the Federal Government
in the course of their official duties. Pursuant to title 17 Section 105
of the United States Code, this software is not subject to copyright
protection and is in the public domain. 

This software and/or related materials have been determined to be not subject
to the EAR (see Part 734.3 of the EAR for exact details) because it is
a publicly available technology and software, and is freely distributed
to any interested party with no licensing requirements.  Therefore, it is 
permissible to distribute this software as a free download from the internet.

Disclaimer: 
This software and/or related materials was developed to promote biometric
standards and biometric technology testing for the Federal Government
in accordance with the USA PATRIOT Act and the Enhanced Border Security
and Visa Entry Reform Act. Specific hardware and software products identified
in this software were used in order to perform the software development.
In no case does such identification imply recommendation or endorsement
by the National Institute of Standards and Technology, nor does it imply that
the products and equipment identified are necessarily the best available
for the purpose.

This software and/or related materials are provided "AS-IS" without warranty
of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
licensed product, however used. In no event shall NIST be liable for any
damages and/or costs, including but not limited to incidental or consequential
damages of any kind, including economic damage or injury to property and lost
profits, regardless of whether NIST shall be advised, have reason to know,
or in fact shall know of the possibility.

By using this software, you agree to bear all risk relating to quality,
use and performance of the software and/or related materials.  You agree
to hold the Government harmless from any claim arising from your use
of the software.

*******************************************************************************/

/***********************************************************************
      PACKAGE:        Bozorth Fingerprint Matcher

      FILE:           BOZORTH3.C

      ALGORITHM:      Allan S. Bozorth (FBI)
      MODIFICATIONS:  Michael D. Garris (NIST)
                      Stan Janet (NIST)
                      Kenneth Ko (NIST)
      DATE:           09/21/2004
      UPDATE:         02/26/2007


#cat: bozorth3 - Compares two fingerprint minutiae (x,y,theta) templates
#cat:            and returns a score that can be used to decide if the
#cat:            fingerprints are from the same person. It is rotation
#cat:            and translation invariant.

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <signal.h>
#include <bozorth.h>
#include <version.h>

/* Default {x,y,t} representation is "NIST internal", not M1 */
int m1_xyt                  = 0;

int max_minutiae            = DEFAULT_BOZORTH_MINUTIAE;
int min_computable_minutiae = MIN_COMPUTABLE_BOZORTH_MINUTIAE;

/*int verbose         = 0;*/
int verbose_main      = 0;
int verbose_load      = 0;
int verbose_bozorth   = 0;
int verbose_threshold = 0;


FILE * errorfp            = FPNULL;

extern char                *optarg;
extern int                 optind,opterr,optopt;
extern void                usage( FILE * );
extern void                print_version( FILE * );

/**********************************************************************************/


int main( int argc, char ** argv )
{
int nerrors = 0;
int parse_errors = 0;
int dry_run = 0;
int no_output = 0;
int computations = 0;


#ifdef TESTING
int iflag = 0;
#endif

int bflag = 0;
int lflag = 0;
int gallery_from_argv = 1;		/* true if gallery files are to be pulled from the command line */
int probes_from_argv = 1;		/* true if probe   files are to be pulled from the command line */
int neither_from_argv;
int both_from_argv;
int both_fixed;				/* true if probe file has been fixed with "-p" and gallery file with "-g" */
int argcount;


int threshold_set = 0;
int threshold = -1;
int threshold_stop_flag = 0;

#ifdef PARALLEL_SEARCH
int stop_read_fd  = -1;
int stop_write_fd = -1;
int stop_fds = 0;
pid_t pid;				/* if multiple instances are of the matcher are run simultaneously, */
					/*	errors and verbose output lines will be tagged with process ID */
#endif

char * probe_list         = CNULL;
char * gallery_list       = CNULL;
char * mates_list         = CNULL;
char * fixed_probe_file   = CNULL;
char * fixed_gallery_file = CNULL;
char * outdir             = CNULL;
char * outfile            = CNULL;
char * errorfile          = CNULL;

FILE * gallery_fp         = FPNULL;
FILE * probe_fp           = FPNULL;
FILE * mates_fp           = FPNULL;
FILE * outfp              = stdout;

char * program		= PROGRAM;

int max_line_count      = 0;
int max_list_length	= MAX_FILELIST_LENGTH;
char * outfmt		= DEFAULT_SCORE_LINE_FORMAT;
int print_scores_at_end;
int line_count;
char ** lines		= (char **) NULL;
struct xyt_struct * pstruct = XYT_NULL;
struct xyt_struct * gstruct = XYT_NULL;
int probe_len		= 0;		/* set and used only with a fixed_probe_file */
int pline_begin		= -1;
int pline_end		= -1;
int gline_begin		= -1;
int gline_end		= -1;
int plineno		= 0;
int glineno		= 0;
int exit_status		= 0;

static char default_getopt_spec[] = "+A:bhlM:m:n:P:p:G:g:D:o:e:T:qtvV";
static char * getopt_spec;




#ifdef TESTING
	getopt_spec = malloc_or_exit( (int)strlen(default_getopt_spec) + 2, "getopt() string" );
	strcpy( getopt_spec, default_getopt_spec );
	strcat( getopt_spec, "i" );
#else
	getopt_spec = malloc_or_exit( (int)strlen(default_getopt_spec) + 1, "getopt() string" );
	strcpy( getopt_spec, default_getopt_spec );
#endif



while (1) {
	int c;

	c = getopt( argc, argv, getopt_spec );
	if ( c == -1 )
		break;

	if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
		getVersion();
		exit(0);
	}

	switch ( c ) {
		case 'h':
			usage( outfp );
			exit(0);
		case 'A':
			{
			int s;
			static char A_dryrun[]   = "dryrun";
			static char A_nooutput[] = "nooutput";
#ifdef PARALLEL_SEARCH
			static char A_rfd[]      = "rfd=";
			static char A_wfd[]      = "wfd=";
#endif
			static char A_mm[]       = "minminutiae=";
			static char A_mf[]       = "maxfiles=";
			static char A_pl[]       = "plines=";
			static char A_gl[]       = "glines=";
			static char A_fmt[]      = "outfmt=";
			/* Note that these selective verbose options are */
                        /* not currently listed in usage() */
			static char A_verbose[]  = "verbose=";

			if ( strncmp(optarg,A_verbose,strlen(A_verbose)) == 0 ) {
				if ( strcmp( optarg + strlen(A_verbose), "bozorth" ) == 0 )
					verbose_bozorth = 1;
				else if ( strcmp( optarg + strlen(A_verbose), "threshold" ) == 0 )
					verbose_threshold = 1;
				else if ( strcmp( optarg + strlen(A_verbose), "load" ) == 0 )
					verbose_load = 1;
				else if ( strcmp( optarg + strlen(A_verbose), "main" ) == 0 )
					verbose_main = 1;
				else {
					fprintf( stderr, "%s: ERROR: bad verbose specifier \"%s\"\n",
								PROGRAM, optarg+strlen(A_verbose) );
					++parse_errors;
				}
				break;
			}
			if ( strcmp(optarg,A_dryrun) == 0 ) {
				dry_run = 1;
				break;
			}
			if ( strcmp(optarg,A_nooutput) == 0 ) {
				no_output = 1;
				break;
			}

#ifdef PARALLEL_SEARCH
			if ( strncmp(optarg,A_rfd,strlen(A_rfd)) == 0 ) {
				stop_read_fd = atoi( optarg + strlen(A_rfd) );
				if ( stop_read_fd < 0 ) {
					fprintf( stderr, "%s: WARNING: negative read file descriptor (%d) is illegal\n",
								PROGRAM, stop_read_fd );
					stop_read_fd = -1;
				}
				break;
			}
			if ( strncmp(optarg,A_wfd,strlen(A_wfd)) == 0 ) {
				stop_write_fd = atoi( optarg + strlen(A_wfd) );
				if ( stop_write_fd < 0 ) {
					fprintf( stderr, "%s: WARNING: negative write file descriptor (%d) is illegal\n",
								PROGRAM, stop_write_fd );
					stop_write_fd = -1;
				}
				break;
			}
#endif
			if ( strncmp(optarg,A_fmt,strlen(A_fmt)) == 0 ) {
				outfmt = optarg + strlen(A_fmt);
				break;
			}
			if ( strncmp(optarg,A_mm,strlen(A_mm)) == 0 ) {
				min_computable_minutiae = atoi( optarg + strlen(A_mm) );
				if ( min_computable_minutiae < 0 ) {
					fprintf( stderr, "%s: WARNING: negative min_computable_minutiae (%d) is illegal\n",
								PROGRAM, min_computable_minutiae );
					++parse_errors;
				}
				if ( verbose_main )
					fprintf( stderr, "Minimum minutiae set to %d\n", min_computable_minutiae );
				break;
			}
			if ( strncmp(optarg,A_mf,strlen(A_mf)) == 0 ) {
				max_list_length = atoi( optarg + strlen(A_mf) );
				if ( max_list_length < 0 ) {
					fprintf( stderr, "%s: WARNING: negative max_list_length (%d) is illegal\n",
								PROGRAM, max_list_length );
					++parse_errors;
				}
				if ( verbose_main )
					fprintf( stderr, "Maximum files set to %d\n", max_list_length );
				break;
			}
			if ( strncmp(optarg,A_pl,strlen(A_pl)) == 0 ) {
				s = parse_line_range( optarg+strlen(A_pl), &pline_begin, &pline_end );
				if ( s < 0 ) {
					fprintf( stderr, "%s: ERROR: bad probe line range specifier \"%s\" (error code %d)\n",
								PROGRAM, optarg+strlen(A_pl), s );
					++parse_errors;
					break;
				}
				if ( verbose_main )
					fprintf( stderr, "Probe lines restricted to %d - %d\n",
								pline_begin, pline_end );
				break;
			}
			if ( strncmp(optarg,A_gl,strlen(A_gl)) == 0 ) {
				s = parse_line_range( optarg+strlen(A_gl), &gline_begin, &gline_end );
				if ( s < 0 ) {
					fprintf( stderr, "%s: ERROR: bad gallery line range specifier \"%s\" (error code %d)\n",
								PROGRAM, optarg+strlen(A_gl), s );
					++parse_errors;
					break;
				}
				if ( verbose_main )
					fprintf( stderr, "Gallery lines restricted to %d - %d\n",
								gline_begin, gline_end );
				break;
			}
			}
			fprintf( stderr, "%s: ERROR: unknown -A argument \"%s\"\n",
								PROGRAM, optarg );
			++parse_errors;
			break;
#ifdef TESTING
		case 'i':
			++iflag;
			break;
#endif
		case 'b':
			bflag = 1;
			if ( verbose_main )
				fprintf( stderr, "Selected buffered I/O\n" );
			break;
		case 'l':
			lflag = 1;
			if ( verbose_main )
				fprintf( stderr, "Selected line-buffering\n" );
			break;
		case 'M':
			probes_from_argv = 0;
			gallery_from_argv = 0;
			mates_list = optarg;
			if ( verbose_main )
				fprintf( stderr, "Mates list file is %s\n", mates_list );
			break;
		case 'm': /* "-m1" */
			if ( strcmp(optarg,"1") != 0) {
				fprintf( stderr, "%s: ERROR: illegal -m option (-m%s), \"-m1\" expected\n",
					PROGRAM, optarg);
				++parse_errors;
			}
                        m1_xyt = 1;
			if ( verbose_main )
				fprintf( stderr, "-m1 option set\n");
			break;
		case 'P':
			probes_from_argv = 0;
			probe_list = optarg;
			if ( verbose_main )
				fprintf( stderr, "Probe list file is %s\n", probe_list );
			break;
		case 'p':
			probes_from_argv = 0;
			fixed_probe_file = optarg;
			if ( verbose_main )
				fprintf( stderr, "Probe file is fixed as %s\n", fixed_probe_file );
			break;
		case 'G':
			gallery_from_argv = 0;
			gallery_list = optarg;
			if ( verbose_main )
				fprintf( stderr, "Gallery list file is %s\n", gallery_list );
			break;
		case 'g':
			gallery_from_argv = 0;
			fixed_gallery_file = optarg;
			if ( verbose_main )
				fprintf( stderr, "Gallery file is fixed as %s\n", fixed_gallery_file );
			break;
		case 'D':
			outdir = optarg;
			if ( verbose_main )
				fprintf( stderr, "Output directory is %s\n", outdir );
			break;
		case 'o':
			outfile = optarg;
			if ( verbose_main )
				fprintf( stderr, "Output file is %s\n", outfile );
			break;
		case 'e':
			errorfile = optarg;
			if ( verbose_main )
				fprintf( stderr, "Error file is %s\n", errorfile );
			break;
		case 'n':
			if ( verbose_main )
				fprintf( stderr, "Max minutiae to use from each file specified as \"%s\"\n", optarg );
			max_minutiae = atoi( optarg );
			if ( max_minutiae < 0 ) {
				fprintf( stderr, "%s: WARNING: negative max_minutiae (%d) is illegal\n",
								PROGRAM, max_minutiae );
				++parse_errors;
			}
			if ( max_minutiae < MIN_BOZORTH_MINUTIAE ) {
				fprintf( stderr, "%s: WARNING: max_minutiae (%d) < lower limit of %d is illegal\n",
								PROGRAM, max_minutiae, MIN_BOZORTH_MINUTIAE );
				++parse_errors;
			}
			if ( max_minutiae > MAX_BOZORTH_MINUTIAE ) {
				fprintf( stderr, "%s: WARNING: max_minutiae (%d) > upper limit of %d is illegal\n",
								PROGRAM, max_minutiae, MAX_BOZORTH_MINUTIAE );
				++parse_errors;
			}
			break;
		case 'v':
			/* turn on all verbose flags */
			verbose_bozorth = 1;
			verbose_threshold = 1;
			verbose_load = 1;
			verbose_main = 1;

			break;
		case 'T':
			threshold = atoi( optarg );
			if ( threshold < 0 ) {
				fprintf( stderr, "%s: WARNING: negative threshold (%d) is illegal\n",
								PROGRAM, threshold );
				++parse_errors;
			}
			threshold_set = 1;
			break;
		case 'q':
			threshold_stop_flag = 1;
			break;
		default:
			fprintf( stderr, "%s: ERROR: getopt() returned character code 0%o\n",
								PROGRAM, c );
			usage( stderr );
			exit(1);
	}
}




if ( mates_list != CNULL && probe_list != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-M\" and \"-P\" are not currently compatible\n", PROGRAM );
	++parse_errors;
}
if ( mates_list != CNULL && gallery_list != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-M\" and \"-G\" are not currently compatible\n", PROGRAM );
	++parse_errors;
}
if ( mates_list != CNULL && fixed_probe_file != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-M\" and \"-p\" are incompatible\n", PROGRAM );
	++parse_errors;
}
if ( mates_list != CNULL && fixed_gallery_file != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-M\" and \"-g\" are incompatible\n", PROGRAM );
	++parse_errors;
}

if ( gallery_list != CNULL && probe_list != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-G\" and \"-P\" are not currently compatible\n", PROGRAM );
	++parse_errors;
}
if ( probe_list != CNULL && fixed_probe_file != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-P\" and \"-p\" are incompatible\n", PROGRAM );
	++parse_errors;
}
if ( gallery_list != CNULL && fixed_gallery_file != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-G\" and \"-g\" are incompatible\n", PROGRAM );
	++parse_errors;
}
if ( no_output && outfile != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-o\" and \"-A nooutput\" are incompatible\n", PROGRAM );
	++parse_errors;
}
if ( no_output && outdir != CNULL ) {
	fprintf( stderr, "%s: ERROR: flags \"-O\" and \"-A nooutput\" are incompatible\n", PROGRAM );
	++parse_errors;
}

if ( threshold_stop_flag && ! threshold_set ) {
	fprintf( stderr, "%s: ERROR: flag \"-q\" and requires that a threshold be set\n", PROGRAM );
	++parse_errors;
}

#ifdef PARALLEL_SEARCH
if ( stop_read_fd >= 0 && ! threshold_stop_flag ) {
	fprintf( stderr, "%s: ERROR: if stop read fd is set, the threshold stop flag be also set\n", PROGRAM );
	++parse_errors;
}
if ( stop_write_fd >= 0 && ! threshold_stop_flag ) {
	fprintf( stderr, "%s: ERROR: if stop write fd is set, the threshold stop flag be also set\n", PROGRAM );
	++parse_errors;
}
if ( stop_read_fd >= 0 || stop_write_fd >= 0 ) {	/* -p probefile.xyt { gallery*.xyt | -G gallery.lis } */
	if ( fixed_probe_file == CNULL ) {
		fprintf( stderr, "%s: ERROR: options \"-A [rw]fd=#\" requires \"-p\" flag\n", PROGRAM );
		++parse_errors;
	}
	if ( fixed_gallery_file != CNULL ) {
		fprintf( stderr, "%s: ERROR: options \"-A [rw]fd=#\" is incompatible with \"-g\" flag\n", PROGRAM );
		++parse_errors;
	}
	if ( stop_read_fd < 0 || stop_write_fd < 0 ) {
		fprintf( stderr, "%s: ERROR: both options \"-A rfd=#\" and \"-A wfd=#\" must be set if either is\n",
									PROGRAM );
		++parse_errors;
	}
	stop_fds = 1;
}
#endif

if ( gline_begin > 0 && gallery_list == CNULL ) {
	fprintf( stderr, "%s: ERROR: option \"-A glines=<range>\" requires \"-G\" flag\n", PROGRAM );
	++parse_errors;
}
if ( pline_begin > 0 && probe_list == CNULL ) {
	fprintf( stderr, "%s: ERROR: option \"-A plines=<range>\" requires \"-P\" flag\n", PROGRAM );
	++parse_errors;
}
if ( lflag && bflag ) {
	fprintf( stderr, "%s: ERROR: flags \"-b\" and \"-l\" are incompatible\n", PROGRAM );
	++parse_errors;
}



if ( parse_errors > 0 )
	exit(1);



#ifdef PARALLEL_SEARCH
pid = getpid();
if ( stop_fds ) {
	if ( signal( SIGPIPE, SIG_IGN ) == SIG_ERR ) {
		/* fifo(4): When a process tries to write to a FIFO that is not opened for read on the other side, */
		/*			the process is sent a SIGPIPE signal. */
		fprintf( errorfp, "%s: ERROR: signal() failed\n", PROGRAM );
		exit(1);
	}
	set_progname( 1, program, pid );
} else {
	set_progname( 0, program, pid );
}
#else
	set_progname( 0, program, (pid_t)0 );
#endif


if ( errorfile == CNULL ) {
	errorfp = stderr;		/* If no error file has been specified, error and verbose output should go to stderr */
} else {
	errorfp = fopen( errorfile, "w" );
	if ( errorfp == FPNULL ) {
		fprintf( stderr, "%s: ERROR: fopen() of error file \"%s\" failed: %s\n",
					get_progname(), errorfile, strerror(errno) );
		exit(1);
	}
}



if ( outdir != CNULL ) {
	if ( outfile != CNULL ) {
		size_t len;
		char * buf;

		if ( outfile[0] == '/' ) {
			fprintf( errorfp, "%s: ERROR: the output filename \"%s\" cannot be an absolute pathname when the output directory is specified\n",
					get_progname(), outfile );
			exit(1);
		}

		len = strlen(outdir) + strlen(outfile) + 2;
		buf = malloc_or_exit( (int) len, "output filename" );
		sprintf( buf, "%s/%s", outdir, outfile );
		outfile = buf;
	} else {
		if ( fixed_probe_file != CNULL && fixed_gallery_file == CNULL ) {
			if ( (outfile = get_score_filename(outdir,fixed_probe_file)) == CNULL )
			     exit(1);
		} else if ( fixed_gallery_file != CNULL && fixed_probe_file == CNULL ) {
			if ( (outfile = get_score_filename(outdir,fixed_gallery_file)) == CNULL )
			     exit(1);
		} else {
			fprintf( errorfp, "%s: ERROR: output filename can't be assumed unless either \"-p\" or \"-g\" is specified (but not both)\n",
							get_progname() );
			exit(1);
		}
	}
}



if ( probe_list != CNULL && fixed_gallery_file != CNULL ) {			/* -P -g */
	/* print_probe_filename_only = 1; */
} else if ( gallery_list != CNULL && fixed_probe_file != CNULL ) {		/* -G -p */
	/* print_gallery_filename_only = 1; */
}



print_scores_at_end = ( ! lflag                     && ! bflag );
both_fixed          = (   fixed_probe_file != CNULL &&   fixed_gallery_file != CNULL );
both_from_argv      = (   probes_from_argv          &&   gallery_from_argv );
neither_from_argv   = ( ! probes_from_argv          && ! gallery_from_argv );



if ( mates_list != CNULL ) {
	mates_fp = fopen( mates_list, "r" );
	if ( mates_fp == FPNULL ) {
		fprintf( errorfp, "%s: ERROR: fopen() of mates list file \"%s\" failed: %s\n",
								get_progname(), mates_list, strerror(errno) );
		exit(1);
	}
}
if ( gallery_list != CNULL ) {
	gallery_fp = fopen( gallery_list, "r" );
	if ( gallery_fp == FPNULL ) {
		fprintf( errorfp, "%s: ERROR: fopen() of gallery list file \"%s\" failed: %s\n",
								get_progname(), gallery_list, strerror(errno) );
		exit(1);
	}
}
if ( probe_list != CNULL ) {
	probe_fp = fopen( probe_list, "r" );
	if ( probe_fp == FPNULL ) {
		fprintf( errorfp, "%s: ERROR: fopen() of probe list file \"%s\" failed: %s\n",
								get_progname(), probe_list, strerror(errno) );
		exit(1);
	}
}



if ( ! no_output && outfile != CNULL ) {
	outfp = fopen( outfile, "w" );
	if ( outfp == FPNULL ) {
		fprintf( errorfp, "%s: ERROR: fopen() of output file \"%s\" failed: %s\n",
								get_progname(), outfile, strerror(errno) );
		exit(1);
	}
}



if ( neither_from_argv ) {
	if ( optind < argc ) {
		fprintf( errorfp, "%s: ERROR: no extra xyt-files can be specified on the command line when both probes and gallery are set via \"-[PpGg]\"\n",
								get_progname() );
		usage( errorfp );
		exit(1);
	}
	argcount = 0;
} else {
	argcount = ( argc - optind );
	if ( argcount <= 0 ) {
		fprintf( errorfp, "%s: ERROR: no xyt-files are specified on the command line\n",
								get_progname() );
		usage( errorfp );
		exit(1);
	}
	if ( both_from_argv ) {			/* The command line will therefore contain (probes,gallery) xyt-filename pairs */
		if ( argcount % 2 != 0 ) {
			fprintf( errorfp, "%s: ERROR: %d xyt-files specified on the command line; the count must be multiple of 2\n",
								get_progname(), argcount );
			usage( errorfp );
			exit(1);
		}
	}
}



if ( ! no_output && print_scores_at_end ) {
	if ( neither_from_argv ) {
		max_line_count = max_list_length;
	} else if ( both_from_argv ) {
		max_line_count = argcount / 2;
	} else {
		max_line_count = MAX( max_list_length, argcount );
	}
	if ( verbose_main )
		fprintf( errorfp, "maximum scores line count = %d\n", max_line_count );
	lines = (char **) malloc_or_exit( (int) ( max_line_count * sizeof(int *) ), "output score line table" );
}



if ( ! dry_run ) {
	if ( fixed_probe_file != CNULL ) {
		pstruct = bz_load( fixed_probe_file );
		if ( pstruct == XYT_NULL ) {
			fprintf( errorfp, "%s: ERROR: load of fixed probe file %s failed\n",
									get_progname(), fixed_probe_file );
			exit(1);
		}
		probe_len = bozorth_probe_init( pstruct );
	}
	if ( fixed_gallery_file != CNULL ) {
		gstruct = bz_load( fixed_gallery_file );
		if ( gstruct == XYT_NULL ) {
			fprintf( errorfp, "%s: ERROR: load of fixed gallery file %s failed\n",
									get_progname(), fixed_gallery_file );
			exit(1);
		}
	}
}



if ( ! no_output && lflag )
	setvbuf( outfp, CNULL, _IOLBF, 0 );



line_count = 0;
plineno = 0;
glineno = 0;



while (1) {
	int n = 0;
	char * p;
	char * g;
	int ok;
	int done_now;
	int done_probe_now = 0;
	int done_gallery_now = 0;
	int done_afterwards;
	int done_probe_afterwards = 0;
	int done_gallery_afterwards = 0;
	int done_after_open_failure = 0;
	int threshold_met;
	char pline[ MAX_LINE_LENGTH ];
	char gline[ MAX_LINE_LENGTH ];


#ifdef PARALLEL_SEARCH
	if ( stop_fds ) {
		if ( fd_readable( stop_read_fd ) ) {
			(void) close( stop_write_fd );
			(void) close( stop_read_fd );
			stop_write_fd = -1;
			stop_read_fd  = -1;
			fprintf( errorfp, "%s: exiting after %d computations; a match was found by a sibling matcher process\n",
								get_progname(), computations );
			break;
		}
	}
#endif

	p = get_next_file( fixed_probe_file, probe_fp, mates_fp, &done_probe_now, &done_probe_afterwards,
					&pline[0], argc, argv, &optind, &plineno, pline_begin, pline_end );
	g = get_next_file( fixed_gallery_file, gallery_fp, mates_fp, &done_gallery_now, &done_gallery_afterwards,
					&gline[0], argc, argv, &optind, &glineno, gline_begin, gline_end );

	done_now        = ( done_gallery_now        || done_probe_now        );
	done_afterwards = ( done_gallery_afterwards || done_probe_afterwards );
	if ( done_now ) {
		if ( verbose_main > 1 )
			fprintf( errorfp, "breaking main loop now [line_count = %d; %d; %d; %d; %d]\n",
							line_count,
							done_probe_now,
							done_gallery_now,
							done_probe_afterwards,
							done_gallery_afterwards
							);
		break;
	}
	if ( dry_run || verbose_main )
		fprintf( errorfp, "probefile=%s galleryfile=%s\n", p, g );
	if ( dry_run )
		continue;

#ifdef TESTING
	if ( iflag )
		bozorth_init( ( iflag == 1 ) ? 0x00 : 0xFF );
#endif

	ok = 1;
	if ( fixed_probe_file == CNULL ) {		/* unless probe file is fixed, load next probe file */
		pstruct = bz_load( p );
		if ( pstruct == XYT_NULL ) {
			++nerrors;
			ok = 0;
			done_after_open_failure = 1;
		}
	}
	if ( fixed_gallery_file == CNULL ) {		/* unless gallery file is fixed, load next gallery file */
		gstruct = bz_load( g );
		if ( gstruct == XYT_NULL ) {
			++nerrors;
			ok = 0;
			done_after_open_failure = 1;
		}
	}
	if ( done_after_open_failure ) {
		if ( verbose_main )
			fprintf( errorfp, "breaking main loop after encountering an open failure\n" );
		break;
	}


							/* successfully loaded probe and gallery data */
	set_probe_filename( p );
	set_gallery_filename( g );
	if ( ok ) {
		if ( fixed_probe_file != CNULL ) {
			n = bozorth_to_gallery( probe_len, pstruct, gstruct );
		} else {
			n = bozorth_main( pstruct, gstruct );
		}
	}
	++computations;


	if ( fixed_probe_file == CNULL )		/* unless probe file is fixed, free the last probe file's data */
		if ( pstruct != XYT_NULL )
			free( (char *) pstruct );

	if ( fixed_gallery_file == CNULL )		/* unless gallery file is fixed, free the last gallery file's data */
		if ( gstruct != XYT_NULL )
			free( (char *) gstruct );



	threshold_met = threshold_set && ( n >= threshold );



	if ( ! threshold_set || threshold_met ) {
		if ( ! no_output ) {
			if ( print_scores_at_end ) {
                                char * scoreln;
 
                                if ( line_count >= max_line_count ) {
                                        fprintf( errorfp, "%s: ERROR: maximum scores line count (%d) exceeded\n",
                                                                       get_progname(), max_line_count );
                                        fprintf( errorfp, "    (Either increase with \"-A maxfiles=#\", or use \"-l\" or \"-b\" for line- or block-buffering, respectively)\n" );
                                        exit(1);
                                }
                                scoreln = get_score_line( p, g, n, 0, outfmt );
				if ( scoreln == CNULL ) {
					fprintf( errorfp, "%s: ERROR: strdup() for output line %d failed: %s\n",
									get_progname(), line_count+1, strerror( errno) );
					exit(1);
				}
				lines[ line_count++ ] = scoreln;
			} else {
				if ( fputs( get_score_line( p, g, n, 1, outfmt ), outfp ) == EOF ) {
					fprintf( errorfp, "%s: ERROR: fputs() of the match score line failed\n", get_progname() );
					exit(1);
				}
				if ( ferror( outfp ) ) {
					fprintf( errorfp, "%s: ERROR: match score write failure\n", get_progname() );
					exit(1);
				}
			}
		}
	}



	if ( threshold_met && threshold_stop_flag ) {

#ifdef PARALLEL_SEARCH
		if ( stop_fds ) {
			int write_status;

			fprintf( errorfp, "%s: MATCH FOUND after %d computations; match score %d >= threshold %d\n",
								get_progname(), computations, n, threshold );
			if ( verbose_threshold )
				fprintf( errorfp, "%s: closing read fd %d\n",
								get_progname(), stop_read_fd );
			(void) close( stop_read_fd );
			stop_read_fd = -1;
			if ( verbose_threshold )
				fprintf( errorfp, "%s: writing to write fd %d\n",
								get_progname(), stop_write_fd );
			write_status = write( stop_write_fd, "!", 1 );
			if ( write_status < 0 ) {
				perror( "write() to stop file descriptor failed" );
			}
			if ( verbose_threshold )
				fprintf( errorfp, "%s: closing write fd %d\n",
								get_progname(), stop_write_fd );
			(void) close( stop_write_fd );
			stop_write_fd = -1;
			if ( verbose_threshold )
				fprintf( errorfp, "%s: done\n", get_progname() );
		}
#endif

		if ( verbose_main )
			fprintf( errorfp, "breaking main loop after threshold met\n" );
		done_afterwards = 1;
		break;
	}



	if ( done_afterwards ) {
		if ( verbose_main > 1 )
			fprintf( errorfp, "breaking main loop after running out of arguments\n" );
		break;
	}

	if ( both_fixed )
		break;
}



#ifdef PARALLEL_SEARCH
if ( stop_fds ) {
	if ( stop_write_fd >= 0 ) {
		(void) close( stop_write_fd );
		stop_write_fd = -1;
	}
	if ( stop_read_fd >= 0 ) {
		(void) close( stop_read_fd );
		stop_read_fd = -1;
	}
}
#endif


if ( ! no_output ) {

	if ( print_scores_at_end ) {
		int i;

		for ( i=0; i < line_count; i++ ) {
			if ( lines[i] == CNULL ) {
				fprintf( errorfp, "%s: ERROR: output line %d of %d is NULL\n",
									get_progname(), i+1, line_count );
				++nerrors;
				break;
			}
			if ( fputs( lines[i], outfp ) == EOF ) {
				fprintf( errorfp, "%s: ERROR: fputs() of the match score line %d of %d failed\n",
									get_progname(), i+1, line_count );
				++nerrors;
				break;
			}
			if ( ferror( outfp ) ) {
				fprintf( errorfp, "%s: ERROR: match score write failure at line %d of %d\n",
									get_progname(), i+1, line_count );
				++nerrors;
				break;
			}
			free( lines[i] );
		}
		free( (char *) lines );
	}



	/* All match scores have now been printed, regardless of whether they were printed after */
	/*	every computation, or stored for printing at the end */
	if ( fflush( outfp ) != 0 ) {
		fprintf( errorfp, "%s: ERROR: fflush() of the match scores failed: %s",
									get_progname(), strerror(errno) );
		++nerrors;
	}



	/* If scores were printed to a file and not stdout, close the stream */
	if ( outfile != CNULL ) {
		if ( ferror( outfp ) ) {
			fprintf( errorfp, "%s: ERROR: match score write failure on output file \"%s\"\n",
									get_progname(), outfile );
			++nerrors;
		}
		if ( fclose( outfp ) != 0 ) {
			fprintf( errorfp, "%s: ERROR: fclose() of output file \"%s\" failed: %s\n",
									get_progname(), outfile, strerror(errno) );
			++nerrors;
		}
	}

}



if ( mates_fp != FPNULL ) {
	if ( fclose( mates_fp ) != 0 ) {
		fprintf( errorfp, "%s: ERROR: fclose() of mates list \"%s\" failed: %s\n",
								get_progname(), mates_list, strerror(errno) );
		++nerrors;
	}
}



if ( gallery_fp != FPNULL ) {
	if ( fclose( gallery_fp ) != 0 ) {
		fprintf( errorfp, "%s: ERROR: fclose() of gallery list \"%s\" failed: %s\n",
								get_progname(), gallery_list, strerror(errno) );
		++nerrors;
	}
}



if ( probe_fp != FPNULL ) {
	if ( fclose( probe_fp ) != 0 ) {
		fprintf( errorfp, "%s: ERROR: fclose() of probe list \"%s\" failed: %s\n",
								get_progname(), probe_list, strerror(errno) );
		++nerrors;
	}
}



if ( errorfile != CNULL ) {
	if ( fclose( errorfp ) != 0 ) {
		fprintf( errorfp, "%s: ERROR: fclose() of error file \"%s\" failed: %s\n",
								get_progname(), errorfile, strerror(errno) );
		++nerrors;
	}
	errorfp = FPNULL;
}



if ( nerrors > 0 )
	exit_status = 1;

#ifdef PARALLEL_SEARCH
if ( stop_fds ) {
	if ( verbose_threshold )
		fprintf( stderr, "%s: exiting with status %d\n", get_progname(), exit_status );
}
#endif

exit( exit_status );
}
