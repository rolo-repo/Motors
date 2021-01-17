#ifndef BTS7960_h__
#define BTS7960_h__

#include <Constants.h>
#include "Motor.h"

#include <SerialOutput.h>

class BTS7960_1PWM : public Motor
{
protected :
	void(*callback_break)();
public:
	/*
		R_EN -|
			  |- PWM pin
		L_EN -|

		L_PWM -| digital pin
		R_PWM -| digital pin
	*/
	BTS7960_1PWM( PIN i_forward, PIN i_backward, PIN i_speed  /*PWM pin */, void(*i_callback_break)() = []() {}  ) : m_L_PWM(i_forward), m_R_PWM(i_backward), m_ENABLE(i_speed)
	{
		m_direction = Direction::STOPED;
		m_speed = 0;
		callback_break = i_callback_break;
	}

	void begin()
	{
		pinMode(m_R_PWM, OUTPUT);
		pinMode(m_L_PWM, OUTPUT);
		pinMode(m_ENABLE, OUTPUT);
		stop();
	}

	virtual void  forward( SPEED i_speed )
	{
		LOG_MSG( F("Speed ") << i_speed );

		setDirection( Direction::FORWARD );
		setSpeed( i_speed );
	}

	virtual void backward( SPEED i_speed )
	{
		LOG_MSG(F("Speed ") << i_speed);

		setDirection( Direction::BACKWARD );
		setSpeed( i_speed );
	}

	virtual void stop()
	{
		LOG_MSG(F("Speed ") << m_speed);

		setDirection( Direction::STOPED ) ;
		setSpeed(0);
		callback_break();
	}

protected:
	 virtual void setSpeed ( SPEED i_speed ){
		
		 if ( i_speed < m_speed )
			 callback_break();

		m_speed = constrain( i_speed, 0, 255 );
		
		if ( m_speed > m_deadZone ) {
			analogWrite( m_ENABLE, m_speed);
		}
		else {
			digitalWrite(m_L_PWM, LOW);
			digitalWrite(m_R_PWM, LOW);
			m_direction =  Direction::STOPED;
		}
	}

	 virtual void setDirection ( Direction i_direction ){
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

		m_direction = i_direction;
	}

protected:
	
	PIN m_R_PWM;  // backward
	PIN m_ENABLE; //speed
	PIN m_L_PWM;  // forward

	int16_t m_deadZone = 30;
};

class BTS7960_2PWM : public Motor
{
	/*
		R_EN -|
			  |- 5V pin
		L_EN -|

		L_PWM -| PWM pin
		R_PWM -| PWM pin
	*/
public:

	BTS7960_2PWM(PIN i_forward /*PWM*/ ,PIN i_backward /*PWM*/ ):m_L_PWM(i_forward), m_R_PWM(i_backward){}

	void begin()
	{
		pinMode(m_R_PWM, OUTPUT);
		pinMode(m_L_PWM, OUTPUT);
		stop();
	}
	virtual void  forward(SPEED i_speed)
	{
		LOG_MSG(F("Motor -- move forward speed ") << i_speed);

		if (i_speed < m_deadZone)
			return stop();

		analogWrite(m_L_PWM, constrain(i_speed, 0, 255));
		analogWrite(m_R_PWM, 0);

		m_direction = Direction::FORWARD;
	}

	virtual void backward( SPEED i_speed )
	{
		LOG_MSG(F("Motor -- move backward speed ") << i_speed);

		if (i_speed < m_deadZone)
			return stop();

		analogWrite( m_R_PWM, constrain( i_speed, 0, 255 ) );
		analogWrite( m_L_PWM, 0 );

		m_direction = Direction::BACKWARD;
	}

	virtual void stop()
	{
		analogWrite( m_L_PWM, 0 );
		analogWrite( m_R_PWM, 0 );
		m_direction = Direction::STOPED;
	}

private :
	PIN m_R_PWM;  // backward
	PIN m_L_PWM;  // forward

	int16_t m_deadZone = 30;
};

class BTS7960_1PWM_Smooth : public BTS7960_1PWM
{
private:
	SPEED m_targetSpeed;
	Direction m_targetDirection;

	uint64_t tm = 0;

public :
	BTS7960_1PWM_Smooth( PIN i_forward, PIN i_backward, PIN i_speed  /*PWM pin */, void(*i_callback_break)() , int32_t i_deadZone = 30 ) : BTS7960_1PWM ( i_forward, i_backward, i_speed, i_callback_break ) 
	{
		m_targetSpeed = 0;
		tm = millis();
		m_deadZone = i_deadZone;
		m_targetDirection = Motor::Direction::STOPED;
	}


	void run()
	{
		uint16_t dt = 10 ;
		
		if ( tm + dt < millis() ) {	
			if ( getDirection() != m_targetDirection ){
				BTS7960_1PWM::setSpeed( getSpeed() - ( getSpeed() / 10 ) );
			} else if ( abs( m_targetSpeed - getSpeed() ) > 10 ) {
				BTS7960_1PWM::setSpeed( getSpeed() + ( m_targetSpeed - getSpeed() ) / 10 );
			} else  {
				BTS7960_1PWM::setSpeed( m_targetSpeed );
			}

			if ( BTS7960_1PWM::getDirection() == Direction::STOPED 
				&& m_targetSpeed > m_deadZone ) {
				BTS7960_1PWM::setDirection( m_targetDirection );
			}

			tm = millis();

		}	
	}

protected:
    virtual void setSpeed( SPEED i_speed ) {
		LOG_MSG(F("Speed ") << i_speed);
		m_targetSpeed = constrain ( i_speed , 0 ,255 );

		if ( m_targetSpeed < m_deadZone )
			BTS7960_1PWM::setDirection( Direction::STOPED );
	}

	virtual void setDirection( Direction i_direction ) {	
		m_targetDirection = i_direction;
	}

};


#endif // BTS7960_h__
