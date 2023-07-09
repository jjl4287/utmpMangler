////////////////////////////////////////////////////////////////////////////////
/* 
 * utmpMangler.c
 *   synopsis: spp5 [FILE]... [OPTION]...
 * 
 *             a tool to grab stated utmp files and coalesce them into one
 *             file, based on filterable information like date iOR user
 *             if no output file specified, will print to stdout in human
 *             readable form
 *          
 * edge cases: when login is on last day of month and rolls 
 *             over past midnight, it will show on the next
 *             months log as  well as the previous  months.
 * 
 *    options: when no options given will read from /var/log/wtmp to stdout
 *         
 *            -d[MM/YYYY] (M/YYYY works when applicable)
 *                  filters output by month
 * 
 *            -u[USERNAME]
 *                  filters output by username
 *
 *            -o[OUTPUTFILE]
 *                  sends structs directly to OUTPUTFILE in non readable form
 *                  *not* to stdout
 * 
 *     author: Jakob J.A. Langtry, 2022.
 */

#include <utmp.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#define SHOWHOST     // include remote machines on output

int   optflag = 0;   // octal flag to AND for option functionality

// prototyping functions
int   cmp_time(int, int, long);
int   utfilter(struct utmp *, char *, int, int);
void  showtime(long);
void  shw_info(struct utmp *);

int main (int argc, char **argv)
{
    char        *date = NULL; // date filter string
    char        *user = NULL; // user filter string
    char      *output = NULL; // output file string
    int        readfd =    0; // read  file
    int        writfd =    0; // write file
    int      monthnum =    0; // to save month
    int       yearnum =    0; // to save year
    int           opt =   -1; // variable to store getopt() return
    struct utmp        utbuf; // read info to here

    while ((opt = getopt(argc, argv, "o:d:u:")) != -1)
    {
        switch (opt)   // increments octal value for options
        {
            case 'o':  // output file is given
                optflag++;
                output = malloc(sizeof(optarg));
                strcpy(output, optarg);
                break;
            case 'd':  // date range is given
                optflag = optflag + 2;
                date = malloc(sizeof(optarg));
                strcpy(date, optarg);
                break;
            case 'u':  // user filter is given
                optflag = optflag + 4;
                user = malloc(sizeof(optarg));
                strcpy(user, optarg);
                break;
            default:   // nothing is given
                break;
        }
    }

    // if output file is given open output file and set perms
    if (((optflag&1)==1)&&(writfd = open(output, O_CREAT|O_RDWR)) == -1)
    {
        perror( "Output File"); // make sure we can open output file
        exit(1);
    }
    else if ((optflag&1)==1)
    {
        fchmod(writfd, 00644);  // sets the permissions of this file
    }

    if ((optflag&2)==2) // if filtering by date, split input into two ints
    {
        char *myrandompointer = NULL; // this is for strtok_r
        monthnum = atoi(strtok_r(date, "/", &myrandompointer));
        yearnum = atoi(strtok_r(NULL, "/", &myrandompointer));
    }

    /* 
     * while there are input files to read read them
     * this do while loop incorporates both
     * the default case where no input file is given and /var/log/wtmp
     * is used, but also the case where many input files are given
     * in default, it needs to run once but optind is not less than argc
     * so it needs the do while loop for it to run
     */
    do
    {
        if (optind < argc) // if input files are given use those for reading
        {
            if ((readfd = open(argv[optind], O_RDONLY)) == -1) // check for error
            {
                perror("Input File");
                exit(1);
            }
        }
        else // else use /var/log/wtmp for default
        {
            if ((readfd = open("/var/log/wtmp", O_RDONLY)) == -1) // check for error
            {
                perror("Input File");
                exit(1);
            }
        }
        // while there are structs to read read them
        while (read(readfd, &utbuf, sizeof(utbuf)) == sizeof(utbuf))
        {
            // if output file is given then write structs to it non human readably
            if (((optflag & 1) == 1) && (utfilter(&utbuf, user, monthnum, yearnum)))
            {
                write(writfd, &utbuf, sizeof(utbuf));
            }
            // else output to stdout in human readable form
            if (((optflag & 1) != 1) && (utfilter(&utbuf, user, monthnum, yearnum)))
            {
                shw_info(&utbuf);
            }
        }
        close(readfd);
        optind++;
    } while (optind < argc);

    // clear up memory
    free(user);
    free(output);
    free(date);

    // close up the file
    close(writfd);
    return 0; // hooray message
}

/*
 * utfilter - a function that will return 1 if struct matches given
 *            parameters, and the struct should be printed, and will 
 *            return a 0 if the struct should not be printed.
 */
int utfilter(struct utmp *utbufp, char *user, int monthnum, int yearnum)
{
    // if date filter given, compare the times of given struct with args
    if (((optflag&2)==2)&&(!cmp_time(monthnum, yearnum, utbufp->ut_time)))
    {
        return(0);
    }
    // if user filter given, compare given user string with utmp struct
    if (((optflag&4)==4)&&(strncmp(user, utbufp->ut_user, UT_NAMESIZE)))
    {
        return(0);
    }
    return(1);
}

/*
 * show_info - displays contents of the utmp struct in human readable form
 *             *note* these sizes should not be hardwired
 * 
 *             from the books who code (mostly)
 */
void shw_info(struct utmp *utbufp)
{
    if (utbufp->ut_type != USER_PROCESS)
        return;
    printf("%-8.8s", utbufp->ut_user);  // the logname
    printf(" ");                        // a space
    printf("%-8.8s", utbufp->ut_line);  // the tty
    printf(" ");                        // a space
    showtime(utbufp->ut_time);
#ifdef SHOWHOST
    if ( utbufp->ut_host[0] != '\0' )
        printf("(%s)", utbufp->ut_host);// the host
#endif
    printf("\n");                       // newline
}

/*
 * cmp_time - a function that compares a unix epoch timeval from a 
 *            utmp file listing to a given MM/YYYY time value
 *            
 *            when time is equivelent returns 1
 *            else returns 0
 */
int cmp_time(int monthnum, int yearnum, long timeval)
{
    // declaring struct tm for localtime
    struct tm *tmcmp = NULL;

    // running localtime to convert unix timestamp to days and hours
    tmcmp = localtime(&timeval);

    // making passes variables compliant with tm format
    monthnum--;
    yearnum -= 1900;

    // returning true when true, false when false
    return ((monthnum == tmcmp->tm_mon) && (yearnum == tmcmp->tm_year));
}

/*
 * showtime - when printing in human readable form to stdout
 *            this is used to print out correct time  value.
 */
void showtime(long timeval)
{
    char *cp;               // to hold address of time
    cp = ctime(&timeval);   // convert time to string
    printf("%12.12s", cp+4);
}