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

//rotations = distance/circumference -- circumference = 12.5663706144 inches (maybe use 4.15 if this doesn't work)
Motor top(12);
Motor left(20);
Motor bottom(4);
Motor right(13);

Motor rDrivetrain1(11);
Motor rDrivetrain2(1);
Motor lDrivetrain1(7);
MOtor lDrivetrain2(15);


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

void intake (int force){ // negative force is outtake
    left.move(force);
    right.move(-1*force);
    b.move(-1*force);
}

void spinTopRoller (int force){ // negative force is backward spin, positive is forward
    top.move(force);
}

void score (int force){ // negative force is backward spin, positive is forward
    top.move(force);
    bottom.move(-1*force);
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
    lDrivetrain1=-1*voltL;
    lDrivetrain2=-1*voltL;
    rDrivetrain1=voltR;
    rDrivetrain2=voltR;
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

void translate (int units, int voltage, bool doingIntake, bool outtake){ //units is very small, 500 goes almost nowhere. Use negatives for backward, true if you want it to intake while it drives
	int direction = abs(units)/units;

	ResetDrive();

	while(avgDriveEncoderVal() < abs(units)){
		setDriveVoltage(voltage*direction,voltage*direction);

		if (doingIntake){ // might have to use/reset encoder vals if this doesn't work
			intake(127);
		}
		if (doingOuttake){
			intake(-127); // might have to reduce speed here
		}

		delay(10);
	}

	setDriveVoltage(-10*direction,-10*direction); //brief brake

	if (doingIntake || doingOuttake){ // brake intake as well if it is set to true
		intake(0);
	}

	delay(75);

	setDriveVoltage(0,0);
}

void stopwheels(){
    lDrivetrain1.move(0);
    lDrivetrain2.move(0);
    rDrivetrain1.move(0);
    rDrivetrain2.move(0);
}

void turnRightNoGyro(int voltage, int time){
    lDrivetrain1.move(voltage);
    lDrivetrain2.move(voltage);
    rDrivetrain1.move(-1*voltage);
    rDrivetrain2.move(-1*voltage);
    delay (time);
    stopwheels();
}

void turnLeftNoGyro(int voltage, int time){
    lDrivetrain1.move(-1*voltage);
    lDrivetrain2.move(-1*voltage);
    rDrivetrain1.move(voltage);
    rDrivetrain2.move(voltage);
    delay (time);
    stopwheels();
}

void intakeFor(int volt, int time){ //positive for intake, negative for outake
	intake(volt);
	delay(time);
	intake(0);
}

void blueRightCorner(){
    //void translate (int units, int voltage, bool doingIntake, bool doingOuttake){ //units is very small, 500 goes almost nowhere. Use negatives for backward, true if you want it to intake while it drives
    // chimp auto:
    top.move(127);
    delay(750);
    top.move(0);
     
    translate(2500, 50, true, false); // tweak units
    
    // outtake l and r 
    left.move(-127);
    right.move(127);
    
    translate(3750, 50, false, false); // tweak units
    left.move(0);
    right.move(0);
    score(127);
    delay(2000);
    top.move(0);
    bottom.move(0);
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
	const int intakeSpeed = 127; // out of 127
	const int tickLength = 5; // in ms

	while (true) {
		controls = getControllerState();
        
        lDrivetrain1.move(-1*controls.left);
        lDrivetrain2.move(-1*controls.left);
        rDrivetrain2.move(controls.right);
        rDrivetrain2.move(contols.right);

		if (controls.a){ // for testing auto, comment out for competition
			delay(500);
			autonomous();
		  delay(500);
		}
        
        
        if (controls.rightBumper1 || contols.rightBumper2){
            top.move(contols.rightBumper1? intakeSpeed : -1*intakeSpeed);
        } else  if(){
            top.move(intakeSpeed);
            bottom.move(-1*intakeSpeed);
        } else {
            top.move(0);
            bottom.move(0);
        }
        
        if (controls.leftBumper1 || controls.leftBumper2){
            intake(controls.leftBumper1? intakeSpeed : -1 * intakeSpeed);
        } else {
            intake(0);
        }
        
		delay(tickLength);
	}
}
