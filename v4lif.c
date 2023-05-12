/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: v4lif.c,v 1.4 2006/04/29 09:13:22 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <bitmap.h>
#include <png.h>

#include <sys/mman.h>
#include <linux/types.h>
#include <sys/ioctl.h>  
#include <linux/videodev.h> 

#include <fcntl.h>

static char* rcsid="$Id: v4lif.c,v 1.4 2006/04/29 09:13:22 tommy Exp $";

#define DEF_WIDTH	348
#define DEF_HEIGHT	288

#define FMT_UNKNOWN	0
#define FMT_PPM		1
#define FMT_PNG		2
#define FMT_JPEG	3

#define IN_TV		0
#define IN_COMPOSITE	1
#define IN_COMPOSITE2	2
#define IN_SVIDEO	3
#define IN_DEFAULT	8

#define NORM_PAL	0
#define NORM_NTSC	1
#define NORM_SECAM	2
#define NORM_DEFAULT	0

#define QUAL_DEFAULT	80

static char * v4lgrabimage (int dev, int width, int height, int input,int norm,int fmt,int *size) {
  struct video_capability vid_caps;
  struct video_mbuf vid_buf;
  struct video_mmap vid_mmap;
  struct video_channel vid_chnl;
  char *map;
  int len;

  static char smap[DEF_WIDTH*DEF_HEIGHT*3];

  if (ioctl (dev, VIDIOCGCAP, &vid_caps) == -1) {
    perror ("ioctl (VIDIOCGCAP)");
    return (NULL);
  }

  if (input != IN_DEFAULT) {
    vid_chnl.channel = -1;
    if (ioctl (dev, VIDIOCGCHAN, &vid_chnl) == -1) {
      perror ("ioctl (VIDIOCGCHAN)");
    } else {
      vid_chnl.channel = input;
      vid_chnl.norm    = norm;
      if (ioctl (dev, VIDIOCSCHAN, &vid_chnl) == -1) {
	perror ("ioctl (VIDIOCSCHAN)");
	return (NULL);
      }
    }
  }

  if (ioctl (dev, VIDIOCGMBUF, &vid_buf) == -1) {
    struct video_window vid_win;
    if (ioctl (dev, VIDIOCGWIN, &vid_win) != -1) {
      vid_win.width  = width;
      vid_win.height = height;
      if (ioctl (dev, VIDIOCSWIN, &vid_win) == -1)
        return (NULL);
    }

    fprintf(stderr,"v4l: performing read()\n");
 
    map = malloc (width * height * 3);
    len = read (dev, map, width * height * 3);
    if (len <=  0) {
      free (map);
      return (NULL);
    }
    *size = 0;
    return (map);
  }

  fprintf(stderr,"v4l: performing mmap()\n");

  map = mmap (0, vid_buf.size, PROT_READ|PROT_WRITE,MAP_SHARED,dev,0);
  if ((unsigned char *)-1 == (unsigned char *)map) {
    perror ("mmap()");
    return (NULL);
  }

  vid_mmap.format = fmt;
  vid_mmap.frame = 0;
  vid_mmap.width = width;
  vid_mmap.height =height;

  if (ioctl (dev, VIDIOCMCAPTURE, &vid_mmap) == -1) {
    perror ("VIDIOCMCAPTURE");
    munmap (map, vid_buf.size);
    return (NULL);
  }

  if (ioctl (dev, VIDIOCSYNC, &vid_mmap) == -1) {
    perror ("VIDIOCSYNC");
    munmap (map, vid_buf.size);
    return (NULL);
  }

  *size = vid_buf.size;
  memcpy(smap,map,DEF_WIDTH*DEF_HEIGHT*3);
  munmap (map, vid_buf.size);
  return (smap);
}

static int grabberfd=0;

char *v4lgrab() { 
  int size;
  char *rc;

  if(grabberfd == 0) {
    if((grabberfd=open("/dev/video0", O_RDWR)) < 0) {
      fprintf(stderr,"cannot open /dev/video\n");
      return(NULL);
    }
  }
  rc=v4lgrabimage (grabberfd, DEF_WIDTH, DEF_HEIGHT, IN_COMPOSITE, NORM_PAL,VIDEO_PALETTE_RGB24,&size); 
  //rc=v4lgrabimage (grabberfd, DEF_WIDTH, DEF_HEIGHT, IN_DEFAULT, NORM_PAL,15,&size); 
  return(rc);
}

