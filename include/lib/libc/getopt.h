#ifndef GETOPT_H
#define GETOPT_H

/* include files needed by this include file */

#define no_argument		0
#define required_argument	1
#define optional_argument	2

/* types defined by this include file */
struct option
{
	const char *name;		/* the name of the long option */
	int has_arg;			/* one of the above macros */
	int *flag;			/* determines if getopt_long() returns a
					 * value for a long option; if it is
					 * non-NULL, 0 is returned as a function
					 * value and the value of val is stored in
					 * the area pointed to by flag.  Otherwise,
					 * val is returned. */
	int val;			/* determines the value to return if flag is
					 * NULL. */

};

/* externally-defined variables */
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

/* function prototypes */
int getopt (int __argc, char *const __argv[], const char *__optstring);

int getopt_long (int __argc, char *const __argv[], const char *__shortopts,
		 const struct option * __longopts, int *__longind);

int getopt_long_only (int __argc, char *const __argv[], const char *__shortopts,
		      const struct option * __longopts, int *__longind);

#endif /* GETOPT_H */

