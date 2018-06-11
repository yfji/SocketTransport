#ifndef CONFIG_H
#define CONFIG_H

#endif // CONFIG_H

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_
#endif

//#define IN
#define COMPRESS
#define SIMPLE
#define FINITE_FRAMES

#ifdef IN
#define INETADDR	"10.106.18.187"
#else
#define OUTADDR    "10.106.20.73"
#define INETADDR	"10.106.20.73"
#endif

#define IMAGE_PORT  9010
#define DATA_PORT   9012

#define RESIZE	1
#define MAX_LEN	32768
