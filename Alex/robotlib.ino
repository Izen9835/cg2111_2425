#include <AFMotor.h>
// Direction values
// Motor control
#define FRONT_RIGHT  2 // M1 on the driver shield
#define BACK_RIGHT   1 // M2 on the driver shield

AF_DCMotor motorFR(FRONT_RIGHT);
AF_DCMotor motorBR(BACK_RIGHT);

void move(float speed, int direction)
{
  int speed_scaled = (speed/100.0) * 255;
  motorFR.setSpeed(speed_scaled);
  motorBR.setSpeed(speed_scaled);

  switch(direction)
    {
      case BACK:
        motorFR.run(BACKWARD);
        motorBR.run(FORWARD); 
      break;
      case GO:
        motorFR.run(FORWARD);
        motorBR.run(BACKWARD); 
      break;
      case CW:
        motorFR.run(FORWARD);
        motorBR.run(FORWARD); 
      break;
      case CCW:
        motorFR.run(BACKWARD);
        motorBR.run(BACKWARD); 
      break;
      case STOP:
      default:
        motorFR.run(STOP);
        motorBR.run(STOP);
    }
}

void forward(float dist, float speed)
{
  if (dist > 0){
    deltaDist = dist;
  }
  else{
    deltaDist = 999999;
  }
  newDist = forwardDist + deltaDist;
  dir = (TDirection) FORWARD; // GO
  move(speed, FORWARD);
}

void backward(float dist, float speed)
{
  if(dist > 0){
    deltaDist = dist;
  }
  else{
    deltaDist = 9999999;
  }
  newDist = reverseDist + deltaDist;
  dir = (TDirection) BACKWARD;
  move(speed, BACKWARD);
}

void ccw(float dist, float speed)
{
  dir = (TDirection) LEFT; // CCW
  move(speed, CCW);
}

void cw(float dist, float speed)
{
  dir = (TDirection) RIGHT; // CW
  move(speed, CW);
}

void stop()
{
  dir = (TDirection) STOP; // defaults to 0 as not defined in enum TDirection
  move(0, STOP);
}
