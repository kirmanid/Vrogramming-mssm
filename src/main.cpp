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
} controls;

Controller master (E_CONTROLLER_MASTER);

Motor frontLeftMotor(11);
Motor frontRightMotor(17);
Motor backLeftMotor(19);
Motor backRightMotor(18);
Motor leftGrabblerMotor(14);
Motor rightGrabblerMotor(16);
Motor retractRamp(15);

controllerState getControllerState (){
	controls.left = master.get_analog(ANALOG_LEFT_Y); //y val of left stick
	controls.right = master.get_analog(ANALOG_RIGHT_Y); // y val of right stick
	
	controls.rightBumper1 = master.get_digital(DIGITAL_R1);
	controls.leftBumper1 = master.get_digital(DIGITAL_L2);
	controls.rightBumper2 = master.get_digital(DIGITAL_R2);
	controls.leftBumper2 = master.get_digital(DIGITAL_L2);	

	return controls;
}

// spins grabblers, positive direction --> sucks up
void grabblerSucc (int grabblerRotation){
	leftGrabblerMotor.move(grabblerRotation);
	rightGrabblerMotor.move(-1 * grabblerRotation);
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
void competition_initialize() {}

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
void autonomous() {}



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
	const int grabblerSpeed = 100; // out of 127
	const int tickLength = 5; // in ms
	const int grabblerRamp = 40; // out of 127

	while (true) {
		controls = getControllerState();

		frontLeftMotor.move(controls.left);
		backLeftMotor.move(controls.left);

		frontRightMotor.move(-1 * controls.right); // for right motors, positive motor spin sends bot back
		backRightMotor.move(-1 * controls.right);

		if (controls.leftBumper1){
			grabblerSucc (grabblerSpeed); // succ in le blocks
		} else if (controls.leftBumper2){
			grabblerSucc (-1 * grabblerSpeed); // succ out le blocks
		}	else {
			grabblerSucc (0); // stop grabbler movement
		}

		//move le ramp (chimp)
		if (controls.rightBumper1 || controls.rightBumper2){
			if (controls.rightBumper2){ // move tower out
				retractRamp.move(-1 * grabblerSpeed);
			} else { // move tower in
				retractRamp.move(grabblerSpeed);
			}
		} else {
			retractRamp.move(0);
		}

		delay(tickLength);
	}
}
