#ifndef BTS7960_h__
#define BTS7960_h__


#include "Motor.h"
#include "SerialOutput.h"
#include "Constants.h"

class BTS7960_1PWM : public Motor
{
protected:
	void(*callback_break)();
public:
	/*
		R_EN -|
			  |- PWM pin
		L_EN -|

		L_PWM -| digital pin
		R_PWM -| digital pin
	*/
	BTS7960_1PWM(PIN i_forward, PIN i_backward, PIN i_speed  /*PWM pin */, void(*i_callback_break)() = []() {}, uint8_t i_deadZone = 30)
		: m_L_PWM(i_forward), m_R_PWM(i_backward), m_ENABLE(i_speed) {
		callback_break = i_callback_break;
		m_deadZone = i_deadZone;
	}

	void begin()
	{
		pinMode(m_R_PWM, OUTPUT);
		pinMode(m_L_PWM, OUTPUT);
#ifdef ESP32
		m_channel = analogWriteChannel(m_ENABLE, 15000 , 8);
#else
		pinMode(m_ENABLE, OUTPUT);
#endif
		stop();
	}

	virtual void  forward(SPEED i_speed)
	{
		LOG_MSG(F("Speed ") << i_speed);

		setDirection(Direction::FORWARD);
		setSpeed(i_speed);
	}

	virtual void backward(SPEED i_speed)
	{
		LOG_MSG(F("Speed ") << i_speed);

		setDirection(Direction::BACKWARD);
		setSpeed(i_speed);
	}

	virtual void stop()
	{
		digitalWrite(m_L_PWM, LOW);
		digitalWrite(m_R_PWM, LOW);

		Motor::setDirection(Direction::STOPED);
		Motor::setSpeed(0);

		LOG_MSG(F("Speed ") << (int)getSpeed() << F(" Direction ") << (short)getDirection());
		callback_break();
	}

	void adjust ( SPEED i_speed ) {

		if ( i_speed > m_deadZone ) {
			analogWrite(m_ENABLE, i_speed );
		}
	}

protected:
	virtual void setSpeed( SPEED i_speed ) {

		SPEED current_speed = getSpeed();

		if ( i_speed < current_speed )
			callback_break();

		Motor::setSpeed( i_speed );

		if ( getSpeed() > m_deadZone ) {
			analogWrite(m_ENABLE, getSpeed() );
		}
		else {
			stop();
		}
	}

	virtual void setDirection(Direction i_direction) {

		Motor::setDirection(i_direction);

		switch (i_direction) {
		case Direction::FORWARD:
			digitalWrite(m_L_PWM, HIGH);
			digitalWrite(m_R_PWM, LOW);
			break;
		case Direction::BACKWARD:
			digitalWrite(m_L_PWM, LOW);
			digitalWrite(m_R_PWM, HIGH);
			break;
		case Direction::STOPED:
			digitalWrite(m_L_PWM, LOW);
			digitalWrite(m_R_PWM, LOW);
			break;
		}
	}

private:
	PIN m_R_PWM;  // backward
	PIN m_ENABLE; //speed
	PIN m_L_PWM;  // forward

#ifdef ESP32
	int16_t m_channel;
#endif

protected:
	int16_t m_deadZone;
};

template < class Base >
class BTS7960 : public Base
{
private:
	Motor::SPEED m_targetSpeed;
	Motor::Direction m_targetDirection;

	uint64_t tm = 0;

public:
	BTS7960(PIN i_forward, PIN i_backward, PIN i_speed  /*PWM pin */, void(*i_callback_break)(), uint8_t i_deadZone = 30) : Base(i_forward, i_backward, i_speed, i_callback_break) {
		m_targetSpeed = 0;
		tm = millis();
		Base::m_deadZone = i_deadZone;
		m_targetDirection = Motor::Direction::STOPED;
	}

	void run() {
		static uint8_t dt = 10;

		if (tm + dt < millis()) {
			if (Base::getDirection() != m_targetDirection) {
				Base::setSpeed(constrain(Base::getSpeed() - (Base::getSpeed() / dt), 0, Base::getSpeed()));
			}
			else if (abs(m_targetSpeed - Base::getSpeed()) > 10) {
				Base::setSpeed(Base::getSpeed() + (m_targetSpeed - Base::getSpeed()) / dt);
			}
			else {
				Base::setSpeed(m_targetSpeed);
			}

			if (Base::getDirection() == Motor::Direction::STOPED
				&& m_targetSpeed > Base::m_deadZone) {
				Base::setDirection(m_targetDirection);
			}

			tm = millis();
		}
	}

protected:
	virtual void setSpeed(Motor::SPEED i_speed) {
		LOG_MSG(F("Speed ") << i_speed);
		m_targetSpeed = constrain(i_speed, 0, 255);

		if (m_targetSpeed < Base::m_deadZone)
			Base::setDirection(Motor::Direction::STOPED);
	}

	virtual void setDirection(Motor::Direction i_direction) {
		m_targetDirection = i_direction;
	}
};


#endif // BTS7960_h__
