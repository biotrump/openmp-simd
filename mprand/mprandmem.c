/* 
 cc  mprandmem.c cpuidc.c -lrt -lc -lm -O3 -o MP-RandMem
 #define version "Linux/ARM v1.0"

 gcc 4.8
  gcc mprandmem.c cpuidc.c -lrt -lc -lm -O3 -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -funsafe-math-optimizations -lpthread -o MP-RandMemPiA7
 ./MP-RandMemPiA7
 #define version "Linux/ARM V7A v1.0"

 Affinity Setting - use 1 CPU
 taskset 0x00000001 ./MP-RandMemPiA7

 */
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <time.h>
 #include "cpuidh.h"
 #include <pthread.h>
 
 // #define version "Linux/ARM v1.0"
 #define version "Linux/ARM V7A v1.0"

int     xi[3300000]; // for 12.8 MB
int     extraLoops;
int     memoryWords;
int     threads = 1;
int     testToRun;

unsigned int     result[8];

pthread_t tid[100]; 
pthread_attr_t * attrt = NULL; 
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

double  runSecs = 0.5;
double  startSecs;
double  secs;

char    resultchars[1000];


//  ************** PROTOPTYPES *****************

void thread1TestRW(int xiwr[], int t);    
void thread1TestRD(int xird[], int t);


void *runTests(void *arg)
{
    long thread;

    thread = (long)arg;

    if (testToRun == 1) thread1TestRD(xi, thread);
    if (testToRun == 2) thread1TestRW(xi, thread);
    if (testToRun == 3) thread1TestRD(xi, thread);
    if (testToRun == 4) thread1TestRW(xi, thread);
}

void checkTime()
{
    if (secs < runSecs)
    {
        if (secs < runSecs / 8.0)
        {
             extraLoops = extraLoops * 10;
        }
        else
        {
             extraLoops = (int)(runSecs * 1.25 / secs * (double)extraLoops+1);
        }
    }
}


void main()
{
    int     words[3];
    int     mbpersec;
    int     mbps[48];
    int     g, i, w, k, t;
    int     wds;
    int     res;

    char    errorMsg[120];
 
    words[0]   = 3072;           // 3200
    words[1]   = words[0] * 10;  // 32000
    words[2]   = words[1] * 100; // 3200000

    for (i=0; i<48; i++)
    {
        mbps[i] = 0;
    }
    
    sprintf(errorMsg, "     No Errors Found");
    res = 0;

    FILE    *outfile;

    outfile = fopen("MP-RndMem.txt","a+");
    if (outfile == NULL)
    {
        printf ("Cannot open results file \n\n");
        printf(" Press Enter\n");
        g  = getchar();
        exit (0);
    }
    printf("\n");
    getDetails();
    local_time();     

    printf(" ##########################################\n"); 
    fprintf (outfile, " #####################################################\n");                     

    printf ("\nFrom File /proc/cpuinfo\n");
    printf("%s\n", configdata[0]);
    printf ("\nFrom File /proc/version\n");
    printf("%s\n", configdata[1]);

    printf("\n MP-RandMem %s %s\n", version, timeday);
    printf("  MB/Second Using 1, 2, 4 and 8 Threads\n\n");
    printf("  KB       SerRD SerRDWR   RndRD RndRDWR\n\n");

    fprintf(outfile, "\n MP-RandMem %s %s\n", version, timeday);
    fprintf(outfile, "  MB/Second Using 1, 2, 4 and 8 Threads\n\n");
    fprintf(outfile, "  KB       SerRD SerRDWR   RndRD RndRDWR\n\n");

    for (wds=0;wds<3; wds++)
    {       
        memoryWords = words[wds];

        if(wds == 0) printf(" 12.3 1T");
        if(wds == 1) printf("122.9 1T");
        if(wds == 2) printf("12288 1T");
        if(wds == 0) fprintf(outfile, " 12.3 1T");
        if(wds == 1) fprintf(outfile, "122.9 1T");
        if(wds == 2) fprintf(outfile, "12288 1T");

        for (threads=1; threads<9; threads=threads*2)
        {
            if (threads == 2) printf("      2T");
            if (threads == 4) printf("      4T");
            if (threads == 8) printf("      8T");
            if (threads == 2) fprintf(outfile, "      2T");
            if (threads == 4) fprintf(outfile, "      4T");
            if (threads == 8) fprintf(outfile, "      8T");

            for (testToRun=1; testToRun<5; testToRun++)
            {
                if (testToRun == 1 || testToRun == 2)
                {
                    for (k=0; k<memoryWords+1; k++)
                    {
                        xi[k] = k+1;
                    }
                }
                else
                {
                    srand(123);
    
                    for (k=0; k<memoryWords+1; k++)
                    {
                       xi[k] = (int)((double)memoryWords * ((double)(rand()) / (double)2147483648.0));
                    }                
                }

                extraLoops = 1;
                do
                {
                   start_time();
                   for (t=0; t<threads; t++)
                   {
                       result[t] = 12345;
                       pthread_create(&tid[t], attrt, runTests, (void *)t);
                   }
        
                   for(t=0;t<threads;t++)
                   {
                      pthread_join(tid[t], NULL);
                   }
                   end_time();
                   checkTime();
               }
               while (secs < runSecs);
               mbpersec = (int)((double)threads * (double)extraLoops * (double)memoryWords * 4 / secs / 1000000);
               mbps[res] = mbpersec;
               res = res + 1;
               if (testToRun == 1)
               {
                    for (k=0; k<threads; k++)
                    {
                        if (result[k] != result[0])
                        {
                           sprintf(errorMsg, "     Inconsistent Results");
                        }
                    }
               }
               printf("%8d", mbpersec);
               fflush(stdout);                               
            }
            printf("\n");            
            fprintf(outfile, "%8d%8d%8d%8d\n", mbps[0], mbps[1], mbps[2], mbps[3]);
            res = 0;
            fflush(stdout);                               
            fflush(outfile);                                           
        }
    }
    printf("\n%s\n\n", errorMsg);
    fprintf(  outfile, "\n%s\n\n", errorMsg);
           
   local_time();
   printf("   End of test %s\n", timeday);
   fprintf(outfile, "    End of test %s", timeday);        

   fprintf (outfile, "\nSYSTEM INFORMATION\n\nFrom File /proc/cpuinfo\n");
   fprintf (outfile, "%s \n", configdata[0]);
   fprintf (outfile, "\nFrom File /proc/version\n");
   fprintf (outfile, "%s \n", configdata[1]);
   fprintf (outfile, "\n");
   fflush(outfile);                

   char moredata[1024];
   printf("Type additional information to include in MP-RndMem.txt - Press Enter\n");
   if (fgets (moredata, sizeof(moredata), stdin) != NULL)
             fprintf (outfile, "Additional information - %s\n", moredata);                
   fflush(stdout);                
   fclose(outfile);
   return;    
}

void thread1TestRD(int xird[], int thread)    
{
  int  i, j, k;
  int  start, end, length;

  length = (int)(memoryWords/threads);
  start  = thread * length;
  end    = start  + length;
  for (j=0; j<extraLoops; j++)
  {
      for (k=0; k<memoryWords/length; k++)
      {
        for (i=start; i<end; i = i + 64)
        {
          result[thread] = result[thread]
            + xird[xird[i+   0]] | xird[xird[i+   2]]
            - xird[xird[i+   4]] | xird[xird[i+   6]]
            + xird[xird[i+   8]] | xird[xird[i+  10]]
            - xird[xird[i+  12]] | xird[xird[i+  14]]
            + xird[xird[i+  16]] | xird[xird[i+  18]]
            - xird[xird[i+  20]] | xird[xird[i+  22]]
            + xird[xird[i+  24]] | xird[xird[i+  26]]
            - xird[xird[i+  28]] | xird[xird[i+  30]]
            + xird[xird[i+  32]] | xird[xird[i+  34]]
            - xird[xird[i+  36]] | xird[xird[i+  38]]
            + xird[xird[i+  40]] | xird[xird[i+  42]]
            - xird[xird[i+  44]] | xird[xird[i+  46]]
            + xird[xird[i+  48]] | xird[xird[i+  50]]
            - xird[xird[i+  52]] | xird[xird[i+  54]]
            + xird[xird[i+  56]] | xird[xird[i+  58]]
            - xird[xird[i+  60]] | xird[xird[i+  62]];
        }
        end = end + length;
        if (end > memoryWords) end = length;
        start = end - length;
      }
  }    
  return;
}

void thread1TestRW(int xiwr[], int thread)    
{
  int i, j, k;
  int strtWRW, lastWRW;
  int  start, end, length;

  length = (int)(memoryWords/threads);
  start  = thread * length;
  end    = start  + length;

  pthread_mutex_lock( &mutex1 );

  for (j=0; j<extraLoops; j++)
  {
      for (k=0; k<memoryWords/length; k++)
      {
        for (i=start; i<end; i = i + 64)
        {
             xiwr[xiwr[i+   0]] = xiwr[xiwr[i+   2]];
             xiwr[xiwr[i+   4]] = xiwr[xiwr[i+   6]];
             xiwr[xiwr[i+   8]] = xiwr[xiwr[i+  10]];
             xiwr[xiwr[i+  12]] = xiwr[xiwr[i+  14]];
             xiwr[xiwr[i+  16]] = xiwr[xiwr[i+  18]];
             xiwr[xiwr[i+  20]] = xiwr[xiwr[i+  22]];
             xiwr[xiwr[i+  24]] = xiwr[xiwr[i+  26]];
             xiwr[xiwr[i+  28]] = xiwr[xiwr[i+  30]];
             xiwr[xiwr[i+  32]] = xiwr[xiwr[i+  34]];
             xiwr[xiwr[i+  36]] = xiwr[xiwr[i+  38]];
             xiwr[xiwr[i+  40]] = xiwr[xiwr[i+  42]];
             xiwr[xiwr[i+  44]] = xiwr[xiwr[i+  46]];
             xiwr[xiwr[i+  48]] = xiwr[xiwr[i+  50]];
             xiwr[xiwr[i+  52]] = xiwr[xiwr[i+  54]];
             xiwr[xiwr[i+  56]] = xiwr[xiwr[i+  58]];
             xiwr[xiwr[i+  60]] = xiwr[xiwr[i+  62]];
        }
        end = end + length;
        if (end > memoryWords) end = length;
        start = end - length;
      }
  }
  pthread_mutex_unlock( &mutex1 );
  return;
}




