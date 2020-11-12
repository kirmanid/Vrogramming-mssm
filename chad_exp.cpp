#include "main.h"
//#include "globals.cpp"
//#include "globals.hpp"
//#include "Rollers.cpp"
//#include "intake.hpp"

using namespace pros;

struct controllerState {
	int left;
	int right;
	bool rightBumper1;
	bool leftBumper1;
	bool rightBumper2;
	bool leftBumper2;
	bool a;
	bool b;
	bool y;
	bool rarrow;
	bool uarrow;
	bool darrow;
} controls;

pros::Controller master(pros::E_CONTROLLER_MASTER);
//Globals
Motor frontLeftMotor(7);  //rotations = distance/circumference -- circumference = 12.5663706144 inches (maybe use 4.15 if this doesn't work)
Motor frontRightMotor(11);
Motor backLeftMotor(15);
Motor backRightMotor(1);

Motor TopRoller(12);
Motor BottomRoller(4);
Motor intakeL(20);
Motor intakeR(13);


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

void intakeSucc (int intakeRotation){ // spins intakes, negative direction --> sucks out
	intakeL.move(intakeRotation);
	intakeR.move(-1*intakeRotation);
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
	pros::lcd::set_text(1, "4393G RISE UP");
	pros::lcd::register_btn1_cb(on_center_button);
//----------------------------------------------------------------------------------------------------------------------experimentalish
	frontLeftMotor.set_brake_mode(E_MOTOR_BRAKE_COAST);//set_brake_mode(E_MOTOR_BRAKE_COAST); -- for drive motors
	frontRightMotor.set_brake_mode(E_MOTOR_BRAKE_COAST); // coast = will continue on momentum when button no longer pressed
	backLeftMotor.set_brake_mode(E_MOTOR_BRAKE_COAST);
	backRightMotor.set_brake_mode(E_MOTOR_BRAKE_COAST);
	intakeL.set_brake_mode(E_MOTOR_BRAKE_HOLD);
	intakeR.set_brake_mode(E_MOTOR_BRAKE_HOLD);
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

void translate (int units, int voltage, bool intake, bool outake, int intakeVolt, int outakeVolt){ //units is very small, 500 goes almost nowhere. Use negatives for backward, true if you want it to intake while it drives
	int direction = abs(units)/units;

	ResetDrive();

	while(avgDriveEncoderVal()<abs(units)){
		setDriveVoltage(voltage*direction,voltage*direction);

		if (intake){ // might have to use/reset encoder vals if this doesn't work
			intakeSucc(intakeVolt);
		}
		if (outake){
			intakeSucc(-outakeVolt); // might have to reduce speed here
		}

		delay(10);
	}

		setDriveVoltage(-10*direction,-10*direction); //brief brake

		if (intake){ // brake intake as well if it is set to true
			intakeSucc(0);
		}
		if (outake){
			intakeSucc(0);
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
		frontLeftMotor.move(-voltage);
		frontRightMotor.move(-voltage);
		backRightMotor.move(-voltage);
		backLeftMotor.move(-voltage);
		delay (time);
		stopwheels();
}

void intakeOutake(int volt, int time){ //positive for intake, negative for outake
	intakeSucc(volt);
	delay(time);
	intakeSucc(0);
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

	const int intakeSpeed = 127;

	while (true) {
		controls = getControllerState();
//		setIntakeMotors();
//		setRollerMotors();
//		setBRollerMotor();


////////////////////////////////////////////////////////////driving////////////////////////////////////////////////////////////
		frontLeftMotor.move(-1*controls.left);
		backLeftMotor.move(-1*controls.left);
		frontRightMotor.move(controls.right); // for left motors, negative motor spin needs to be used to go forward w/ left positive motors
		backRightMotor.move(controls.right);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////__amd64
/*		intakeR.move(controls.leftBumper2);
		intakeL.move (-1*controls.leftBumper2);
		intakeR.move(-1*controls.rightBumper1);
		intakeL.move(controls.rightBumper1);
*/
/*		if (controls.a){ // for testing auto, comment out for competition
			delay(500);
			autonomous();
		  delay(500);
		}
*/
//////////////////////////////////////////////////intake and outtake///////////////////////////////////////////////////////
if (controls.leftBumper2){
	intakeSucc (-1*intakeSpeed); // succ out le balls
}
else	if (controls.leftBumper1){
	intakeSucc (intakeSpeed); // succ in le balls
}
	else {
	intakeSucc (0); // stop intake movement
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//		delay(tickLength);
	}
