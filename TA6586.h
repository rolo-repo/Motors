#ifndef TA6586_h__
#define TA6586_h__

#include "Motor.h"
#include "Constants.h"
#include "SerialOutput.h"

class TA6586 : public Motor
{
protected:
	void(*callback_break)();
public:
	TA6586(PIN i_forward, PIN i_backward, void(*i_callback_break)() = []() {}, uint8_t i_deadZone = 30)
		: m_L_PWM(i_forward), m_R_PWM(i_backward) {
		callback_break = i_callback_break;
		m_deadZone = i_deadZone;
	}

	void begin()
	{
		pinMode(m_R_PWM, OUTPUT);
		pinMode(m_L_PWM, OUTPUT);

#ifdef ESP32
		m_channelR = analogWriteChannel(m_R_PWM, 50, 8);
		m_channelL = analogWriteChannel(m_L_PWM, 50, 8);
#endif
		stop();
	}

	virtual void  forward(SPEED i_speed)
	{
		LOG_MSG(F("Speed ") << i_speed);
		setSpeed(i_speed);

		if ( m_speed > m_deadZone ) {
			setDirection(Direction::FORWARD);

			digitalWrite(m_R_PWM, LOW);
#ifdef ESP32
			analogWrite(m_R_PWM, 0);
#endif
			analogWrite(m_L_PWM, m_speed);
		}
		else {
			stop();
		}
	}

	virtual void backward(SPEED i_speed)
	{
		LOG_MSG(F("Speed ") << i_speed);

		setSpeed(i_speed);
		if ( m_speed > m_deadZone ) {
			setDirection(Direction::BACKWARD);
			digitalWrite(m_L_PWM, LOW);
#ifdef ESP32
			analogWrite(m_L_PWM, 0);
#endif
			analogWrite(m_R_PWM, m_speed);
		}
		else {
			stop();
		}

	}

	virtual void stop()
	{
		Motor::setDirection(Direction::STOPED);
		Motor::setSpeed(0);
#ifdef ESP32
		analogWrite( m_R_PWM, 0 );
		analogWrite( m_L_PWM, 0 );
#endif

		digitalWrite(m_L_PWM, LOW);
		digitalWrite(m_R_PWM, LOW);

		callback_break();

		LOG_MSG(F("Speed ") << (int)getSpeed()
			<< F(" Direction ") << (short)getDirection());

	}

	void adjust( SPEED i_speed ) {

		i_speed = constrain(i_speed, 0, 255);

		switch ( getDirection() )
		{
		case Direction::FORWARD:
			analogWrite(m_L_PWM, i_speed);
			break;
		case Direction::BACKWARD:
			analogWrite(m_R_PWM, i_speed);
			break;
		default:
			stop();
		}
	}


private:
	PIN m_R_PWM;  // backward
	PIN m_L_PWM;  // forward

#ifdef ESP32
	int16_t m_channelR;
	int16_t m_channelL;
#endif

protected:
	int16_t m_deadZone;
};

#endif // TA6586_h__
