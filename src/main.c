#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <vslib.h>

#include "ver.h"

struct pcm_wave_format {
  uint16_t format_tag;
  uint16_t channels;
  uint32_t samples_per_sec;
  uint32_t avg_bytes_per_sec;
  uint16_t block_align;
  uint16_t bits_per_sample;
};

struct file_format {
  int freq;
  int depth;
  int channels;
};

bool read_wave_format(FILE *fp, struct file_format *ff) {
  char sig[4] = { 0 };
  if (!fread(sig, 4, 1, fp)) {
    printf("failed to read signature.\n");
    return false;
  }
  if (sig[0] != 'R' || sig[1] != 'I' || sig[2] != 'F' || sig[3] != 'F') {
    printf("invalid signature.\n");
    return false;
  }
  uint32_t size = 0;
  if (!fread(&size, sizeof(uint32_t), 1, fp)) {
    printf("failed to read total file size.\n");
    return false;
  }
  if (!fread(sig, 4, 1, fp)) {
    printf("failed to read signature.\n");
    return false;
  }
  if (sig[0] != 'W' || sig[1] != 'A' || sig[2] != 'V' || sig[3] != 'E') {
    printf("invalid signature.\n");
    return false;
  }
  for(;;) {
    if (!fread(sig, 4, 1, fp)) {
      printf("failed to read signature2.\n");
      return false;
    }
    if (!fread(&size, sizeof(uint32_t), 1, fp)) {
      printf("failed to read total file size.\n");
      return false;
    }
    if (sig[0] != 'f' || sig[1] != 'm' || sig[2] != 't' || sig[3] != ' ') {
      if (fseek(fp, size, SEEK_CUR)) {
        printf("failed to seek.\n");
        return false;
      }
      continue;
    }
    struct pcm_wave_format pcmwf;
    if (!fread(&pcmwf, sizeof(struct pcm_wave_format), 1, fp)) {
      printf("failed to read PCM Wave Format structure.\n");
      return false;
    }
    ff->freq = pcmwf.samples_per_sec;
    ff->channels = pcmwf.channels;
    ff->depth = pcmwf.bits_per_sample;
    return true;
  }
}

bool get_wave_format(char *path, struct file_format *ff) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    return false;
  }
  const bool r = read_wave_format(fp, ff);
  fclose(fp);
  return r;
}

int main(int argc, char *argv[])
{
  if (argc != 5) {
    printf("vstouch %s\n", VERSION);
    printf("\n");
    printf("USAGE:\n");
    printf("  vstouch speed pitch infile outfile\n");
    return 1;
  }
  char *p;
  float speed = strtof(argv[1], &p);
  if (p == argv[1]) {
    printf("speed is invalid.\n");
    return 1;
  }
  float pitch = strtof(argv[2], &p);
  if (p == argv[2]) {
    printf("pitch is invalid.\n");
    return 1;
  }
  char *infile = argv[3];
  char *outfile = argv[4];

  struct file_format ff = { 0 };
  if (!get_wave_format(infile, &ff)) {
    printf("failed to read \"%s\".\n", infile);
    return 1;
  }

  HVSPRJ h;
  int r = VslibCreateProject(&h);
  if (r != VSERR_NOERR) {
    printf("failed to create project.\n");
    return 1;
  }

  VSPRJINFO proj;
  r = VslibGetProjectInfo(h, &proj);
  if (r != VSERR_NOERR) {
    printf("failed to get project information.\n");
    goto failed;
  }
  proj.sampFreq = ff.freq;
  r = VslibSetProjectInfo(h, &proj);
  if (r != VSERR_NOERR) {
    printf("failed to set project information.\n");
    goto failed;
  }

  int idx;
  r = VslibAddItemEx(h, infile, &idx, 24, 84, ANALYZE_OPTION_VOCAL_SHIFTER);
  if (r != VSERR_NOERR) {
    switch(r) {
      case VSERR_PRM:
        printf("internal error VSERR_PRM.\n");
        break;
      case VSERR_WAVEOPEN:
        printf("failed to open \"%s\".\n", infile);
        break;
      case VSERR_WAVEFORMAT:
        printf("unsupported wave format.\n");
        break;
      case VSERR_FREQ:
        printf("unsupported sampling frequency.\n");
        break;
      case VSERR_MAX:
        printf("internal error VSERR_MAX.\n");
        break;
    }
    goto failed;
  }

  VSITEMINFO item;
  r = VslibGetItemInfo(h, idx, &item);
  if (r != VSERR_NOERR) {
    printf("failed to get an inserted item.\n");
    goto failed;
  }
  item.synthMode = SYNTHMODE_MF;
  r = VslibSetItemInfo(h, idx, &item);
  if (r != VSERR_NOERR) {
    printf("failed to update item info.\n");
    goto failed;
  }

  int numTimeCtrl = 0;
  r = VslibGetTimeCtrlPntNum(h, idx, &numTimeCtrl);
  if (r != VSERR_NOERR) {
    printf("failed to get number of time control points.\n");
    goto failed;
  }
  for (int i = 0; i < numTimeCtrl; ++i) {
    int t1, t2;
    r = VslibGetTimeCtrlPnt(h, idx, i, &t1, &t2);
    if (r != VSERR_NOERR) {
      printf("failed to get time control point.\n");
      goto failed;
    }
    t2 /= speed;
    r = VslibSetTimeCtrlPnt(h, idx, i, t1, t2);
    if (r != VSERR_NOERR) {
      printf("failed to set time control point.\n");
      goto failed;
    }
  }

  for(int i = 0; i < item.ctrlPntNum; ++i){
    VSCPINFOEX cinfo;
    VslibGetCtrlPntInfoEx(h, idx, i, &cinfo);
    cinfo.pitEdit = VslibFreq2Cent(VslibCent2Freq(cinfo.pitEdit) * pitch);
    VslibSetCtrlPntInfoEx(h, idx, i, &cinfo);
  }

  r = VslibExportWaveFile(h, outfile, ff.depth, ff.channels);
  if (r != VSERR_NOERR) {
    switch(r) {
      case VSERR_PRM:
        printf("internal error VSERR_PRM.\n");
        break;
      case VSERR_WAVEOPEN:
        printf("failed to open \"%s\".\n", outfile);
        break;
    }
    goto failed;
  }
  VslibDeleteProject(h);
  return 0;

failed:
  VslibDeleteProject(h);
  return 1;
}
