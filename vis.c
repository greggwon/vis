/***************************************************************************
 *
 *   VIS, Windowing of command output.
 *
 *   Vis displays the output of 1 or more commands on the
 *   terminal.  The output is updated repeatedly at 60 second
 *   intervals, or for the time (in seconds) of the value
 *   following the -t flag.
 *
 *   Coded by:
 *
 *   Mark Vasoll
 *   Department of Computing and Information Sciences
 *   Oklahoma State University
 *
 ***************************************************************************
 *
 *   Modifcations for windowing and multiple command capabilities done by:
 *
 *   Gregg Wonderly
 *
 *   October 1, 1985:  Added my own popen, that uses the users SHELL, or the
 *                     password file shell, depending on the existance of the
 *                     former
 */

# include	<curses.h>
# include	<signal.h>
# include	<ctype.h>
# include	<unistd.h>
# include	<unistd.h>
# include 	<stdio.h>
# include	<string.h>
# include	<stdlib.h>
# include	<poll.h>
# include	<sys/time.h>
# include	<time.h>
# include	<sys/wait.h>

# define   BELL  "\007"
# define   MAXCOLS  COLS
# define   MAXROWS  LINES
# define   STRFORM  "%.*s\n"

#define HEADER	2
#define OUTLINE	3

extern FILE *mypopen(char *, char*);
extern void wcenter( WINDOW *w, int cols, int color, char *);
extern void catch();
extern int mypclose (FILE *fp );
static WINDOW* showhelp(WINDOW *par);

int main (int argc, char **argv)
{
	char fbuf[1000];
	char buf[1000], s[1000], dashes[1000];
	char shell[1000], *getshell();
	int yyy = -1, yy, xx, i, t, intv;
	int first;
	FILE *p;
	struct pollfd fds;
	nfds_t fdcnt;
	fds.fd = 0;
	fds.events = POLLIN;
	fds.revents = 0;

	if( argc < 2 ) {
		fprintf( stderr, "usage: %s [-t time] cmd1 [cmd2] [cmd3] [...]\n", argv[0] );
		exit(1);
	}

	signal (SIGINT, SIG_IGN);
	WINDOW *mainWin = initscr ();
	start_color();
	cbreak();
	noecho();

	init_pair(HEADER, COLOR_BLACK, COLOR_WHITE);
	init_pair(OUTLINE, COLOR_BLACK, COLOR_GREEN);

	strcpy (shell, getshell());

	for (i=0; i<MAXCOLS-1 && i < (sizeof(dashes)-1); i++) dashes[i] = '-';
	dashes[i] = '\0';

	if (strcmp(argv[1], "-t") == 0) {
		intv = atoi(argv[2]); 
		first = 3;
	} 
	else {
		first = 1;
		intv = 60;
	}

	if (intv < 0) 
		intv = 5;

	signal (SIGINT, catch);
	move (0,0);
	printw("Time delay = %d second(s)", intv);
	refresh();

	int n = 0;
	for (;;) {
		int cnt = 1; /*  Initial number of lines used on the screen.  */
		for (t=first; t<argc; ++t) {
			move (cnt++,0);
			if ((p = mypopen (argv[t], shell)) == NULL) {
				printw ("Unable to open requested command.  Sorry.\n");
				catch();
			}
			strcpy(s, "Command - "); 
			strcat(s, argv[t]);
			attron(COLOR_PAIR(HEADER));
			wcenter (mainWin, MAXCOLS, 0, s); 
 			clrtoeol();
			attroff(COLOR_PAIR(HEADER));
 			move( cnt, 0 );
 			printw("$ ");
 			printw("%s", argv[t]);
 			refresh();
			while (fgets (buf, sizeof(buf)-1, p) != NULL) {
				buf[sizeof(buf)-1] = 0;
				sprintf (fbuf, STRFORM, COLS, buf); 
				move( cnt++, 0 );
				printw("%s",fbuf);
				clrtoeol();
				refresh();
				if (cnt >= MAXROWS) break;
			}
			mypclose (p);
		}
		getyx(stdscr, yy, xx);
		//clrtobot();
		if (yyy == -1) {
			yyy=yy;
		} 
		else if (yyy != yy) {
			printf(BELL);
			yyy=yy;
		}
		WINDOW *help = NULL;
		int v = 0;
		do {
			move (0,0);
			char buf[30];
			time_t now;
			time(&now);
			sprintf( buf, "%.24s", ctime( &now ));
			wcenter (mainWin, MAXCOLS, 0, buf); 
			move (0,0);
			printw("Time delay = %d second(s)", intv);
			refresh();
			fdcnt = 1;
			fds.fd = 0;
			fds.revents = 0;
			fds.events = POLLIN;
			time_t delay = intv*1000;
			if( delay == 0 )
				delay = 250;
			v= poll(&fds, fdcnt, delay);
			if( v == 1 ) {
				char c = getch();
				if( help != NULL ) {
					delwin(help);
					help = NULL;
				}
				switch( c ) {
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						intv = intv * 10 + (c-'0');
						break;
					case '\n': case '\r':
						intv=0;
						break;
					case '?':
						help = showhelp(mainWin);
						break;
					case 'q':
						catch();
					default:
						// run all commands again
						break;
				}	
			}
		} while( v == 1 );
	}
}

static void highOff(WINDOW *sub) {
	wattroff(sub,COLOR_PAIR(OUTLINE));
	//wstandend(sub);
}

static void highOn(WINDOW *sub) {
	wattron(sub,COLOR_PAIR(OUTLINE));
	//wstandout(sub);
}

static WINDOW* showhelp(WINDOW *main) {
#define HHH 10
#define WWW 40
	WINDOW *sub = derwin( main, HHH, WWW, 10, 20 );
	wmove( sub, 0, 0 );

	wclrtobot(sub);
	char *outc = " ";
	for( int i = 0; i < WWW; ++i ) {
		wmove(sub, 0, i );
		highOn(sub);
		wprintw(sub,"%s",outc);	
		highOff(sub);
	}
	wmove( sub, 0, 0 );
	wcenter(sub, WWW, OUTLINE, "HELP");
	for( int i = 1; i < HHH-1; ++i ) {
		wmove(sub, i, 0 );
		highOn(sub);
		wprintw(sub,"%s",outc);	
		highOff(sub);

		wmove(sub, i, WWW-1 );

		highOn(sub);
		wprintw(sub,"%s",outc);	
		highOff(sub);
	}
	wmove( sub, HHH-1, 0 );
	for( int i = 0; i < WWW; ++i ) {
		wmove(sub, HHH-1, i );

		highOn(sub);
		wprintw(sub,"%s",outc);	
		highOff(sub);
	}
	int ln = 2;
	wmove(sub, ln++, 3 );
	wprintw( sub, "%s","?     - this help" );
	wmove(sub, ln++, 3 );
	wprintw( sub, "%s","q     - quit" );
	wmove(sub, ln++, 3 );
	wprintw( sub, "%s","[0-9] - new delay after <CR>" );
	wmove(sub, ln++, 3 );
	wprintw( sub, "%s","<CR>  - reset delay to 250ms" );

	wrefresh(sub);
	return sub;

	wgetch(sub);
	catch();
	delwin(sub);
}

void wcenter (WINDOW *w, int cols, int color, char *s)
{
	char buf[400];
	unsigned long i, skp;

	skp = (cols/2) - strlen(s)/2;
	for (i=0; i<skp; i++)
		buf[i] = ' ';
	buf[i] = '\0';
	strcat (buf, s);
	buf[i] = (islower(buf[i]))? toupper(buf[i]): buf[i];
	for (i=strlen(buf); i<cols; i++)
		buf[i] = ' ';
	buf[i] = '\0';
	if( color > 0 )
		wattron(w,COLOR_PAIR(color));
	wprintw (w, "%s", buf);
	if( color > 0 )
		wattroff(w,COLOR_PAIR(color));
}

void catch ()
{
	signal(SIGINT, SIG_IGN);
	move(MAXROWS-1, 0);
	printw("\n");
	nocbreak();
	echo();
	refresh();
	endwin ();
	exit (0);
}

#include	<pwd.h>

char *getshell ()
{
	char *t = (char *) getenv("SHELL");
	struct passwd *pass = getpwuid(getuid());

	if (t == NULL) {
		if (pass -> pw_shell[0] == '\0')	/* RJS */
			return ("/bin/sh");		/* RJS */
		else					/* RJS */
			return (pass -> pw_shell);
	} else {
		return (t);
	}
}

int pid;

FILE *mypopen (char *s, char *shell)
{
	FILE *fp;
	int p[2];

	pipe (p);

	if ((pid = fork()) == 0) {
		close (1);
		close (2);
		dup (p[1]);
		dup (p[1]);
		close (p[0]);
		close (p[1]);

		execl (shell, shell, "-c", s, (char *)0);
		perror (shell);
		exit (1);
	} else if (pid < 0) {
		perror ("fork");
		exit (1);
	} else {
		fp = fdopen (p[0], "r");
		close (p[1]);
	}
	return (fp);
}

int mypclose (FILE *fp)
{
	int i, status;

	while ((i = wait (&status)) != pid && i != -1);

	return fclose (fp);
}
