#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//define colours to be used
#define red 1
#define green 2
#define yellow 3
#define purple 5
#define white 7

//define states
#define SYNC 0
#define MAIN 1
#define PRESSING 2

//create arrays
char nameArr[26] = {0};
String descArr[26] = {"0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0","0"};
short valArr[26] = {0};
short minArr[26] = {0};
short maxArr[26] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};

//create state variables
short lastState = SYNC;
short state = SYNC;

//create button state variables
short lastStateUp = 0;
short currentStateUp = 0;

short lastStateDown = 0;
short currentStateDown = 0;

short lastStateSelect = 0; 
short currentStateSelect = 0;
bool isPressingSelect = false;
bool isLongDetectedSelect = false;    
long selectPressedTime = 0;

//create print variable
short startIndex = 0;

//create custom lcd character byte arrays
//Demonstration of UDCHARS extension
byte upAr[] = {
  	B00000,
  	B00100,
  	B01110,
  	B00100,
  	B00100,
  	B00100,
  	B00100,
  	B00000
};

byte dnAr[] = {
  	B00000,
  	B00100,
  	B00100,
  	B00100,
  	B00100,
  	B01110,
  	B00100,
	B00000
};

//create setup function

void setup() {
    Serial.begin(9600);
    lcd.begin(16,2);

    //use custom char arrays to create custom characters
    lcd.createChar(00, upAr);
    lcd.createChar(01, dnAr);

}

//create sub-state functions (functions needed by the state functions)

int asciiIndex(char el) {
    return (int(el) - 65);
}

int checkElements(char el) {
    if (nameArr[asciiIndex(el)] == el){
        //return index if channel exists
        return asciiIndex(el);
    } else {
        //return -1 if channel does not exist 
        return -1;
    }
}

void checkVals() {
    short suitCount; //stores the number of suitable values
    //check all short arrays at the same index, to see if values are suitable
    for (short i = 0; i<26; i++) {
        if (valArr[i] < minArr[i] && minArr[i] < maxArr[i]) {
            //initialise count variable
            short maxCount = 0;
            //start check for subsequent more than max values
            for (short j = 0; j<26; j++){
                if (valArr[j] > maxArr[j]){
                maxCount += 1;
                }
            }
            if (state == MAIN) {
                if (maxCount > 0){
                lcd.setBacklight(yellow);
                } else {
                lcd.setBacklight(green);
                }
            }
		} else if (valArr[i] > maxArr[i] && minArr[i] < maxArr[i]) {
            short minCount = 0;
			for (short j = 0; j<26; j++){
                if (valArr[j] < minArr[j]){
                    minCount += 1;
                }
            }
            if (state == MAIN) {
                if (minCount > 0){
                    lcd.setBacklight(yellow);
                } else {
                    lcd.setBacklight(red);
                }
            }
		} 
		//after all checks are passed, value is suitable
		else {
            suitCount++;
            if (suitCount == 26) {
                lcd.setBacklight(white);
            }
		}
    }
}

int lastNonZero(short endIndex) {
    for (short i = endIndex - 1; i>=0; i--){
        if (nameArr[i] != 0){
            return i;
        }
    }
    return -1;
}

int nonZero(short start) {
    for (short i = start; i<26; i++) {
        if (nameArr[i] != 0) {
            return i;
        }
    }
    return -1;
}

void printToLCD() {
    //find first index
    int printIndex = nonZero(startIndex);
    //This code below was used to help me determine the outcome of the button presses in relation the the array indexing
    //Serial.println(startIndex);
    //Serial.println(printIndex);

    //set cursor to appropriate place
    lcd.setCursor(0,0);
    lcd.print(" ");
    //print arrow only if the index is not the first non zero index
    if(printIndex != nonZero(-1)){
        lcd.setCursor(0,0);
        //Demonstration of UDCHARS extension
        lcd.write(00);   
    }
    if (printIndex > -1) {
        lcd.setCursor(1,0);
        lcd.print(nameArr[printIndex]);
        lcd.setCursor(2,0);
        lcd.print("   ");
        lcd.setCursor(2,0);
        if (valArr[printIndex] != -1) {
            if (valArr[printIndex] < 10) {
                lcd.print("  ");
                lcd.print(valArr[printIndex]);
            } else if (valArr[printIndex] < 100) {
                lcd.print(" ");
                lcd.print(valArr[printIndex]);
            } else {
                lcd.print(valArr[printIndex]);
            }
        }
        //Demonstration of NAMES extension
        lcd.setCursor(5,0);
        lcd.print(" ");
        if (nameArr[printIndex] != '0'){
            lcd.print(descArr[printIndex]);
            for (int i=6 + descArr[printIndex].length(); i<17; i++){
                lcd.setCursor(i,0);
                lcd.print(" ");
            }
        }
    }

    short printIndexSec = nonZero(printIndex+1);
    //Likewise here, it proved helpful
    //Serial.println(printIndex);
    //Serial.println(printIndexSec);

    lcd.setCursor(0,1);
    lcd.print(" ");
    //prints down arrow if printIndexSec is not the last non 0 element
    if (printIndexSec != lastNonZero(26) && printIndexSec != printIndex && printIndexSec != -1) { 
        lcd.setCursor(0,1);
        lcd.write(01);
    }
    if (printIndexSec != -1 && printIndexSec != printIndex) {
        lcd.setCursor(1,1);
        lcd.print(nameArr[printIndexSec]);
        lcd.setCursor(2,1);
        lcd.print("   ");
        lcd.setCursor(2,1);
        if (valArr[printIndexSec] != -1) {
            if (valArr[printIndexSec] < 10) {
                lcd.print("  ");
                lcd.print(valArr[printIndexSec]);
            } else if (valArr[printIndexSec] < 100) {
                lcd.print(" ");
                lcd.print(valArr[printIndexSec]);
            } else {
                lcd.print(valArr[printIndexSec]);
            }
        }
        lcd.setCursor(5,1);
        lcd.print(" ");
        if (nameArr[printIndexSec] != '0'){
            lcd.print(descArr[printIndexSec]);
            for (int i=6 + descArr[printIndexSec].length(); i<17; i++){
                lcd.setCursor(i,1);
                lcd.print(" ");
            }
        }
    }
}

//create state functions

void sync() {
    lcd.clear();
    lcd.setBacklight(purple);
	Serial.print('Q');
    state = SYNC;
	while(Serial.available()){
		char read = Serial.read();
		if (read=='X'){
			state = MAIN;
			break;
		} else {
            state = SYNC;
			break;
		}
	}
    if (state == MAIN) {
        transition();
    }
    delay(1000);
}

void transition() {
    lcd.clear();
    lcd.setBacklight(white);
    Serial.print("UDCHARS,NAMES\n");
    delay(2000);
}

void mainState() {
    lcd.clear();
    while (state == MAIN){
        currentStateUp = lcd.readButtons();

        if (lastStateUp == 0 && currentStateUp & BUTTON_UP){
            //find the first non 0 element before the current print 
            //index by decrementing the start index before it gets called 
            //to make the new print index
            if (startIndex > nonZero(0)) {
                startIndex = lastNonZero(startIndex);
            }
        }

        lastStateUp = currentStateUp;

        currentStateDown = lcd.readButtons();

        if (lastStateDown == 0 && currentStateDown & BUTTON_DOWN){
            //finds next element and uses its index if it is more than one 
            //element away from the final non 0 element

            if (nonZero(nonZero(startIndex) + 1) != lastNonZero(26)) {
                startIndex = nonZero(startIndex+1);
            }
        }

        lastStateDown = currentStateDown;

        //add select hold functionality to change state to PRESSING
        //and break from the mainState function
        currentStateSelect = lcd.readButtons() & BUTTON_SELECT;

        if (lastStateSelect == 0 && currentStateSelect == 1) {        // button is pressed
            selectPressedTime = millis();
            isPressingSelect = true;
            isLongDetectedSelect = false;
        } else if (lastStateSelect == 1 && currentStateSelect == 0) {
            isPressingSelect = false;
        }

        if(isPressingSelect == true && isLongDetectedSelect == false) {
            long pressDuration = millis() - selectPressedTime;

            if(pressDuration > 1000) {
                //set state to PRESSING and exit from the function
                state = PRESSING;
                break;
            }
        } 
        // save the the last state
        lastStateSelect = currentStateSelect;

        while(Serial.available()){
            //take string from Serial input
            String indicator = Serial.readString();
            //create index, this will be used a lot later 
            short index = checkElements(indicator[1]);
            //check for indicator type
            if (indicator[0] == 'C') {
                //check if channel exists 
                if (index != -1) {
                    //make sure desc length <= 15
                    if (indicator.substring(2, indicator.length()-1).length() > 15) {
                        //add new description to appropriate descArr index
                        descArr[index] = indicator.substring(2, 17);
                    } else {
                        descArr[index] = indicator.substring(2, indicator.length()-1);
                    }
                    
                } else {
                    //if channel does not exist, add name and desc to appropriate arrays
                    nameArr[asciiIndex(indicator[1])] = indicator[1];
                    if (indicator.substring(2, indicator.length()-1).length() > 15) {
                        descArr[asciiIndex(indicator[1])] = indicator.substring(2, 17);
                    } else {
                        descArr[asciiIndex(indicator[1])] = indicator.substring(2, indicator.length()-1);
                    }
                }
            } else if (indicator[0] == 'V') {
                //check if channel exists
                if (index != -1) {
                    valArr[index] = indicator.substring(2).toInt();
                }
            } else if (indicator[0] == 'X') {
                if (index != -1) {
                    //change appropriate array's max value
                    maxArr[index] = indicator.substring(2).toInt();
                }
            } else if (indicator[0] == 'N') {
                if (index != -1) {
                    minArr[index] = indicator.substring(2).toInt();
                }
            } else if (indicator.substring(0,6) == "DEBUG:") {
                //send DEBUG message to Serial
                Serial.print(indicator);
            } else {
                //send to serial as an error message
                if (indicator.length() > 0 && indicator != "\n"){
                    Serial.print("ERROR: "); 
                    Serial.print(indicator);
                }
            }
        }
        checkVals();
        printToLCD();
    }
}

void pressing() {
    //clear display, set to purple, write student number
    lcd.clear();
    lcd.setBacklight(purple);
    lcd.print("F123531");
    while (state == PRESSING){                    
        isLongDetectedSelect = true;

        currentStateSelect = lcd.readButtons() & BUTTON_SELECT;

        if (lastStateSelect == 1 && currentStateSelect == 0) { // button is released
            isPressingSelect = false;
            long offPressDuration = millis() - selectPressedTime;
            if(offPressDuration > 1000){
                lcd.clear();
                lcd.setBacklight(white);
                state = MAIN;
                break;
            }
        }

        lastStateSelect = currentStateSelect;

        //same functionality as mainState() without the printing to screen

        currentStateUp = lcd.readButtons();

        if (lastStateUp == 0 && currentStateUp & BUTTON_UP){
            //find the first non 0 element before the current print 
            //index by decrementing the start index before it gets called 
            //to make the new print index
            if (startIndex > nonZero(0)) {
                startIndex = lastNonZero(startIndex);
            }
        }

        lastStateUp = currentStateUp;

        currentStateDown = lcd.readButtons();

        if (lastStateDown == 0 && currentStateDown & BUTTON_DOWN){
            //finds next element and uses its index if it is more than one 
            //element away from the final non 0 element
            if (nonZero(nonZero(startIndex)) != -1){
                startIndex = nonZero(startIndex);
            }
        }

        lastStateDown = currentStateDown;

        while(Serial.available()) {
            //take string from Serial input
            String indicator = Serial.readString();
            //create index, this will be used a lot later 
            short index = checkElements(indicator[1]);
            //check for indicator type
            if (indicator[0] == 'C') {
                //check if channel exists 
                if (index != -1) {
                    //make sure desc length <= 15
                    if (indicator.substring(2, indicator.length()-1).length() > 15) {
                        //add new description to appropriate descArr index
                        descArr[index] = indicator.substring(2, 17);
                    } else {
                        descArr[index] = indicator.substring(2, indicator.length()-1);
                    }
                    
                } else {
                    //if channel does not exist, add name and desc to appropriate arrays
                    nameArr[asciiIndex(indicator[1])] = indicator[1];
                    if (indicator.substring(2, indicator.length()-1).length() > 15) {
                        descArr[asciiIndex(indicator[1])] = indicator.substring(2, 17);
                    } else {
                        descArr[asciiIndex(indicator[1])] = indicator.substring(2, indicator.length()-1);
                    }
                }
            } else if (indicator[0] == 'V') {
                //check if channel exists
                if (index != -1) {
                    if (indicator.substring(2).toInt() < minArr[index] && indicator.substring(2).toInt() > maxArr[index]) {
                        lcd.setBacklight(yellow);
                    } else if (indicator.substring(2).toInt() < minArr[index]) {
                        lcd.setBacklight(green);
                    } else if (indicator.substring(2).toInt() > maxArr[index]) {
                        lcd.setBacklight(red);
                    } 
                    //after all checks are passed, value is suitable and should be added 
                    else {
                        valArr[index] = indicator.substring(2).toInt();
                        lcd.setBacklight(white);
                    }
                }
            } else if (indicator[0] == 'X') {
                if (index != -1) {
                    //change appropriate array's max value
                    maxArr[index] = indicator.substring(2).toInt();
                }
            } else if (indicator[0] == 'N') {
                if (index != -1) {
                    minArr[index] = indicator.substring(2).toInt();
                }
            } else if (indicator.substring(0,6) == "DEBUG:") {
                //send DEBUG message to Serial
                Serial.print(indicator);
            } else {
                //send to serial as an error message
                if (indicator.length() > 0 && indicator != "\n") {
                    Serial.print("ERROR: "); 
                    Serial.print(indicator);
                }
            }
        }
        checkVals();
    }
}

//create loop function/state handler

void loop() {
	switch (state) {
		case SYNC:
			sync(); 
			break;
		case MAIN:
			mainState(); 
			break;
		case PRESSING: 
			pressing(); 
			break;
	}
    //save states for transition call
    lastState = state;
}