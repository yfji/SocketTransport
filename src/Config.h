/*
 * Config.h
 *
 *  Created on: 2017年4月12日
 *      Author: yufeng
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_
#define PORT	"8010"
//#define IN
#define COMPRESS
#define SIMPLE
#define FINITE_FRAMES
#ifdef IN
#define INETADDR	"10.106.18.187"
#else
#define OUTADDR    "192.168.3.24"
#define INETADDR	"192.168.3.24"
#endif
#define MAX_LEN	32768
class Config {
public:
};

#endif /* SRC_CONFIG_H_ */
