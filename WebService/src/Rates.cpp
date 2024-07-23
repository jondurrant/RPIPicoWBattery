/*
 * Rates.cpp
 *
 *  Created on: 20 Mar 2024
 *      Author: jondurrant
 */

#include "../../Rates/src/Rates.h"

#include "Request.h"
#include <cstdio>

#include "../../Rates/src/Digits6x11.h"
#include "tiny-json.h"

#include "FreeRTOS.h"

Rates::Rates() {
	pScreenBuf = pvPortMalloc (
				PicoGraphics_ExtRGB888::buffer_size(
						xGalacticUnicorn.WIDTH,
						xGalacticUnicorn.HEIGHT));

		if (pScreenBuf != NULL){
			pGraphics = new PicoGraphics_ExtRGB888(
					xGalacticUnicorn.WIDTH,
					xGalacticUnicorn.HEIGHT,
					pScreenBuf);
		} else {
			pGraphics = NULL;
			printf("ERROR: FAILED TO ALLOCATE\n");
		}

		pClipBuf = pvPortMalloc (
				PicoGraphics_ExtRGB888::buffer_size(
						70,
						xGalacticUnicorn.HEIGHT));
		if (pClipBuf != NULL){
			pClip = new PicoGraphics_ExtRGB888(
							70,
							xGalacticUnicorn.HEIGHT,
							pClipBuf);
		}else {
			pClip = NULL;
			printf("ERROR: FAILED TO ALLOCATE\n");
		}
}

Rates::~Rates() {
	delete(pGraphics);
	vPortFree(pScreenBuf);
}


void Rates::init(){
	xGalacticUnicorn.init();
	pGraphics->set_pen(0, 0,0);
	pGraphics->clear();
	pGraphics->set_pen(0x30, 0x30,0xC0);
	pGraphics->set_font("bitmap8");
	pGraphics->text("Rates", {0,0},  -1, 1.0);
	xGalacticUnicorn.update(pGraphics);
}



void Rates::update(){
	Digits6x11 digits;

	webservice();

	pClip->set_pen(0, 0, 0);
	pClip->clear();

	pClip->set_pen(0xAF, 0xFF, 0xAF);

	int col = 0;
	col = digits.drawPounds(xPounds, col, 0, pClip);
	col +=4;

	pClip->set_pen(0xAF, 0xAF, 0xFF);
	col = digits.drawEuros(xEuros, col, 0, pClip);

}


void Rates::display(){



	for (int i = 0; i < xGalacticUnicorn.WIDTH; i++){
		pGraphics->wipe_right(i);
		xGalacticUnicorn.update(pGraphics);
		vTaskDelay(50);
	}


	Point from = {0,0};
	Point to = {0,0};
	for (int col = xGalacticUnicorn.WIDTH -1; col >=0; col--){
		to = {col, 0};
		pGraphics->paste(
				pClip,
				&pGraphics->bounds,
				&from,
				&to);;

		xGalacticUnicorn.update(pGraphics);
		vTaskDelay(50);
	}

	to = {0,0};
	for (int col = 1; col <  70 ; col ++){
		from = {col, 0};
		pGraphics->paste(
				pClip,
				&pGraphics->bounds,
				&from,
				&to);

		xGalacticUnicorn.update(pGraphics);
		vTaskDelay(50);
	}

}


void Rates::webservice(){
	Request req((char *)xUserBuf, BUF_LEN);
	char json[BUF_LEN];
	 json_t pool[MAX_TAGS];
	bool res;
	const char URL[] = "https://v6.exchangerate-api.com/v6/%s/latest/USD";
	char url[80];

	sprintf(url, URL, EXCHANGERATEKEY);

	printf("URL: %s\n", url);
	res = req.get(url);
	if ( res ){
		res = (req.getStatusCode() == 200);
	}
	if (res){
		printf("WS: %.*s\n", req.getPayloadLen(), req.getPayload());
		memcpy(json, req.getPayload(), req.getPayloadLen());
		json[req.getPayloadLen()] = 0;
		json_t const* parent = json_create(
				json,
				pool,
				MAX_TAGS);
		if ( parent != NULL ){
			json_t const* rates = json_getProperty( parent, "conversion_rates" );
			if ( rates != NULL ){


				//GBP
				json_t const* gbp = json_getProperty( rates, "GBP" );
				if ( gbp != NULL ){
					if ( json_getType( gbp ) == JSON_REAL ){
						xPounds = json_getReal(gbp);
					}
				} else {
					printf("No GBP\n");
				}

				//EURO
				json_t const* euro = json_getProperty( rates, "EUR" );
				if ( gbp != NULL ){
					if ( json_getType( euro ) == JSON_REAL ){
						xEuros = json_getReal(euro);
					}
				} else {
					printf("No EURO\n");
				}

			} else {
				printf("rates not found\n");
			}

		} else {
			printf("JSON Pass failed\n");
		}

	} else {
		printf("WS failed %d\n", req.getStatusCode());
	}
}

