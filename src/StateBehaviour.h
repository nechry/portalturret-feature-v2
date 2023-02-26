#define IDNAME(name) #name

enum class TurretMode
{
  Automatic = 0,
  Manual = 1,
  Unknown = -1
};

enum class TurretState {
  Idle = 0,
  Activated = 1,
  Searching = 2,
  Engaging = 3,
  TargetLost = 4,
  PickedUp = 5,
  Shutdown = 6,
  ManualEngaging = 7,
  Rebooting = 8,
};

enum class ManualState {
  Idle,
  Opening,
  Closing,
  Firing,
};

TurretMode currentTurretMode;
TurretState currentState = TurretState::Idle;
ManualState currentManualState = ManualState::Idle;

unsigned long detectTime = 0;
unsigned long undetectedTime = 0;
unsigned long previousTime = 0;
unsigned long stateStartTime = 0;
unsigned long lastMovementTime = 0;

void setState(TurretState nextState) {

  //Stop the Wing Servos just in case;
  int degree = wingServo.read();
if(degree != STATIONARY_ANGLE){
  debugln("Wing Servo need to Stop!");
  wingServo.write(STATIONARY_ANGLE);  
}  
  
  if (currentTurretMode == TurretMode::Automatic) {
    debug("Automatic Mode -");
    debug("Setting TurretState to: ");

    switch (nextState) {
      case TurretState::Activated:
      debugln("Activated");
        activatedRoutine.reset();
        break;
      case TurretState::Engaging:
      debugln("Engaging");
        engagingRoutine.reset();
        break;
      case TurretState::Searching:
      debugln("Searching");
        searchingRoutine.reset();
        break;
      case TurretState::TargetLost:
      debugln("TargetLost");
        targetLostRoutine.reset();
        break;
      case TurretState::PickedUp:
      debugln("PickedUp");
        pickedUpRoutine.reset();
        break;
      case TurretState::Shutdown:
      debugln("Shutdown");
        shutdownRoutine.reset();
        break;
      case TurretState::ManualEngaging:
      debugln("ManualEngaging");
        manualEngagingRoutine.reset();
        break;
      case TurretState::Rebooting:
      debugln("Rebooting");
        rebootRoutine.reset();
        break;
      case TurretState::Idle:
        //debugln("Idle");
        break;
    }
    stateStartTime = millis();
    currentState = nextState;
  }
}

void setManualState(ManualState nextState) {

  //Stop the Wing Servos just in case;
  int degree = wingServo.read();
  if(degree != STATIONARY_ANGLE){
  debugln("Wing Servo need to Stop!");
  wingServo.write(STATIONARY_ANGLE);  
} 
  
  if (currentTurretMode == TurretMode::Manual) {
    debug("Manual Mode -");
    debugln("Setting ManualState to: " + String(IDNAME(nextState)));
    switch (nextState) {
      case ManualState::Opening:
        openWingsRoutine.reset();
        break;
      case ManualState::Closing:
        closeWingsRoutine.reset();
        break;
      case ManualState::Firing:
        manualEngagingRoutine.reset();
        break;
    }
    currentManualState = nextState;
  }
}

void manualRotation(unsigned long deltaTime) {
  debugln("Manual Rotation");
  manualMovementRoutine.runCoroutine();
}
void stateBehaviour() {

  unsigned long deltaTime = millis() - previousTime;
  previousTime = millis();

  if (currentTurretMode == TurretMode::Manual) {
    switch (currentManualState) {
      case ManualState::Idle:
        manualRotation(deltaTime);
        break;
      case ManualState::Opening:
        openWingsRoutine.runCoroutine();
        if (openWingsRoutine.isDone()) {
          setManualState(ManualState::Idle);
        }
        break;
      case ManualState::Closing:
        closeWingsRoutine.runCoroutine();
        if (closeWingsRoutine.isDone()) {
          setManualState(ManualState::Idle);
        }
        break;
      case ManualState::Firing:
        manualRotation(deltaTime);
        manualEngagingRoutine.runCoroutine();
        if (manualEngagingRoutine.isDone()) {
          setManualState(ManualState::Idle);
        }
        break;
    }
  }
  if (currentTurretMode == TurretMode::Automatic) {
    
    bool motionDetected = isDetectingMotion();
    float zMovement = (smoothZ / measurements * SENSORS_GRAVITY_STANDARD * ADXL345_MG2G_MULTIPLIER);
   // debugln("Z Movement: " + String(zMovement));
    bool pickedUp = accelerometerBuffered && (zMovement < GFORCE_PICKED_UP_MIN || zMovement > GFORCE_PICKED_UP_MAX);
    bool movedAround = accelerometerBuffered && (zMovement < GFORCE_STEADY_MIN || zMovement > GFORCE_STEADY_MAX);
    bool onItsSide = accelerometerBuffered && (zMovement < TIPPED_OVER_Z_TRESHOLD);
    
    if (movedAround) {
      lastMovementTime = millis();
    }

    if (pickedUp && currentState != TurretState::PickedUp && currentState != TurretState::Shutdown && currentState != TurretState::Rebooting) {
      setState(TurretState::PickedUp);
    }
    
    switch (currentState) {
      case TurretState::Idle:
        if (motionDetected) {
          setState(TurretState::Activated);
        }
        break;
      case TurretState::Activated:
        activatedRoutine.runCoroutine();
        if (activatedRoutine.isDone()) {
          if (motionDetected) {
            setState(TurretState::Engaging);
          } else {
            setState(TurretState::Searching);
          }
        }
        break;
      case TurretState::Searching:
        searchingRoutine.runCoroutine();
        if (millis() > stateStartTime + 10000) {
          setState(TurretState::TargetLost);
        }
        if (motionDetected && millis() > stateStartTime + 100) {
          setState(TurretState::Engaging);
        }
        break;
      case TurretState::TargetLost:
        targetLostRoutine.runCoroutine();
        if (targetLostRoutine.isDone()) {
          setState(TurretState::Idle);
        }
        break;
      case TurretState::Engaging:
        engagingRoutine.runCoroutine();
        if (engagingRoutine.isDone()) {
          if (wingsOpen) {
            setState(TurretState::Searching);
          } else {
            setState(TurretState::Idle);
          }
        }
        break;
      case TurretState::PickedUp:
        pickedUpRoutine.runCoroutine();
        if (onItsSide) {
          setState(TurretState::Shutdown);
        }
        else if (!movedAround && millis() > lastMovementTime + 5000) {
          setState(TurretState::Activated);
        }
        break;
      case TurretState::Shutdown:
        shutdownRoutine.runCoroutine();
        if (shutdownRoutine.isDone() && !onItsSide) {
          setState(TurretState::Rebooting);
        }
        break;
      case TurretState::Rebooting:
        rebootRoutine.runCoroutine();
        if (rebootRoutine.isDone()) {
          setState(TurretState::Idle);
        }
        break;
    }
  }
}
