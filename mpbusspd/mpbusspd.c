/*
cc  mpbusspd.c cpuidc.c -lrt -lc -lm -O3 -o MP-BusSpd
 #define version "Linux/ARM v1.0"

 gcc 4.8
 gcc mpbusspd.c cpuidc.c -lrt -lc -lm -O3 -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -lpthread -o MP-BusSpdPiA7
 #define version "Linux/ARM V7A v1.0"
 ./MP-BusSpdPiA7
 
 taskset 0x00000001 ./MP-BusSpdPiA7
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <inttypes.h>
 #include <stdint.h>
 #include "cpuidh.h"
 #include <time.h>
 #include <pthread.h> 

// #define version "Linux/ARM v1.0"
 #define version "Linux/ARM V7A v1.0"

 pthread_t tid[100]; 
 pthread_attr_t * attrt = NULL; 

double  runSecs = 0.1;
double  startSecs;
double  secs;

char    resultchars[1000];

volatile int testToRun;
volatile int passCount;
volatile int wordsToTest;

int     andsum1[10];
int     array[3300000]; // 12.8 MB+
int     words[3]; 
int     repeats[3]; 
int     andsumx = 0x55555555;


void inc32words(int t);
void inc16words(int t);
void inc8words(int t);
void inc4words(int t);
void inc2words(int t);
void inc1word(int t);
void checkTime();

void *runAll(void *arg)
{
    long thread;

    thread = (long)arg;
    if (testToRun == 1) inc32words(thread);
    if (testToRun == 2) inc16words(thread);
    if (testToRun == 3) inc8words(thread);
    if (testToRun == 4) inc4words(thread);
    if (testToRun == 5) inc2words(thread);
    if (testToRun == 6) inc1word(thread);
    return;
}



void main()
{
    int     threads;
    int     wds;
    int     g, i, s, t, w;
    int     mbpersec;
    int     mbps[72];
    int     res;
    char    errorMsg[40];
    double  addressInc[10];

    words[0]   = 3072;           // 3200
    words[1]   = words[0] * 10;  // 32000
    words[2]   = words[1] * 100; // 3200000

    addressInc[1] = 32; // inc32words
    addressInc[2] = 16; // inc16words
    addressInc[3] = 8;  // inc8words
    addressInc[4] = 4;  // inc4words
    addressInc[5] = 2;  // inc2words
    addressInc[6] = 1;  // inc1word
    
    for (i=0; i<72; i++)
    {
        mbps[i] = 0;
    }
    
    sprintf(errorMsg, "         No Errors Found");
    res = 0;

    FILE    *outfile;
    
    outfile = fopen("MP-BusSpd.txt","a+");
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
        
    printf("\n MP-BusSpd %s %s\n", version, timeday);
    printf("   MB/Second Reading Data, 1, 2, 4 and 8 Threads\n\n");
    printf("  KB      Inc32  Inc16   Inc8   Inc4   Inc2  RdAll\n\n");

    fprintf(outfile, "\n MP-BusSpd %s %s\n", version, timeday);
    fprintf(outfile, "   MB/Second Reading Data, 1, 2, 4 and 8 Threads\n\n");
    fprintf(outfile, "  KB      Inc32  Inc16   Inc8   Inc4   Inc2  RdAll\n\n");

    for (wds=0;wds<3; wds++)
    {
        if(wds == 0) printf(" 12.3 1T");
        if(wds == 1) printf("122.9 1T");
        if(wds == 2) printf("12288 1T");
        if(wds == 0) fprintf(outfile, " 12.3 1T");
        if(wds == 1) fprintf(outfile, "122.9 1T");
        if(wds == 2) fprintf(outfile, "12288 1T");
        
        for (w=0; w<words[wds]; w++)
        {
            array[w] = andsumx;
        }
        wordsToTest = words[wds];
        
        for (threads=1; threads<9; threads=threads*2)
        {
            if (threads == 2) printf("      2T");
            if (threads == 4) printf("      4T");
            if (threads == 8) printf("      8T");
            if (threads == 2) fprintf(outfile, "      2T");
            if (threads == 4) fprintf(outfile, "      4T");
            if (threads == 8) fprintf(outfile, "      8T");
            for (s=0; s<threads; s++)
            {
                andsum1[s] = andsumx;
            }
            for (testToRun=1; testToRun<7; testToRun++) 
            {
                passCount = 1;        
                do
                {
                   start_time();
                   for (t=0; t<threads; t++)
                   {
                       pthread_create(&tid[t], attrt, runAll, (void *)t);
                   }
        
                   for(t=0;t<threads;t++)
                   {
                      pthread_join(tid[t], NULL);
                   }
                   end_time();
                   checkTime();
               }
               while (secs < runSecs);
                   
               mbpersec = (int)((double)threads * (double)passCount * wordsToTest * 4 / secs / addressInc[testToRun] / 1000000);
               mbps[res] = mbpersec;
               res = res + 1;

               for(t=0;t<threads;t++)
               {
                   if (andsum1[t] != andsumx)
                   {
                       sprintf(errorMsg, "         ERRORS found on sum check ");
                   }
               }
               printf("%7d", mbpersec);
               fflush(stdout);                               
            }
            printf("\n");            
            fprintf(outfile, "%7d%7d%7d%7d%7d%7d\n", mbps[0], mbps[1], mbps[2], mbps[3], mbps[4], mbps[5]);
            res = 0;
            fflush(stdout);                               
            fflush(outfile);                               
        }    
    }
    printf("\n%s\n\n", errorMsg);
    fprintf(  outfile, "\n%s\n\n", errorMsg);
           
   local_time();
   printf("   End of test %s\n", timeday);
   fprintf(outfile, "         End of test %s", timeday);        

   fprintf (outfile, "\nSYSTEM INFORMATION\n\nFrom File /proc/cpuinfo\n");
   fprintf (outfile, "%s \n", configdata[0]);
   fprintf (outfile, "\nFrom File /proc/version\n");
   fprintf (outfile, "%s \n", configdata[1]);
   fprintf (outfile, "\n");
   fflush(outfile);                

   char moredata[1024];
   printf("Type additional information to include in MP-BusSpd.txt - Press Enter\n");
   if (fgets (moredata, sizeof(moredata), stdin) != NULL)
             fprintf (outfile, "Additional information - %s\n", moredata);     fflush(stdout);                
   fflush(stdout);                
   fclose(outfile);
   return;
} 


void checkTime()
{
 if (secs < runSecs)
 {
      if (secs < runSecs / 8.0)
      {
            passCount = passCount * 10;
      }
      else
      {
            passCount = (int)(runSecs * 1.25 / secs * (double)passCount+1);
      }
  }
}


void inc32words(int t)
{
   int i, j;

   for(j=0; j<passCount; j++)
   {
       for (i=0; i<wordsToTest; i=i+768)
       {
           andsum1[t] = andsum1[t] & array[i     ] & array[i+32  ] & array[i+64  ] & array[i+96  ]
                                   & array[i+128 ] & array[i+160 ] & array[i+192 ] & array[i+224 ]
                                   & array[i+256 ] & array[i+288 ] & array[i+320 ] & array[i+352 ]
                                   & array[i+384 ] & array[i+416 ] & array[i+448 ] & array[i+480 ]
                                   & array[i+512 ] & array[i+544 ] & array[i+576 ] & array[i+608 ]
                                   & array[i+640 ] & array[i+672 ] & array[i+704 ] & array[i+736 ];
       }
   }
}

void inc16words(int t)
{
   int i, j;

   for(j=0; j<passCount; j++)
   {
       for (i=0; i<wordsToTest; i=i+768)
       {
           andsum1[t] = andsum1[t] & array[i    ] & array[i+16 ] & array[i+32 ] & array[i+48 ]
                                   & array[i+64 ] & array[i+80 ] & array[i+96 ] & array[i+112]
                                   & array[i+128] & array[i+144] & array[i+160] & array[i+176]
                                   & array[i+192] & array[i+208] & array[i+224] & array[i+240]
                                   & array[i+256] & array[i+272] & array[i+288] & array[i+304]
                                   & array[i+320] & array[i+336] & array[i+352] & array[i+368]
                                   & array[i+384] & array[i+400] & array[i+416] & array[i+432]
                                   & array[i+448] & array[i+464] & array[i+480] & array[i+496]
                                   & array[i+512] & array[i+528] & array[i+544] & array[i+560]
                                   & array[i+576] & array[i+592] & array[i+608] & array[i+624]
                                   & array[i+640] & array[i+656] & array[i+672] & array[i+688]
                                   & array[i+704] & array[i+720] & array[i+736] & array[i+752];
       }
   }
}

void inc8words(int t)
{
   int i, j;

   for(j=0; j<passCount; j++)
   {
       for (i=0; i<wordsToTest; i=i+384)
       {
           andsum1[t] = andsum1[t] & array[i    ] & array[i+8  ] & array[i+16 ] & array[i+24 ]
                                   & array[i+32 ] & array[i+40 ] & array[i+48 ] & array[i+56 ]
                                   & array[i+64 ] & array[i+72 ] & array[i+80 ] & array[i+88 ]
                                   & array[i+96 ] & array[i+104] & array[i+112] & array[i+120]
                                   & array[i+128] & array[i+136] & array[i+144] & array[i+152]
                                   & array[i+160] & array[i+168] & array[i+176] & array[i+184]
                                   & array[i+192] & array[i+200] & array[i+208] & array[i+216]
                                   & array[i+224] & array[i+232] & array[i+240] & array[i+248]
                                   & array[i+256] & array[i+264] & array[i+272] & array[i+280]
                                   & array[i+288] & array[i+296] & array[i+304] & array[i+312]
                                   & array[i+320] & array[i+328] & array[i+336] & array[i+344]
                                   & array[i+352] & array[i+360] & array[i+368] & array[i+376];
       }
   }
}

void inc4words(int t)
{
   int i, j;

   for(j=0; j<passCount; j++)
   {
       for (i=0; i<wordsToTest; i=i+256)
       {
           andsum1[t] = andsum1[t] & array[i    ] & array[i+4  ] & array[i+8  ] & array[i+12 ]
                                   & array[i+16 ] & array[i+20 ] & array[i+24 ] & array[i+28 ]
                                   & array[i+32 ] & array[i+36 ] & array[i+40 ] & array[i+44 ]
                                   & array[i+48 ] & array[i+52 ] & array[i+56 ] & array[i+60 ]
                                   & array[i+64 ] & array[i+68 ] & array[i+72 ] & array[i+76 ]
                                   & array[i+80 ] & array[i+84 ] & array[i+88 ] & array[i+92 ]
                                   & array[i+96 ] & array[i+100] & array[i+104] & array[i+108]
                                   & array[i+112] & array[i+116] & array[i+120] & array[i+124]
                                   & array[i+128] & array[i+132] & array[i+136] & array[i+140]
                                   & array[i+144] & array[i+148] & array[i+152] & array[i+156]
                                   & array[i+160] & array[i+164] & array[i+168] & array[i+172]
                                   & array[i+176] & array[i+180] & array[i+184] & array[i+188]
                                   & array[i+192] & array[i+196] & array[i+200] & array[i+204]
                                   & array[i+208] & array[i+212] & array[i+216] & array[i+220]
                                   & array[i+224] & array[i+228] & array[i+232] & array[i+236]
                                   & array[i+240] & array[i+244] & array[i+248] & array[i+252];
       }
   }
}

void inc2words(int t)
{
   int i, j;

   for(j=0; j<passCount; j++)
   {
       for (i=0; i<wordsToTest; i=i+128)
       {
           andsum1[t] = andsum1[t] & array[i    ] & array[i+2  ] & array[i+4  ] & array[i+6  ]
                                   & array[i+8  ] & array[i+10 ] & array[i+12 ] & array[i+14 ]
                                   & array[i+16 ] & array[i+18 ] & array[i+20 ] & array[i+22 ]
                                   & array[i+24 ] & array[i+26 ] & array[i+28 ] & array[i+30 ]
                                   & array[i+32 ] & array[i+34 ] & array[i+36 ] & array[i+38 ]
                                   & array[i+40 ] & array[i+42 ] & array[i+44 ] & array[i+46 ]
                                   & array[i+48 ] & array[i+50 ] & array[i+52 ] & array[i+54 ]
                                   & array[i+56 ] & array[i+58 ] & array[i+60 ] & array[i+62 ]
                                   & array[i+64 ] & array[i+66 ] & array[i+68 ] & array[i+70 ]
                                   & array[i+72 ] & array[i+74 ] & array[i+76 ] & array[i+78 ]
                                   & array[i+80 ] & array[i+82 ] & array[i+84 ] & array[i+86 ]
                                   & array[i+88 ] & array[i+90 ] & array[i+92 ] & array[i+94 ]
                                   & array[i+96 ] & array[i+98 ] & array[i+100] & array[i+102]
                                   & array[i+104] & array[i+106] & array[i+108] & array[i+110]
                                   & array[i+112] & array[i+114] & array[i+116] & array[i+118]
                                   & array[i+120] & array[i+122] & array[i+124] & array[i+126];
       }
    }
}

void inc1word(int t)
{
   int i, j;

   for(j=0; j<passCount; j++)
   {
       for (i=0; i<wordsToTest; i=i+64)
       {
           andsum1[t] = andsum1[t] & array[i   ] & array[i+1 ] & array[i+2 ] & array[i+3 ]
                                   & array[i+4 ] & array[i+5 ] & array[i+6 ] & array[i+7 ]
                                   & array[i+8 ] & array[i+9 ] & array[i+10] & array[i+11]
                                   & array[i+12] & array[i+13] & array[i+14] & array[i+15]
                                   & array[i+16] & array[i+17] & array[i+18] & array[i+19]
                                   & array[i+20] & array[i+21] & array[i+22] & array[i+23]
                                   & array[i+24] & array[i+25] & array[i+26] & array[i+27]
                                   & array[i+28] & array[i+29] & array[i+30] & array[i+31]
                                   & array[i+32] & array[i+33] & array[i+34] & array[i+35]
                                   & array[i+36] & array[i+37] & array[i+38] & array[i+39]
                                   & array[i+40] & array[i+41] & array[i+42] & array[i+43]
                                   & array[i+44] & array[i+45] & array[i+46] & array[i+47]
                                   & array[i+48] & array[i+49] & array[i+50] & array[i+51]
                                   & array[i+52] & array[i+53] & array[i+54] & array[i+55]
                                   & array[i+56] & array[i+57] & array[i+58] & array[i+59]
                                   & array[i+60] & array[i+61] & array[i+62] & array[i+63];
       }
   }
}


