/*
 Battery.cpp - Battery library
 Copyright (c) 2014 Roberto Lo Giacco.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as 
 published by the Free Software Foundation, either version 3 of the 
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 Fork by Mickey Chan
 */

#include "Battery.h"
#include <Arduino.h>

Battery::Battery(uint16_t minVoltage, uint16_t maxVoltage, uint8_t sensePin, uint16_t resolution) {
	this->sensePin = sensePin;
	this->activationPin = 0xFF;
	this->minVoltage = minVoltage;
	this->maxVoltage = maxVoltage;
	this->dacResolution = resolution;
}

void Battery::begin(uint16_t refVoltage, float dividerRatio, mapFn_t mapFunction) {
	this->refVoltage = refVoltage;
	this->dividerRatio = dividerRatio;
	pinMode(this->sensePin, INPUT);
	if (this->activationPin < 0xFF) {
		pinMode(this->activationPin, OUTPUT);
	}
	this->mapFunction = mapFunction ? mapFunction : &linear;
}

void Battery::onDemand(uint8_t activationPin, uint8_t activationMode) {
	if (activationPin < 0xFF) {
		this->activationPin = activationPin;
		this->activationMode = activationMode;
		pinMode(this->activationPin, OUTPUT);
		digitalWrite(activationPin, !activationMode);
	}
}

uint8_t Battery::level() {
	return this->level(this->voltage());
}

uint8_t Battery::level(uint16_t voltage) {
	if (voltage <= minVoltage) {
		return 0;
	} else if (voltage >= maxVoltage) {
		return 100;
	} else {
		return (*mapFunction)(voltage, minVoltage, maxVoltage);
	}
}

uint16_t Battery::voltage() {
	if (activationPin != 0xFF) {
		digitalWrite(activationPin, activationMode);
		delayMicroseconds(10); // copes with slow switching activation circuits
	}
	analogRead(sensePin);
	delay(2); // allow the ADC to stabilize
	uint16_t reading = analogRead(sensePin) * dividerRatio * refVoltage / this->dacResolution;
	if (activationPin != 0xFF) {
		digitalWrite(activationPin, !activationMode);
	}
	return reading;
}

// Added by Mickey
void Battery::setUpdateInterval(uint16_t interval) {
	this->updateInterval = interval;
}

void Battery::loop() {
	unsigned long curTime = millis();
	if (this->curReadState == IDLE && (unsigned long)(curTime - this->lastPassAt) >= this->updateInterval) {
		if (this->activationPin != 0xFF) {
			digitalWrite(this->activationPin, this->activationMode);
		}
		this->lastPassAt = curTime;
		this->curReadState = PASS_1;
		return;
	} else if (this->curReadState == PASS_1 && (unsigned long)(curTime - this->lastPassAt) >= 1) {
		analogRead(this->sensePin);
		this->lastPassAt = curTime;
		this->curReadState = PASS_2;
		return;
	} else if (this->curReadState == PASS_2 && (unsigned long)(curTime - this->lastPassAt) >= 2000) {
		uint16_t reading = analogRead(this->sensePin) * this->dividerRatio * this->refVoltage / this->dacResolution;
		if (this->activationPin != 0xFF) {
			digitalWrite(this->activationPin, !this->activationMode);
		}
		this->lastVoltage = reading;
		this->lastPassAt = curTime;
		this->curReadState = IDLE;
		return;
	}
}