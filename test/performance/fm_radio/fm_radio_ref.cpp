/*
 * fmref.c: C reference implementation of FM Radio
 * David Maze <dmaze@cag.lcs.mit.edu>
 * $Id: fmref.c,v 1.15 2003-11-05 18:13:10 dmaze Exp $
 */

#include "../drivers/test_driver.hpp"
#include <valgrind/callgrind.h>

#ifdef raw
#include <raw.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif
#include <math.h>

#define SAMPLING_RATE 250000000
#define CUTOFF_FREQUENCY 108000000
#define NUM_TAPS 64
#define MAX_AMPLITUDE 27000.0
#define BANDWIDTH 10000
#define DECIMATION 4
/* Must be at least NUM_TAPS+1: */
#define IN_BUFFER_LEN 10000

#define PRINT 0
#define DEBUG 0

void begin(void);

typedef struct FloatBuffer
{
  float buff[IN_BUFFER_LEN];
  int rpos = 0, rlen = 0;
  void reset() { rpos = 0; rlen = 0; }
} FloatBuffer;

void fb_compact(FloatBuffer *fb);
int fb_ensure_writable(FloatBuffer *fb, int amount);

/* Reading data: */
void get_floats(FloatBuffer *fb);

/* Low pass filter: */
typedef struct LPFData
{
  float coeff[NUM_TAPS];
  float freq;
  int taps, decimation;
} LPFData;
float lpf_coeff[NUM_TAPS];
void init_lpf_data(LPFData *data, float freq, int taps, int decimation);
void run_lpf(FloatBuffer *fbin, FloatBuffer *fbout, LPFData *data);

void run_demod(FloatBuffer *fbin, FloatBuffer *fbout);

#define EQUALIZER_BANDS 10
float eq_cutoffs[EQUALIZER_BANDS + 1] =
#if 0
  { 55.000004, 77.78174, 110.00001, 155.56354, 220.00002, 311.12695,
    440.00003, 622.25415, 880.00006, 1244.5078, 1760.0001 };
#endif
{
55.000000000000000,
80.835395812988281,
118.806571960449219,
174.614120483398438,
256.636383056640625,
377.187347412109375,
554.365234375000000,
814.769775390625000,
1197.495239257812500,
1760.000000000000000,
2586.732666015625000
};

typedef struct EqualizerData
{
  LPFData lpf[EQUALIZER_BANDS + 1];
  FloatBuffer fb[EQUALIZER_BANDS + 1];
  float gain[EQUALIZER_BANDS];
} EqualizerData;
void init_equalizer(EqualizerData *data);
void run_equalizer(FloatBuffer *fbin, FloatBuffer *fbout, EqualizerData *data);

void write_floats(FloatBuffer *fb);

struct fm_radio_test
{
    FloatBuffer fb1, fb2, fb3, fb4;
    LPFData lpf_data;
    EqualizerData eq_data;

    void initialize()
    {
        fb1.reset();
        fb2.reset();
        fb3.reset();
        fb4.reset();
        init_lpf_data(&lpf_data, CUTOFF_FREQUENCY, NUM_TAPS, DECIMATION);
        init_equalizer(&eq_data);

        /* Startup: */
        get_floats(&fb1);
        /* LPF needs at least NUM_TAPS+1 inputs; get_floats is fine. */
        run_lpf(&fb1, &fb2, &lpf_data);
        /* run_demod needs 1 input, OK here. */
        /* run_equalizer needs 51 inputs (same reason as for LPF).  This means
         * running the pipeline up to demod 50 times in advance: */

        for (int i = 0; i < 64; i++)
        {
          if (fb1.rlen - fb1.rpos < NUM_TAPS + 1)
            get_floats(&fb1);
          run_lpf(&fb1, &fb2, &lpf_data);
          run_demod(&fb2, &fb3);
        }
    }

    void run()
    {
        CALLGRIND_TOGGLE_COLLECT;
        for(int i = 0; i < 500; ++i)
        {
          /* The low-pass filter will need NUM_TAPS+1 items; read them if we
           * need to. */
          if (fb1.rlen - fb1.rpos < NUM_TAPS + 1)
            get_floats(&fb1);
          run_lpf(&fb1, &fb2, &lpf_data);
          run_demod(&fb2, &fb3);
          run_equalizer(&fb3, &fb4, &eq_data);
          write_floats(&fb4);
        }
        CALLGRIND_TOGGLE_COLLECT;
    }
};

int main()
{
    fm_radio_test test;

    for (int i = 0; i < 1000; ++i)
    {
        test.initialize();
        test.run();
    }


    //test_driver<fm_radio_test> driver;
    //driver.go(test, 3, 1000);
}

void fb_compact(FloatBuffer *fb)
{
  memmove(fb->buff, fb->buff+fb->rpos, fb->rlen - fb->rpos);
  fb->rlen -= fb->rpos;
  fb->rpos = 0;
}

int fb_ensure_writable(FloatBuffer *fb, int amount)
{
  int available = IN_BUFFER_LEN - fb->rlen;
  if (available >= amount)
    return 1;
  
  /* Nope, not enough room, move current contents back to the beginning. */
  fb_compact(fb);
  
  available = IN_BUFFER_LEN - fb->rlen;
  if (available >= amount)
    return 1;

  /* Hmm.  We're probably hosed in this case. */
#ifndef raw
  printf("fb_ensure_writable(%p): couldn't ensure %d bytes (only %d available)\n", fb, amount, available);
#endif
  return 0;
}

void get_floats(FloatBuffer *fb)
{
  static int x = 0;
  fb_compact(fb);
  
  /* Fill the remaining space in fb with 1.0. */
  while (fb->rlen < IN_BUFFER_LEN) {
    fb->buff[fb->rlen++] = (float)x;
    //fb->buff[fb->rlen++] = 1.0f;
    x++;
  }
}

void init_lpf_data(LPFData *data, float freq, int taps, int decimation)
{
  /* Assume that CUTOFF_FREQUENCY is non-zero.  See comments in
   * StreamIt LowPassFilter.java for origin. */
  float w = 2 * M_PI * freq / SAMPLING_RATE;
  int i;
  float m = taps - 1.0;

  data->freq = freq;
  data->taps = taps;
  data->decimation = decimation;
#if DEBUG
  printf("m = %.15f\n", m);
  printf("w = %.30f\n", w);
#endif
  for (i = 0; i < taps; i++)
  {
    if (i - m/2 == 0.0)
      data->coeff[i] = w / M_PI;
    else
      data->coeff[i] = sinf(w * (i - m/2)) / M_PI / (i - m/2) *
        (0.54 - 0.46 * cos(2 * M_PI * i / m));

    //printf("sin %d = %.10f\n", i,  sin(w));
    //printf("coef %d = %.10f\n", i, data->coeff[i]);
  }
}

void run_lpf(FloatBuffer *fbin, FloatBuffer *fbout, LPFData *data)
{
  float sum = 0.0;
  int i = 0;

#ifndef raw
  if (fbin->rpos + data->taps - 1 >= fbin->rlen)
    printf("WARNING: upcoming underflow in run_lpf()\n");
#endif

  for (i = 0; i < data->taps; i++)
    sum += fbin->buff[fbin->rpos + i] * data->coeff[i];

  fbin->rpos += data->decimation + 1;
  
  /* Check that there's room in the output buffer; move data if necessary. */
  fb_ensure_writable(fbout, 1);
  fbout->buff[fbout->rlen++] = sum;
}

void run_demod(FloatBuffer *fbin, FloatBuffer *fbout)
{
  float temp, gain;
  gain = MAX_AMPLITUDE * SAMPLING_RATE / (BANDWIDTH * M_PI);
  temp = fbin->buff[fbin->rpos] * fbin->buff[fbin->rpos + 1];
  temp = gain * atanf(temp);
  /*
  printf("%.15f * %.15f * %.15f = %.15f\n",
         fbin->buff[fbin->rpos],
          fbin->buff[fbin->rpos + 1],
          gain,
          temp);
          */
  fbin->rpos++;
  fb_ensure_writable(fbout, 1);
  fbout->buff[fbout->rlen++] = temp;
}

void init_equalizer(EqualizerData *data)
{
  int i;
  
  /* Equalizer structure: there are ten band-pass filters, with
   * cutoffs as shown below.  The outputs of these filters get added
   * together.  Each band-pass filter is LPF(high)-LPF(low). */
  for (i = 0; i < EQUALIZER_BANDS + 1; i++)
    init_lpf_data(&data->lpf[i], eq_cutoffs[i], 64, 0);

  /* Also initialize member buffers. */
  for (i = 0; i < EQUALIZER_BANDS + 1; i++)
    data->fb[i].rpos = data->fb[i].rlen = 0;

  for (i = 0; i < EQUALIZER_BANDS; i++) {
    // the gain amplifies the middle bands the most
    float val = (((float)i)-(((float)(EQUALIZER_BANDS-1))/2.0f)) / 5.0f;
    data->gain[i] = val > 0 ? 2.0-val : 2.0+val;
#if DEBUG
    printf("EQ amp %d = %.15f\n", i, data->gain[i]);
#endif
  }
}

void run_equalizer(FloatBuffer *fbin, FloatBuffer *fbout, EqualizerData *data)
{
  int i, rpos;
  float lpf_out[EQUALIZER_BANDS + 1];
  float sum = 0.0;

  /* Save the input read location; we can reuse the same input data on all
   * of the LPFs. */
  rpos = fbin->rpos;
  
  /* Run the child filters. */
  for (i = 0; i < EQUALIZER_BANDS + 1; i++)
  {
    fbin->rpos = rpos;
    run_lpf(fbin, &data->fb[i], &data->lpf[i]);
    lpf_out[i] = data->fb[i].buff[data->fb[i].rpos++];
  }

  /* Now process the results of the filters.  Remember that each band is
   * output(hi)-output(lo). */
  for (i = 0; i < EQUALIZER_BANDS; i++)
    sum += (lpf_out[i+1] - lpf_out[i]) * data->gain[i];

  /* Write that result.  */
  fb_ensure_writable(fbout, 1);
  fbout->buff[fbout->rlen++] = sum;
}

static volatile float v;

void write_floats(FloatBuffer *fb)
{
  /* printf() any data that's available: */
#if PRINT
  while (fb->rpos < fb->rlen)
    printf("%f\n", fb->buff[fb->rpos++]);
#else
  while (fb->rpos < fb->rlen)
      v = fb->buff[fb->rpos++];
#endif
}
