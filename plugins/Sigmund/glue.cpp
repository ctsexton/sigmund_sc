#include "sigmund.c"
#include <iostream>
#include <string>

#define NHIST 100

#define MODE_STREAM 1
#define MODE_BLOCK 2        /* unimplemented */
#define MODE_TABLE 3

#define NPOINTS_DEF 1024
#define NPOINTS_MIN 128
#define NPOINTS_MAX 4194304

#define HOP_DEF 512
#define NPEAK_DEF 20

#define VIBRATO_DEF 1
#define STABLETIME_DEF 50
#define MINPOWER_DEF 50
#define GROWTH_DEF 7

#define OUT_PITCH 0
#define OUT_ENV 1
#define OUT_NOTE 2
#define OUT_PEAKS 3
#define OUT_TRACKS 4
#define OUT_SMSPITCH 5
#define OUT_SMSNONPITCH 6

void loginfo (std::string output) {
  std::cout << output << std::endl;
}


typedef struct _sigmund
{
    t_float x_sr;       /* sample rate */
    int x_mode;         /* MODE_STREAM, etc. */
    int x_npts;         /* number of points in analysis window */
    int x_npeak;        /* number of peaks to find */
    int x_loud;         /* debug level */
    t_sample *x_inbuf;  /* input buffer */
    int x_infill;       /* number of points filled */
    int x_countdown;    /* countdown to start filling buffer */
    int x_hop;          /* samples between analyses */ 
    t_float x_maxfreq;    /* highest-frequency peak to report */ 
    t_float x_vibrato;    /* vibrato depth in half tones */ 
    t_float x_stabletime; /* period of stability needed for note */ 
    t_float x_growth;     /* growth to set off a new note */ 
    t_float x_minpower;   /* minimum power, in DB, for a note */ 
    t_float x_param1;     /* three parameters for temporary use */
    t_float x_param2;
    t_float x_param3;
    t_float *bigbuf; /* THE BIG BUF */
    t_notefinder x_notefinder;  /* note parsing state */
    t_peak *x_trackv;           /* peak tracking state */
    int x_ntrack;               /* number of peaks tracked */
    unsigned int x_dopitch:1;   /* which things to calculate */
    unsigned int x_donote:1;
    unsigned int x_dotracks:1;
} t_sigmund;

static void sigmund_preinit(t_sigmund *x)
{
    x->x_npts = 1024; //NPOINTS_DEF
    x->x_param1 = 6;
    x->x_param2 = 0.5;
    x->x_param3 = 0;
    x->x_hop = HOP_DEF;
    x->x_mode = MODE_STREAM;
    x->x_npeak = NPEAK_DEF;
    x->x_vibrato = VIBRATO_DEF;
    x->x_stabletime = STABLETIME_DEF;
    x->x_growth = GROWTH_DEF;
    x->x_minpower = MINPOWER_DEF;
    x->x_maxfreq = 1000000;
    x->x_loud = 0;
    x->x_sr = 1;
    x->x_trackv = 0;
    x->x_ntrack = 0;
    x->x_dopitch = x->x_donote = x->x_dotracks = 0;
    x->x_inbuf = 0;
}

static void sigmund_npts(t_sigmund *x, t_floatarg f)
{
    int nwas = x->x_npts, npts = f;
        /* check parameter ranges */
    if (npts < NPOINTS_MIN)
        npts = NPOINTS_MIN;
    if (npts > NPOINTS_MAX)
        npts = NPOINTS_MAX;
    if (npts != (1 << sigmund_ilog2(npts)))
        // Adjusting analysis size
        npts = (1 << sigmund_ilog2(npts));
    if (npts != nwas)
        x->x_countdown = x->x_infill = 0;
    if (x->x_mode == MODE_STREAM)
    {
        if (x->x_inbuf)
        {
            x->x_inbuf = (t_sample *)resizebytes(x->x_inbuf,
                sizeof(*x->x_inbuf) * nwas, sizeof(*x->x_inbuf) * npts);
        }
        else
        {
            x->x_inbuf = (t_sample *)getbytes(sizeof(*x->x_inbuf) * npts);
            memset((char *)(x->x_inbuf), 0, sizeof(*x->x_inbuf) * npts);
        }
    }
    else x->x_inbuf = 0;
    x->x_npts = npts;
}

static void sigmund_hop(t_sigmund *x, t_floatarg f)
{
    int hop = f;
    if (hop < 0)
    {
        printf("sigmund~: ignoring negative hopsize %d", hop);
        return;
    }
    x->x_hop = hop;
    if (0 == hop) return;
        /* check parameter ranges */
    if (x->x_hop != (1 << sigmund_ilog2(x->x_hop)))
        printf("sigmund~: adjusting analysis size to %d points",
            (x->x_hop = (1 << sigmund_ilog2(x->x_hop))));
}

static void sigmund_npeak(t_sigmund *x, t_floatarg f)
{
    if (f < 1)
        f = 1;
    x->x_npeak = f;
}

static void sigmund_maxfreq(t_sigmund *x, t_floatarg f)
{
    x->x_maxfreq = f;
}

static void sigmund_vibrato(t_sigmund *x, t_floatarg f)
{
    if (f < 0)
        f = 0;
    x->x_vibrato = f;
}

static void sigmund_stabletime(t_sigmund *x, t_floatarg f)
{
    if (f < 0)
        f = 0;
    x->x_stabletime = f;
}

static void sigmund_growth(t_sigmund *x, t_floatarg f)
{
    if (f < 0)
        f = 0;
    x->x_growth = f;
}

static void sigmund_minpower(t_sigmund *x, t_floatarg f)
{
    if (f < 0)
        f = 0;
    x->x_minpower = f;
}

static void sigmund_doit(t_sigmund *x, int npts, t_float *arraypoints,
    int loud, t_float srate)
{
    // DYNAMIC_MEMORY_ALLOCATION
    loginfo("alloc peakv");
    t_peak *peakv = (t_peak *)alloca(sizeof(t_peak) * x->x_npeak);
    int nfound, i, cnt;
    t_float freq = 0, power, note = 0;

    // IMPORTANT STUFF:
    loginfo("get da peaks...");
    sigmund_getrawpeaks(npts, arraypoints, x->x_npeak, peakv, &nfound, &power, srate, loud, x->x_maxfreq, x->bigbuf);
    loginfo("get pitch...");
    sigmund_getpitch(nfound, peakv, &freq, npts, srate, x->x_param1, x->x_param2, loud);
    loginfo("track them peaks...");
    sigmund_peaktrack(nfound, peakv, x->x_ntrack, x->x_trackv, 2* srate / npts, loud);
    loginfo("done the sigmund");

}

static void sigmund_clear(t_sigmund *x)
{
    if (x->x_trackv)
        memset(x->x_trackv, 0, x->x_ntrack * sizeof(*x->x_trackv));
    x->x_infill = x->x_countdown = 0;
}

static void sigmund_free(t_sigmund *x)
{
    if (x->x_inbuf)
    {
        freebytes(x->x_inbuf, x->x_npts * sizeof(*x->x_inbuf));
    }
    if (x->x_trackv)
        freebytes(x->x_trackv, x->x_ntrack * sizeof(*x->x_trackv));
}


// Needs to run every time the Ugen is called,
// seems like maybe this is the real "next" function
static void sigmund_tick(t_sigmund *x)
{
    // Only do stuff when x_infill has reached the size of the analysis window
    if (x->x_infill == x->x_npts) 
    {
        sigmund_doit(x, x->x_npts, x->x_inbuf, x->x_loud, x->x_sr);
        if (x->x_hop >= x->x_npts)
        {
            x->x_infill = 0;
            x->x_countdown = x->x_hop - x->x_npts;
        }
        else
        {
            memmove(x->x_inbuf, x->x_inbuf + x->x_hop,
                (x->x_infill = x->x_npts - x->x_hop) * sizeof(*x->x_inbuf));
            x->x_countdown = 0;
        }
        if (x->x_loud)
            x->x_loud--;
    }
}

// also needs to run every time the ugen is called
// seems like this just fills up the internal buffer (inbuf)
// with chunks of samples. The other method above (tick)
// is what processes the inbuf once it is full (reaches size
// determined by x_npts - the analysis window size).
// Seems perhaps that countdown is unnecessary if
// x_infill is handling countdown duties?
static int sigmund_perform(t_sigmund *x, const t_sample *in, int n)
{
    if (x->x_hop % n)
        return -1;
    if (x->x_countdown > 0)
        x->x_countdown -= n;
    else if (x->x_infill != x->x_npts)
    {
        int j;
        t_float *fp = x->x_inbuf + x->x_infill;
        // Copy the samples into x_inbuf,
        // starting from where we left off the previous
        // time this function was called (x_infill)
        for (j = 0; j < n; j++)
            *fp++ = *in++;
        // Update x_infill to reflect the advanced position
        x->x_infill += n;
    }
    return 0;
}
