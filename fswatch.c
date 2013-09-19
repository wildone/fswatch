#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <CoreServices/CoreServices.h>

/* fswatch.c
 * 
 * usage: ./fswatch -h -q -s -p /some/directory[:/some/otherdirectory:...]
 *
 * compile me with something like: gcc fswatch.c -framework CoreServices -o fswatch
 *
 * adapted from the FSEvents api PDF
*/

extern char **environ;
int qflag = 0; //quit
int sflag = 0; //silent
int pflag = 0; //path
int hflag = 0; //help
char *pvalue = "";

// call back from OS API, write out some info when there's any change in watched files
void callback( 
    ConstFSEventStreamRef streamRef, 
    void *clientCallBackInfo, 
    size_t numEvents, 
    void *eventPaths, 
    const FSEventStreamEventFlags eventFlags[], 
    const FSEventStreamEventId eventIds[]) 
{ 

  for (int i=0; i<numEvents; ++i) {
	printf("%x %s; ", eventFlags[i], ((char **)eventPaths)[i]);
    printf("\n");
  }
  fflush(stdout);

  if (qflag == 1) {
    exit(1);
  }
} 

void doPrintUsage() {
    printf ("\nusage: ./fswatch [options] -p /some/directory[:/some/otherdirectory:...]\n\n");
    printf ("options:\n\th\thelp\n\tq\tquit after first change\n\ts\tsilent\n\n");
    printf ("directory:\n\tp\tdirectory to watch\n\n");
}

void doPrintConfig() {
    printf ("config:\n\tquit after first change = %s,\n\tsilent = %s\n\thelp = %s\n\tpath to watch = [%s]\n\n", (qflag==1?"true":"false"),(sflag==1?"true":"false"),(hflag==1?"true":"false"), pvalue);
}

//set up fsevents and callback
int main(int argc, char **argv) {

    int index;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "hqsp:")) != -1)
     switch (c)
       {
       case 'h':
        hflag = 1;
        break;
       case 's':
         sflag = 1;
         break;
        case 'q':
         qflag = 1;
         break;
       case 'p':
         pvalue = optarg;
         break;
       case '?':
         if (optopt == 'p')
           fprintf (stderr, "Option -%c requires a directory argument.\n", optopt);
         else if (isprint (optopt))
           fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         else
           fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
         return 1;
       default:
        abort();
       }

    if (sflag==0 | hflag==1) {
        doPrintConfig();

        for (index = optind; index < argc; index++) {
         printf ("Non-option argument %s\n", argv[index]);
        }
    }

    if (hflag==1) {
        doPrintUsage();
        exit(1);
    }

    if (strcmp (pvalue,"") != 0) {
        if (sflag==0) {
            printf ("\nwatching...\n\n");
        }

        //CALL OS API
        CFStringRef watchPath = CFStringCreateWithCString(NULL, pvalue, kCFStringEncodingUTF8);
        CFArrayRef pathsToWatch = CFStringCreateArrayBySeparatingStrings (NULL, watchPath, CFSTR(":"));

        void *callbackInfo = NULL;
        FSEventStreamRef stream;
        CFAbsoluteTime latency = 1.0;

        stream = FSEventStreamCreate(NULL,
        &callback,
        callbackInfo,
        pathsToWatch,
        kFSEventStreamEventIdSinceNow,
        latency,
        kFSEventStreamCreateFlagFileEvents
        );

        FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        FSEventStreamStart(stream);
        CFRunLoopRun();
    } else {
        doPrintUsage();
        printf ("no path specified\n");
    }

}

