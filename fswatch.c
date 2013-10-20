#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <CoreServices/CoreServices.h>

/* fswatch.c
 * 
 * usage: ./fswatch -h -q -s -r -p /some/directory[:/some/otherdirectory:...]
 *
 * example: ./fswatch --help --silent --quit --debug --ignore .DS_store:.idea:.git --p .
 *
 * compile me with something like: gcc fswatch.c -framework CoreServices -o fswatch
 *
 * adapted from the FSEvents api PDF
*/

extern char **environ;

//command line stuff
static int quit_flag = 0; //quit
static int silent_flag = 0; //silent
static int path_flag = 0; //path
static int help_flag = 0; //help
static int debug_flag = 0; //debug
static int ignore_flag = 0; //ignore
static char *path_value = ""; //paths
static char *ignore_value = ""; //ignores

static char seps[]   = ":";
static char **ignores = NULL;
static int ignore_size = 0;


static struct option long_options[] =
         {
           {"help",         no_argument,       &help_flag, 1},
           {"quit",         no_argument,       &quit_flag, 1},
           {"silent",       no_argument,       &silent_flag, 1},
           {"debug",        no_argument,       &debug_flag, 1},
           {"ignore",       required_argument, &ignore_flag, 1},
           {"path",         required_argument, &path_flag, 1},
           {0, 0, 0, 0}
         };

struct option_desc {
    #if defined (__STDC__) && __STDC__
      const char *name;
      const char *desc;
    #else
      char *name;
      char *desc;
    #endif
};

static struct option_desc option_desc[] =
    {
        {"help",         "help"},
        {"quit",         "quit after first event"},
        {"silent",       "silent"},
        {"debug",        "debug"},
        {"ignore",       "ignore paths with this patter, colon(:) separated"},
        {"path",         "watch these paths, colon(:) separated"},
        {"", ""}
    };

// call back from OS API, write out some info when there's any change in watched files
void callback( 
    ConstFSEventStreamRef streamRef, 
    void *clientCallBackInfo, 
    size_t numEvents, 
    void *eventPaths, 
    const FSEventStreamEventFlags eventFlags[], 
    const FSEventStreamEventId eventIds[]) 
{ 
  int quitReady =0; //found something worthwhile

    if (debug_flag==1) {
        printf("notified with %x events\n", (unsigned int)numEvents);
    }

  for (int i=0; i<numEvents; ++i) {
    //check ignores

    if (debug_flag==1) {
        printf("event #%i %x %s\n", i, eventFlags[i], ((char **)eventPaths)[i]);
    }

    if (ignore_flag==1) {
        int ignoreMatch = 0;

        for (int j = 0; j < (ignore_size+1); ++j) {
            if (ignores[j] != NULL) {
                if (strstr(((char **)eventPaths)[i], ignores[j]) != NULL) {
                    // contains
                    if (debug_flag==1) {
                        printf("\tignore this event\n");
                    }

                    ignoreMatch=1;
                    continue;
                }
            }
        }

        if (ignoreMatch==1) {
            if (debug_flag==1) {
                printf("\t\tignoring\n");
            }
            continue;
        } else {
            if (debug_flag==1) {
                printf("\t\tnot ignoring\n");
            }
            quitReady = 1;
        }
    }
    if (silent_flag==0) {
        printf("%x %s\n", eventFlags[i], ((char **)eventPaths)[i]);
    }
  }

  fflush(stdout);

  if (quitReady==1) {
      if (quit_flag == 1) {
        if (debug_flag==1) {
            printf("done\n");
        }
        free (ignores);
        exit(EXIT_SUCCESS);
      }
  }
} 


void doPrintConfig() {
    printf ("provided config:\n\thelp = %s\n\tquit = %s\n\tsilent = %s\n\tdebug = %s\n\tignore = %s\n\tpath to watch = %s [%s]\n\n", (help_flag==1?"true":"false"), (quit_flag==1?"true":"false"),(silent_flag==1?"true":"false"), (debug_flag==1?"true":"false"), (ignore_flag==1?"true":"false"), (path_flag==1?"true":"false"), path_value);

    printf ("ignore list:\n");
    for (int i = 0; i < (ignore_size+1); ++i) {
        if (ignores[i]!=NULL) {
            printf ("\t%s\n", ignores[i]);
        }
    }
    printf ("\n");
}


void doPrintUsage() {
    printf ("fswatch (Mac OSX)\n");
    printf ("usage: ./fswatch --[options] -i pattern[:pattern] -p /some/directory[:/some/otherdirectory:...]\n\n");

    printf ("options:\n");

    int options = sizeof(long_options) / sizeof(long_options[0]);
    for (int i = 0; i< options; i++) {
        if (long_options[i].name == NULL) {
        continue;
        }
        printf("\t%s = %s\n",  long_options[i].name, option_desc[i].desc);
    }

    printf ("\n");

    doPrintConfig();
}

//set up fsevents and callback
int main(int argc, char **argv) {

    int index;
    int c;

    opterr = 0;


    int option_index = 0;

   while ((c = getopt_long_only (argc, argv, "",long_options, &option_index)) != -1)
     switch (c)
       {
       break;
         case 0:
           /* If this option set a flag, do nothing else now. */
           if (long_options[option_index].flag != 0 & long_options[option_index].has_arg == no_argument) {
             break;
           } else {
               if (strstr(long_options[option_index].name,"ignore")!=NULL) {
                    ignore_value = optarg;

                    char *stringPart = strtok ( ignore_value, seps );

                    while (stringPart != NULL) {
                      ignores = realloc (ignores, sizeof (char*) * ++ignore_size);

                      if (ignores == NULL) {
                        exit (-1); /* memory allocation failed */
                      }

                      ignores[ignore_size-1] = stringPart;

                      stringPart = strtok ( NULL , seps );
                    }

                    ignores = realloc (ignores, sizeof (char*) * (ignore_size+1));
                    ignores[ignore_size] = 0;

//                    for (int i = 0; i < (ignore_size+1); ++i) {
//                      printf ("ignores[%d] = %s\n", i, ignores[i]);
//                    }

                } else if (strstr(long_options[option_index].name,"path")!=NULL) {
                    path_value = optarg;
               }
           }
           break;

       case '?':
        break;
       default:
        abort();
       }

    if (silent_flag==0) {
        for (int index = optind; index < argc; index++) {
         printf ("Non-option argument %s\n", argv[index]);
        }
    }

    if (help_flag==1) {
        doPrintUsage();
        exit(EXIT_SUCCESS);
    }

    if (strcmp (path_value,"") != 0) {
        if (silent_flag==0) {
            printf ("\nwatching...\n\n");
        }

        //CALL OS API
        CFStringRef watchPath = CFStringCreateWithCString(NULL, path_value, kCFStringEncodingUTF8);
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

