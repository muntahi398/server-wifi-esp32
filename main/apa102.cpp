/* Copyright (c) 2016 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */

#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"

#include "apa102.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define HIGH 1
#define LOW 0
int global_intensity = 1;
static const char *TAG = "APA102";
//extern int global_intensity;


apa102::apa102(uint32_t ledcount) {
	setLEDCount(ledcount);

	uint64_t bitmask = 0;
	bitmask = bitmask | (1<<clockPin);
	bitmask = bitmask | (1<<dataPin);

	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = bitmask;
	gpioConfig.mode         = GPIO_MODE_OUTPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
	gpioConfig.intr_type    = GPIO_INTR_DISABLE;
	gpio_config(&gpioConfig);

	gpio_set_level(dataPin, LOW);
	gpio_set_level(clockPin, LOW);
}

uint8_t apa102::random(uint8_t min,uint8_t max) {
	if (min>max) {
		uint8_t swap;
		swap = min;
		min = max;
		max = swap;
	}
	return (uint8_t)(min + esp_random() % (max + 1 - min));
}

void apa102::setLEDCount(uint32_t ledcount) {
	this->ledcount = ledcount;
}

uint32_t apa102::getLEDCount() const {
	return ledcount;
}


void apa102::writeByte(uint8_t b) {
	uint8_t pos;
	for (pos=0;pos<=7;pos++) {
		gpio_set_level(dataPin, b >> (7-pos) & 1);
		gpio_set_level(clockPin, HIGH);
		gpio_set_level(clockPin, LOW);
	}
}

void apa102::startFrame() {
	ESP_LOGD(TAG, "startFrame");
	writeByte(0);
	writeByte(0);
	writeByte(0);
	writeByte(0);
}

void apa102::endFrame() {
	ESP_LOGD(TAG, "endFrame");
	writeByte(0xFF);
	writeByte(0xFF);
	writeByte(0xFF);
	writeByte(0xFF);
}

void apa102::writeRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
	writeByte(0b11100000 | brightness);
	writeByte(blue);
	writeByte(green);
	writeByte(red);
}

void apa102::writeColor(colorRGBB color) {
	writeRGB(color.red, color.green, color.blue, color.brightness);
}

void apa102::writeColors(colorRGBB * colors, uint16_t count) {
	ESP_LOGD(TAG, "writeColors");
	startFrame();
	for(uint16_t i = 0; i < count; i++) {
		writeColor(colors[i]);
	}
	endFrame();
}

void apa102::setColor(colorRGBB color) {
	colorRGBB RGB[ledcount];

	for(uint16_t i = 0; i < ledcount; i++) {

		RGB[i].red=color.red;
		RGB[i].green=color.green;
		RGB[i].blue=color.blue;
		RGB[i].brightness=color.brightness;
	}
	writeColors(RGB, ledcount);
}


void apa102::ramdomBlink(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];
	uint8_t bpos,bdir;

	bpos=0;
	bdir=0;
	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < ledcount; i++) {

			RGB[i].red=random(0,255);
			RGB[i].green=random(0,255);
			RGB[i].blue=random(0,255);
			RGB[i].brightness=bpos;
		}
		if (bdir==0) {
			bpos++;
		} else {
			bpos--;
		}
		if (bpos>=31) {
			bpos=30;
			bdir=1;
		}
		if (bpos==0) {
			bpos=1;
			bdir=0;
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}

void apa102::ramdomFade(uint32_t loops, uint16_t delayms) {
	uint8_t bpos,bdir;
	colorRGBB RGB[ledcount];

	bpos=0;
	bdir=0;
	for(uint32_t pos = 0; pos < loops; pos++) {
		RGB[0].red=random(0,255);
		RGB[0].green=random(0,255);
		RGB[0].blue=random(0,255);
		for(uint16_t i = 1; i < ledcount; i++) {

			RGB[i].red=RGB[0].red;
			RGB[i].green=RGB[0].green;
			RGB[i].blue=RGB[0].blue;
			RGB[i].brightness=bpos;
		}
		if (bdir==0) {
			bpos++;
		} else {
			bpos--;
		}
		if (bpos>=31) {
			bpos=30;
			bdir=1;
		}
		if (bpos==0) {
			bpos=1;
			bdir=0;
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}

void apa102::ramdomWalk(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];

	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < ledcount; i++) {
			if (i==(pos%(ledcount))) {
				RGB[i].red=random(0,255);
				RGB[i].green=random(0,255);
				RGB[i].blue=random(0,255);
				RGB[i].brightness=31;
			} else {
				RGB[i].red=0;
				RGB[i].green=0;
				RGB[i].blue=0;
				RGB[i].brightness=0;
			}
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}

void apa102::ramdomWalk_mnk(uint32_t loops, uint16_t delayms, uint16_t num, uint16_t intensity) {
    colorRGBB RGB[ledcount];
    int led_number;

    for(uint32_t pos = 0; pos < loops; pos++) {
        for(uint16_t i = 0; i < ledcount; i++) {

            for(uint16_t k = 0; k < ledcount; k++) { // setting all to zero
                RGB[k].red=0;
                RGB[k].green=0;
                RGB[k].blue=0;
                RGB[k].brightness=0;
            }
            for (uint16_t j = 0; j < num; j++) {
                if (i==(pos%(ledcount))) {
                    int led_number=0;
                if((pos+j) <8 ){
                    led_number =pos+j;
                } else
                led_number = (pos+j)-ledcount;
                RGB[led_number].red=intensity;
                RGB[led_number].green=intensity;
                RGB[led_number].blue=intensity;
                RGB[led_number].brightness=31;

            }
        }
        }
        writeColors(RGB, ledcount);
        vTaskDelay(delayms/portTICK_PERIOD_MS);

    }
}

void apa102::ramdomStep_mnk(uint32_t loops, uint16_t delayms, uint16_t num) {
    colorRGBB RGB[ledcount];
    int led_number;

    for(uint32_t pos = 0; pos < loops; pos++) {
        for(uint16_t i = 0; i < ledcount; i++) {
            for (uint16_t j = 0; j < num; j++) {
                if (i <= (pos % (ledcount))) {
                    int led_number=0;
                    if((i+j) <8 ){
                        led_number =i+j;
                    } else
                    {led_number = i+j-ledcount;}
                    RGB[led_number].red = random(0, 255);
                    RGB[led_number].green = random(0, 255);
                    RGB[led_number].blue = random(0, 255);
                    RGB[led_number].brightness = 31;
                } else {
                    RGB[i].red = 0;
                    RGB[i].green = 0;
                    RGB[i].blue = 0;
                    RGB[i].brightness = 0;
                }
            }
            writeColors(RGB, ledcount);
            vTaskDelay(delayms / portTICK_PERIOD_MS);
        }

    }
}

void apa102::ramdomStep_mnk_external(uint32_t loops, uint16_t delayms, uint16_t num) {
    colorRGBB RGB[ledcount];
    int led_number;

    for(uint32_t pos = 0; pos < loops; pos++) {
        for(uint16_t i = 0; i < ledcount; i++) {
            for (uint16_t j = 0; j < num; j++) {
                if (i <= (pos % (ledcount))) {
                    int led_number=0;
                    if((i+j) <8 ){
                        led_number =i+j;
                    } else
                    {led_number = i+j-ledcount;}
                    RGB[led_number].red = global_intensity;
                    RGB[led_number].green = global_intensity;
                    RGB[led_number].blue = global_intensity;
                    RGB[led_number].brightness = 31;
                } else {
                    RGB[i].red = 0;
                    RGB[i].green = 0;
                    RGB[i].blue = 0;
                    RGB[i].brightness = 0;
                }
            }
            writeColors(RGB, ledcount);
            vTaskDelay(delayms / portTICK_PERIOD_MS);
        }

    }
}

void apa102::ramdomStep_mnk_external_mod(uint32_t loops, uint16_t delayms, uint16_t num) {
    colorRGBB RGB[ledcount];
    int led_number;

    for(uint32_t pos = 0; pos < loops; pos++) {
        for(uint16_t i = 0; i < ledcount; i++) {
            for(uint16_t k = 0; k < ledcount; k++) { // setting all to zero
                RGB[k].red=0;
                RGB[k].green=0;
                RGB[k].blue=0;
                RGB[k].brightness=0;
            }
            for (uint16_t j = 0; j < num; j++) {
                 {
                    int led_number=0;
                    if((i+j) <8 ){
                        led_number =i+j;
                    } else
                    {led_number = i+j-ledcount;}
                    RGB[led_number].red = global_intensity;
                    RGB[led_number].green = global_intensity;
                    RGB[led_number].blue = global_intensity;
                    RGB[led_number].brightness = 31;
                }
            }
            writeColors(RGB, ledcount);
            vTaskDelay(delayms / portTICK_PERIOD_MS);
        }

    }
}

void apa102::ramdomStep_mnk_left_right_mod(uint32_t loops, uint16_t delayms) {
    colorRGBB RGB[ledcount];
    int led_number=0;
    int pattern[4][3] =
						 {
								 {7,0,1}, // row 0
								 { 3,4,5}, // row 1
								 { 1,2,3}, // row 2
								 {5,6,7}
						 };

        for(uint16_t i = 0; i < 2; i++) {
            for(uint16_t k = 0; k < ledcount; k++) { // setting all to zero
                RGB[k].red=0;
                RGB[k].green=0;
                RGB[k].blue=0;
                RGB[k].brightness=0;
            }
         if (led_number==0)
            {led_number=4;
            }
            else if (led_number==4)
         { led_number=0;
         }



                    RGB[led_number].red = global_intensity;
                    RGB[led_number].green = global_intensity;
                    RGB[led_number].blue = global_intensity;
                    RGB[led_number].brightness = 31;

            writeColors(RGB, ledcount);
            vTaskDelay(delayms / portTICK_PERIOD_MS);
        }


}

void apa102::ramdomStep_mnk_lr_ud_mod(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];
	int led_number=0;
//	int pattern[4][3] =
//			{
//					{7,0,1}, // top
//					{ 3,4,5}, // bottom
//					{ 1,2,3}, // left
//					{5,6,7}  //right
//			};
    int pattern[4][3] =
            {
                    {0,1}, // top
                    {4,5}, // bottom
                    {2,3}, // left
                    {6,7}  //right
            };
	int pat_row = sizeof pattern / sizeof pattern[0];
	int pat_col = sizeof pattern[0]/ sizeof(int);

		for(uint16_t rr=0; rr<pat_row; rr++){

				for (uint16_t k = 0; k < ledcount; k++) { // setting all to zero
					RGB[k].red = 0;
					RGB[k].green = 0;
					RGB[k].blue = 0;
					RGB[k].brightness = 0;
				}


			for(uint16_t cc=0; cc<pat_col; cc++)
			{	RGB[pattern[rr][cc]].red = global_intensity;
				RGB[pattern[rr][cc]].green = global_intensity;
				RGB[pattern[rr][cc]].blue = global_intensity;
				RGB[pattern[rr][cc]].brightness = 31;
			}
			writeColors(RGB, ledcount);
			vTaskDelay(delayms / portTICK_PERIOD_MS);
		}
    for (uint16_t k = 0; k < ledcount; k++) { // setting all to zero
        RGB[k].red = 0;
        RGB[k].green = 0;
        RGB[k].blue = 0;
        RGB[k].brightness = 0;
    }
    writeColors(RGB, ledcount);
    vTaskDelay(delayms / portTICK_PERIOD_MS);

}

void apa102::ramdomBackWalk(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];

	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < ledcount; i++) {
			if (i==((ledcount)-(pos%(ledcount)))) {
				RGB[i].red=random(0,255);
				RGB[i].green=random(0,255);
				RGB[i].blue=random(0,255);
				RGB[i].brightness=31;
			} else {
				RGB[i].red=0;
				RGB[i].green=0;
				RGB[i].blue=0;
				RGB[i].brightness=0;
			}
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}


void apa102::ramdomStep(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];

	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < ledcount; i++) {
			if (i<=(pos%(ledcount))) {
				RGB[i].red=random(0,255);
				RGB[i].green=random(0,255);
				RGB[i].blue=random(0,255);
				RGB[i].brightness=31;
			} else {
				RGB[i].red=0;
				RGB[i].green=0;
				RGB[i].blue=0;
				RGB[i].brightness=0;
			}
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}



void apa102::ramdomStepR(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];

	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < ledcount; i++) {
			if (i<=((ledcount)-(pos%(ledcount)))) {
				RGB[i].red=random(0,255);
				RGB[i].green=random(0,255);
				RGB[i].blue=random(0,255);
				RGB[i].brightness=31;
			} else {
				RGB[i].red=0;
				RGB[i].green=0;
				RGB[i].blue=0;
				RGB[i].brightness=0;
			}
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}

void apa102::ramdomBackStep(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];

	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < ledcount; i++) {
			if (i>=((ledcount)-(pos%(ledcount)))) {
				RGB[i].red=random(0,255);
				RGB[i].green=random(0,255);
				RGB[i].blue=random(0,255);
				RGB[i].brightness=31;
			} else {
				RGB[i].red=0;
				RGB[i].green=0;
				RGB[i].blue=0;
				RGB[i].brightness=0;
			}
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}

void apa102::ramdomBackStepR(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];

	for(uint32_t pos = 0; pos < loops; pos++) {
		for(uint16_t i = 0; i < ledcount; i++) {
			if (i>=(pos%(ledcount))) {
				RGB[i].red=random(0,255);
				RGB[i].green=random(0,255);
				RGB[i].blue=random(0,255);
				RGB[i].brightness=31;
			} else {
				RGB[i].red=0;
				RGB[i].green=0;
				RGB[i].blue=0;
				RGB[i].brightness=0;
			}
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);

	}
}


void apa102::ramdomSingle(uint32_t loops, uint16_t delayms) {
	colorRGBB RGB[ledcount];
	uint16_t randLED;

	for(uint32_t pos = 0; pos < loops; pos++) {
		randLED=random(0,ledcount);
		for(uint16_t i = 0; i < ledcount; i++) {
			if (i==randLED) {
				RGB[i].red=random(0,255);
				RGB[i].green=random(0,255);
				RGB[i].blue=random(0,255);
				RGB[i].brightness=random(1,31);
			} else {
				RGB[i].red=0;
				RGB[i].green=0;
				RGB[i].blue=0;
				RGB[i].brightness=0;
			}
		}
		writeColors(RGB, ledcount);
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}

void apa102::test() {
	ESP_LOGI(TAG, "test sequence %d",looppos);

	ramdomStep(ledcount, 10);
	ramdomStepR(ledcount, 10);
	ramdomBackStep(ledcount, 10);
	ramdomBackStepR(ledcount, 10);

	ramdomWalk(ledcount,5);
	ramdomBackWalk(ledcount,5);
	ramdomSingle(200,5);
	ramdomWalk(ledcount,5);
	ramdomBackWalk(ledcount,5);

	ramdomFade(20,50);
	ramdomBlink(50,100);
	ramdomFade(20,50);

	ramdomWalk(ledcount,5);
	ramdomBackWalk(ledcount,5);
	ramdomWalk(ledcount,10);
	ramdomBackWalk(ledcount,10);
	ramdomWalk(ledcount,20);
	ramdomBackWalk(ledcount,20);
	ramdomWalk(ledcount,30);
	ramdomBlink(20,100);
	ramdomBackWalk(ledcount,30);
	ramdomWalk(ledcount,20);
	ramdomBackWalk(ledcount,20);
	ramdomWalk(ledcount,10);
	ramdomBackWalk(ledcount,10);

	looppos++;
}

void apa102::fadeInOutColor(colorRGBB color, uint16_t delayms) {
	for(uint32_t pos = 0; pos < 31; pos+=3) {
		color.brightness=pos;
		setColor(color);
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
	for(uint32_t pos = 30; pos > 0; pos-=3) {
		color.brightness=pos;
		setColor(color);
		vTaskDelay(delayms/portTICK_PERIOD_MS);
	}
}
void apa102::test2() {
	colorRGBB color;

	ESP_LOGI(TAG, "test sequence %d",looppos);

	color.red=255;
	color.green=0;
	color.blue=0;
	color.brightness=0;
	fadeInOutColor(color,200);

	color.red=0;
	color.green=255;
	color.blue=0;
	color.brightness=0;
	fadeInOutColor(color,200);

	color.red=0;
	color.green=0;
	color.blue=255;
	color.brightness=0;
	fadeInOutColor(color,200);

	looppos++;
}



void apa102::white_w_brightness(uint8_t blevel) {
	static unsigned int report_counter=0;
	report_counter += 1;

	if (report_counter > 5) {
		ESP_LOGI(TAG, "setting all white, with brightness %d", blevel);
		report_counter = 0;
	}

	colorRGBB color;

	color.red=blevel;
	color.green=blevel;
	color.blue=blevel;
	color.brightness=31;

	setColor(color);
	vTaskDelay(250/portTICK_PERIOD_MS);

}
