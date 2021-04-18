#ifndef Motor_h__
#define Motor_h__

class Motor
{
public:
	enum class Direction { FORWARD = 1, BACKWARD = 2, STOPED = 8 };
	typedef  uint8_t SPEED;

	virtual void forward( SPEED i_speed ) = 0;
	virtual void backward( SPEED i_speed ) = 0;
	virtual void adjust ( SPEED i_speed ) {}
	//alias
	virtual void right(SPEED i_speed) { forward(i_speed); }
	virtual void left(SPEED i_speed) { backward(i_speed); }

	virtual void stop() = 0;
	virtual void begin() = 0;
	virtual void run() {};

	const Direction&  getDirection() const { return m_direction; }
	const SPEED&      getSpeed() const { return m_speed; }
protected:

	virtual void setDirection(Direction i_direction) { m_direction = i_direction; }
	virtual void setSpeed(SPEED i_speed) { m_speed = constrain(i_speed, 0, 255); }

protected:

	Direction m_direction = Direction::STOPED;
	SPEED	  m_speed = 0;
};

#endif // Motor_h__