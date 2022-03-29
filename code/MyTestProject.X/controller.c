/*
 * File:   controller.c
 * Author: Miriam Welser
 *
 * Created on March 28, 2022, 9:12 PM
 */


#include "xc.h"
#include "stdbool.h"

#define PI = 3.14159265359;
#define oneTurnDistance = PI * 0.6; // distance of one turn in cm

int main(void) {
    return 0;
}



static Sensors s;
static Encoders e;
static Velocities v;

void initSensors(){
    s = {0.0,0.0,0.0,0.0,0.0,0.0};
    e = {0.0,0.0,0.0,0.0,};
}  

CurrentMeasurements getSensorData(void)
{
    float sensorsLeft = get_left_measurement();
    float sensorsRight = get_right_measurement();
    float sensorsFront = get_front_measurement();    
    float timeSensorsBetweenTwoSignals = get_timestep_measurement();
    
    float encodersLeft = get_left_encoder();
    float encodersRight = get_right_encoder();    
    float timeEncoderBetweenTwoSignals = get_timestep_encoder(); 
    
    CurrentMeasurements output = {sensorsLeft, sensorsRight, sensorsFront, timeSensorsBetweenTwoSignals, encodersLeft, encodersRight, timeEncoderBetweenTwoSignals};
    return output;
}

void updateSensorsAndEncoders()
{
    CurrentMeasurements current = getSensorData(void);
    
    s.sLeftLast = s.sLeftNow;
    s.sRightLast = s.sRightNow;
    s.sFrontLast = s.sFrontNow;
    e.eLeftLast = e.eLeftNow;
    e.eRightLast = e.eRightNow;
    
    s.sLeftNow = current.sLeftNow;
    s.sRightNow = current.sRightNow;
    s.sFrontNow = current.sFrontNow;
    e.eLeftNow = current.eLeftNow;
    e.eRightNow = current.eRightNow;
    
}

void computeVelocities(Encoders e, float t)
{
    v.vLeft = e.eLeftNow * oneTurnDistance / t;
    v.vRight = e.eRightNow * oneTurnDistance / t;        
}

float giveCommandToLeftMotor()
{
    
    
    return velocities;
}

float giveCommandToRightMotor()
{
    
}

void controlHighlevel(Sensors s, Velocities v)
{
    if (isWallLeft(s) && isWallRight(s) && !isWallFront(s)){
        continueStraight();
        controlMidOfCell();
    }
    else if (isWallLeft(s) && isWallRight(s) && !isWallFront(s)){
        turn180();
    }
    else if (!isWallLeft(s) && isWallRight(s) && isWallFront(s)){
        turnLeft90();
    }
    else if (isWallLeft(s) && !isWallRight(s) && isWallFront(s)){
        turnRight90();
    }
    else if (!isWallLeft(s) && !isWallRight(s) && isWallFront(s)){
        decideLeftOrRight();
    }
    else if (!isWallLeft(s) && isWallRight(s) && !isWallFront(s)){
        decideLeftOrFront();
    }
    else if (isWallLeft(s) && !isWallRight(s) && !isWallFront(s)){
        decideRightOrFront();
    }
    else if (!isWallLeft(s) && !isWallRight(s) && !isWallFront(s)){
        decideLeftOrRightOrFront();
    }
}


void controlMidOfCell(Sensors s, Velocities v)
{
    float dLeft = s.sLeftNow;
    float dRight = s.sRightNow;
    if (dLeft > dRight){
        // increase velocity right and decrease velocity left
    }
    else if (dLeft < dRight){
        // increase velocity left and decrease velocity right
    }
    else{
        continueStraight();
    }
}


float controlPID(float toBeControlled, float kP, float kI, float kD){
    // control value
    return toBeControlled;
}
