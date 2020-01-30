#include "main.h"

using namespace pros;

struct controllerState {
	int left;
	int right;
	bool rightBumper1;
	bool leftBumper1;
	bool rightBumper2;
	bool leftBumper2;
	bool grabblerFresh;
	bool a;
	bool b;
	bool y;
	bool rarrow;
	bool uarrow;
	bool darrow;
} controls;

Controller master (E_CONTROLLER_MASTER);

Motor frontLeftMotor(19);  //rotations = distance/circumference -- circumference = 12.5663706144 inches (maybe use 4.15 if this doesn't work)
Motor frontRightMotor(17);
Motor backLeftMotor(11);
Motor backRightMotor(18);
Motor leftGrabblerMotor(14);
Motor rightGrabblerMotor(7);
Motor retractRamp(15);
Motor arm(9);

controllerState getControllerState (){
	controls.left = master.get_analog(ANALOG_LEFT_Y); //y val of left stick
	controls.right = master.get_analog(ANALOG_RIGHT_Y); // y val of right stick

	controls.rightBumper1 = master.get_digital(DIGITAL_R1);
	controls.leftBumper1 = master.get_digital(DIGITAL_L1);
	controls.rightBumper2 = master.get_digital(DIGITAL_R2);
	controls.leftBumper2 = master.get_digital(DIGITAL_L2);
	controls.a = master.get_digital_new_press(DIGITAL_A);
	controls.b = master.get_digital(DIGITAL_B);
	controls.y = master.get_digital(DIGITAL_Y);
	controls.rarrow = master.get_digital(DIGITAL_RIGHT);
	controls.uarrow = master.get_digital(DIGITAL_UP);
	controls.darrow = master.get_digital(DIGITAL_DOWN);

	return controls;
}

void grabblerSucc (int grabblerRotation){ // spins grabblers, negative direction --> sucks out
	leftGrabblerMotor.move(grabblerRotation);
	rightGrabblerMotor.move(-1*grabblerRotation);
}

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "UwU");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "4393Z RISE UP");
	pros::lcd::register_btn1_cb(on_center_button);
//----------------------------------------------------------------------------------------------------------------------experimentalish
	frontLeftMotor.set_brake_mode(E_MOTOR_BRAKE_COAST);//set_brake_mode(E_MOTOR_BRAKE_COAST); -- for drive motors
	frontRightMotor.set_brake_mode(E_MOTOR_BRAKE_COAST); // coast = will continue on momentum when button no longer pressed
	backLeftMotor.set_brake_mode(E_MOTOR_BRAKE_COAST);
	backRightMotor.set_brake_mode(E_MOTOR_BRAKE_COAST);
	 retractRamp.set_brake_mode(E_MOTOR_BRAKE_HOLD); //hold for lift motor
	 arm.set_brake_mode(E_MOTOR_BRAKE_HOLD);  // hold = stops motor where it is when button is no longer pressed
	 leftGrabblerMotor.set_brake_mode(E_MOTOR_BRAKE_HOLD);
	 rightGrabblerMotor.set_brake_mode(E_MOTOR_BRAKE_HOLD);

}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
//	autonomous();
}

void setDriveVoltage(int voltL, int voltR){
	frontLeftMotor=voltL;
	frontRightMotor=-voltR;
	backRightMotor=-voltR;
	backLeftMotor=voltL;
}

void ResetDrive(){
	frontLeftMotor.tare_position();
	frontRightMotor.tare_position();
	backRightMotor.tare_position();
	backLeftMotor.tare_position();
}

double avgDriveEncoderVal(){
	return (fabs(frontLeftMotor.get_position()) + fabs(frontRightMotor.get_position()) + fabs(backLeftMotor.get_position()) + fabs(backRightMotor.get_position()))/4;
}

void translate (int units, int voltage, bool intake, bool outake){ //units is very small, 500 goes almost nowhere. Use negatives for backward, true if you want it to intake while it drives
	int direction = abs(units)/units;

	ResetDrive();

	while(avgDriveEncoderVal()<abs(units)){
		setDriveVoltage(voltage*direction,voltage*direction);

		if (intake){ // might have to use/reset encoder vals if this doesn't work
			grabblerSucc(voltage);
		}
		if (outake){
			grabblerSucc(-voltage); // might have to reduce speed here
		}

		delay(10);
	}

	setDriveVoltage(-10*direction,-10*direction); //brief brake

	if (intake){ // brake intake as well if it is set to true
		grabblerSucc(0);
	}
	if (outake){
		grabblerSucc(0);
	}

	delay(75);

	setDriveVoltage(0,0);
}

void stopwheels(){
	frontLeftMotor.move(0);
	frontRightMotor.move(0);
	backRightMotor.move(0);
	backLeftMotor.move(0);
}

void turnRightNoGyro(int voltage, int time){
		frontLeftMotor.move(voltage);
		frontRightMotor.move(voltage);
		backRightMotor.move(voltage);
		backLeftMotor.move(voltage);
		delay (time);
		stopwheels();
}

void turnLeftNoGyro(int voltage, int time){
		frontLeftMotor.move(voltage);
		frontRightMotor.move(voltage);
		backRightMotor.move(voltage);
		backLeftMotor.move(voltage);
		delay (time);
		stopwheels();
}

void raiseRamp(int volt, int time){ //positive = raise up, negative = retract
	retractRamp.move(-volt);
	delay(time);
	retractRamp.move(0);
}

void intakeOutake(int volt, int time){ //positive for intake, negative for outake
	grabblerSucc(volt);
	delay(time);
	grabblerSucc(0);
}

void blueRightCorner(){
	translate(1500, 100, true, false);
	//delay(500);
	intakeOutake(75,500);
	delay(500);
	turnRightNoGyro(100,350); //maybe reduce turning speed, increase time //TEST THIS TURN
	delay(500);
	translate(1500, 100, true, false);
	delay(500);
	turnRightNoGyro(100, 200);
	delay(500);
	translate(500,100,false, false);
//  intakeOutake(-75,700);
	delay(300);
  raiseRamp(75,1500);
	delay(300);
  translate(-1000,50, false, true);


}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	blueRightCorner();
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	/// test & adjust these
	const int grabblerSpeed = 127; // out of 127
	const int tickLength = 5; // in ms
	const int grabblerRamp = 100; // out of 127

	while (true) {
		controls = getControllerState();
////////////////////////////////////////////////////////////driving////////////////////////////////////////////////////////////
		frontLeftMotor.move(controls.left);
		backLeftMotor.move(controls.left);
		frontRightMotor.move(-1*controls.right); // for right motors, negative motor spin needs to be used to go forward w/ left positive motors
		backRightMotor.move(-1*controls.right);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////__amd64

		if (controls.a){ // for testing auto, comment out for competition
			delay(500);
			autonomous();
		  delay(500);
		}

//////////////////////////////////////////////////intake and outtake///////////////////////////////////////////////////////
		if (controls.leftBumper1){
			grabblerSucc (-1*grabblerSpeed); // succ out le blocks
		}
	else	if (controls.leftBumper2){
			grabblerSucc (grabblerSpeed); // succ in le blocks
		}
			else {
			grabblerSucc (0); // stop grabbler movement
		}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////move le ramp (chimp)/////////////////////////////////////////////////////
		// if (controls.rightBumper1 || controls.rightBumper2||controls.b){
		// 	if (controls.rightBumper2){ // move tower out
		// 		retractRamp.move(-1*grabblerRamp);  //maybe swap the retract ramp stuff if it is backwards
		// 	}
		// 	else if(controls.b){
		// 		retractRamp.move(-1*grabblerRamp/2);
		// 	}
		// 	 else { // move tower in
		// 		retractRamp.move(grabblerRamp);
		// 	}
		// } else {
		// 	retractRamp.move(0);
		// }

		if (controls.rightBumper2){ // move tower out
			retractRamp.move(-1*grabblerRamp);
		}
		else if(controls.b){
			retractRamp.move(-1*grabblerRamp/2); //slow move out
		}
		else if(controls.rightBumper1){ // move tower in
			retractRamp.move(grabblerRamp);
		}
	  else {
		  retractRamp.move(0);
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////moving arm//////////////////////////////////////////////////////////
		if (controls.y){ //move arm up
			arm.move(100);
		}
		else if (controls.rarrow){ // move arm down
			arm.move(-1*100);
		}
		else if (controls.uarrow){ //arm up slower
			arm.move(50);
		}
		else if (controls.darrow){ //arm down slower
			arm.move(-1*50);
		}
		else {
			arm.move(0);
		}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		delay(tickLength);
	}
}
