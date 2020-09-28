
typedef struct _sigmund
{
    t_varout *x_varoutv;
    int x_nvarout;
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
    x->x_nvarout = 0;
    x->x_varoutv = (t_varout *)getbytes(0);
    x->x_trackv = 0;
    x->x_ntrack = 0;
    x->x_dopitch = x->x_donote = x->x_dotracks = 0;
    x->x_inbuf = 0;
}
