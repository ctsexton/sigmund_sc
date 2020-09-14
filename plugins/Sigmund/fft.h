/* Copyright (c) 1997- Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* --------- Pd interface to FFTW library; imitate Mayer API ---------- */

/* changes and additions for FFTW3 by Thomas Grill                      */

#include <fftw3.h>

int ilog2(int n)
{
    int ret = -1;
    while (n)
    {
        n >>= 1;
        ret++;
    }
    return (ret);
}

#define MINFFT 0
#define MAXFFT 30

/* from the FFTW website:
 #include <fftw3.h>
     ...
     {
         fftw_complex *in, *out;
     fftw_plan p;
     ...
         in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
         out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
         p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
     ...
         fftw_execute(p);
     ...
     fftw_destroy_plan(p);
         fftw_free(in); fftw_free(out);
     }

FFTW_FORWARD or FFTW_BACKWARD, and indicates the direction of the transform you
are interested in. Alternatively, you can use the sign of the exponent in the
transform, -1 or +1, which corresponds to FFTW_FORWARD or FFTW_BACKWARD
respectively. The flags argument is either FFTW_MEASURE

*/

/* complex stuff */

typedef struct {
    fftwf_plan plan;
    fftwf_complex *in,*out;
} cfftw_info;

static cfftw_info cfftw_fwd[MAXFFT+1 - MINFFT],cfftw_bwd[MAXFFT+1 - MINFFT];

static void cfftw_term(void)
{
    int i, j;
    cfftw_info *cinfo[2];

    for (i = 0; i < MAXFFT+1 - MINFFT; i++)
    {
      cinfo[0] = &cfftw_fwd[i];
      cinfo[1] = &cfftw_bwd[i];

      for (j = 0; j < 2; j++)
      {
        if (cinfo[j]->plan)
        {
          fftwf_destroy_plan(cinfo[j]->plan);
          fftwf_free(cinfo[j]->in);
          fftwf_free(cinfo[j]->out);
          cinfo[j]->plan = 0;
          cinfo[j]->in = 0;
          cinfo[j]->out = 0;
        }
      }
    }
}


/* real stuff */

typedef struct {
    fftwf_plan plan;
    float *in,*out;
} rfftw_info;

static rfftw_info rfftw_fwd[MAXFFT+1 - MINFFT],rfftw_bwd[MAXFFT+1 - MINFFT];

static rfftw_info *rfftw_getplan(int n,int fwd)
{
    rfftw_info *info;
    int logn = ilog2(n);
    if (logn < MINFFT || logn > MAXFFT)
        return (0);
    info = (fwd?rfftw_fwd:rfftw_bwd)+(logn-MINFFT);
    if (!info->plan)
    {
        info->in = (float*) fftwf_malloc(sizeof(float) * n);
        info->out = (float*) fftwf_malloc(sizeof(float) * n);
        info->plan = fftwf_plan_r2r_1d(n, info->in, info->out, fwd?FFTW_R2HC:FFTW_HC2R, FFTW_MEASURE);
    }
    return info;
}

static void rfftw_term(void)
{
    int i, j;
    rfftw_info *rinfo[2];

    for (i = 0; i < MAXFFT+1 - MINFFT; i++)
    {
      rinfo[0] = &rfftw_fwd[i];
      rinfo[1] = &rfftw_bwd[i];

      for (j = 0; j < 2; j++)
      {
        if (rinfo[j]->plan)
        {
          fftwf_destroy_plan(rinfo[j]->plan);
          fftwf_free(rinfo[j]->in);
          fftwf_free(rinfo[j]->out);
          rinfo[j]->plan = 0;
          rinfo[j]->in = 0;
          rinfo[j]->out = 0;
        }
      }
    }
}

static int mayer_refcount = 0;

void mayer_init(void)
{
    if (mayer_refcount++ == 0)
    {
        /* nothing to do */
    }
}

void mayer_term(void)
{
    if (--mayer_refcount == 0)
    {
        cfftw_term();
        rfftw_term();
    }
}

/*
    in the following the sign flips are done to
    be compatible with the mayer_fft implementation,
    but it's probably the mayer_fft that should be corrected...
*/

void mayer_realfft(int n, t_sample *fz)
{
    int i;
    rfftw_info *p = rfftw_getplan(n, 1);
    if (!p)
        return;

    for (i = 0; i < n; i++)
        p->in[i] = fz[i];
    fftwf_execute(p->plan);
    for (i = 0; i < n/2+1; i++)
        fz[i] = p->out[i];
    for (; i < n; i++)
        fz[i] = -p->out[i];
}

void mayer_realifft(int n, t_sample *fz)
{
    int i;
    rfftw_info *p = rfftw_getplan(n, 0);
    if (!p)
        return;

    for (i = 0; i < n/2+1; i++)
        p->in[i] = fz[i];
    for (; i < n; i++)
        p->in[i] = -fz[i];
    fftwf_execute(p->plan);
    for (i = 0; i < n; i++)
        fz[i] = p->out[i];
}
