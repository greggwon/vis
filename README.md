# VIS
A curses visualization tool used for watching and monitoring file and command changes

This program was developed back in the 1980's as a means for monitoring logs and build processes and lots of other things that UNIX administrators get to do.
Mark Vasoll did the initial development and it has been touched and enhanced by several different people over all the years of its use.

## Building
Just use the Makefile to build it.  It should build in all places that have support for the C-Language and have ncurses installed. On older systems,
ncurses might need to be changed to just curses.

## Usage
The -t argument has a parameter which is the number of seconds to delay between invocations of all the commands.  We've talked about a version that allows
different update times for each command line, but have never made that happen.

## Examples
There are lots of things to look at on a UNIX/Linux system.  A lot of things are wired up and watched with SNMP and other tool sets.  If you can formulate
a script to get you the details, putting together several command lines like shown below can be handy too.

### system logging
vis -t 10 'tail /var/log/syslog' 'tail /var/log/dmesg'

### users comming and going and accessing shared files
vis -t 10 'w' 'tail /var/log/samba/log.smbd' 'tail /var/log/samba/log.nmbd'
