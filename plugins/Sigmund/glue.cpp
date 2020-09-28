#include "sigmund.c"

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
    t_notefinder x_notefinder;  /* note parsing state */
    t_peak *x_trackv;           /* peak tracking state */
    int x_ntrack;               /* number of peaks tracked */
    unsigned int x_dopitch:1;   /* which things to calculate */
    unsigned int x_donote:1;
    unsigned int x_dotracks:1;
} t_sigmund;

static void sigmund_preinit(t_sigmund *x)
{
    x->x_npts = NPOINTS_DEF;
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
    t_peak *peakv = (t_peak *)alloca(sizeof(t_peak) * x->x_npeak);
    int nfound, i, cnt;
    t_float freq = 0, power, note = 0;

    // IMPORT STUFF:
    sigmund_getrawpeaks(npts, arraypoints, x->x_npeak, peakv, &nfound, &power, srate, loud, x->x_maxfreq);
    sigmund_getpitch(nfound, peakv, &freq, npts, srate, x->x_param1, x->x_param2, loud);
    sigmund_peaktrack(nfound, peakv, x->x_ntrack, x->x_trackv, 2* srate / npts, loud);

    for (i = 0; i < x->x_ntrack; i++)
    {
        const int index = i * 3;
        bufData[index] = x->x_trackv[i].p_freq;
        bufData[index + 1] = 2*x->x_trackv[i].p_amp;
        bufData[index + 2] = x->x_trackv[i].p_tmp;
    }
}

static void sigmund_free(t_sigmund *x)
{
    if (x->x_inbuf)
    {
        freebytes(x->x_inbuf, x->x_npts * sizeof(*x->x_inbuf));
    }
    if (x->x_trackv)
        freebytes(x->x_trackv, x->x_ntrack * sizeof(*x->x_trackv));
    freebytes(x->x_varoutv, x->x_nvarout * sizeof(t_varout));
}


// Needs to run every time the Ugen is called,
// seems like maybe this is the real "next" function
static void sigmund_tick(t_sigmund *x)
{
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
static t_int *sigmund_perform(t_sigmund *x, t_sample *in, int n)
{
    if (x->x_hop % n)
        return (w+4);
    if (x->x_countdown > 0)
        x->x_countdown -= n;
    else if (x->x_infill != x->x_npts)
    {
        int j;
        t_float *fp = x->x_inbuf + x->x_infill;
        for (j = 0; j < n; j++)
            *fp++ = *in++;
        x->x_infill += n;
    }
    return (w+4);
}


static void *sigmund_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sigmund *x;
    sigmund_preinit(x);
    
    
    // set npeaks
    sigmund_npeak(x, atom_getfloatarg(1, argc, argv));
    
    // toggle tracks calculation
    x->x_dotracks = 1;
    x->x_dopitch = 1;
    x->x_ntrack = x->x_npeak;
    x->x_trackv = (t_peak *)getbytes(x->x_ntrack * sizeof(*x->x_trackv));
    
    x->x_infill = 0;
    x->x_countdown = 0;
    x-x_sr = SAMPLE_RATE;
    sigmund_npts(x, x->x_npts);
    notefinder_init(&x->x_notefinder);
    sigmund_clear(x);
    return (x);
}
