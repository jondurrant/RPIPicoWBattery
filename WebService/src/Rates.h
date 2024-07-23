/*
 * Rates.h
 *
 *
 *  Created on: 20 Mar 2024
 *      Author: jondurrant
 */

#ifndef SEC_10_EXAMPLES_YOUTUBE_SRC_YOUTUBE_H_
#define SEC_10_EXAMPLES_YOUTUBE_SRC_YOUTUBE_H_

#include "../../Rates/src/PicoGraphics_ExtRGB888.h"
#include "pico/stdlib.h"
#include "galactic_unicorn.hpp"

#define BUF_LEN 4096
#define MAX_TAGS 200

class Rates {
public:
	Rates();
	virtual ~Rates();

	void update();


	void display();

    /***
     * Initialise the display
     */
	void init();

protected:

	void webservice();

private:

	char xUserBuf[BUF_LEN];

	pimoroni::GalacticUnicorn xGalacticUnicorn;
	PicoGraphics_ExtRGB888 *pGraphics;
	void * pScreenBuf;

	PicoGraphics_ExtRGB888 *pClip;
	void * pClipBuf;


	float xEuros = 0.0;
	float xPounds = 0.0;

};

#endif /* SEC_10_EXAMPLES_YOUTUBE_SRC_YOUTUBE_H_ */
