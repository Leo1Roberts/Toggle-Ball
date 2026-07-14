#ifndef SMOOTHER_H
#define SMOOTHER_H

// Smoother - move a value to a destination value smoothly in a specified time.
// Modifies its value by setting the following values:

// acceleration
// rate of change of acceleration
// rate of change of rate of change of acceleration

// SetPosition		(value, vel)		- set the start value and vel
// SetDestination	(value, vel, time)	- set the dest value / vel and the amount of time to get there
// Update			(float_time_inc)	- update function

class Smoother {
public:
	Smoother() = default;

	void reset();
	void update(float dt);

	void setPosition(float pos, float vel = 0);
	void setDestination(float destPos, float destVel, float time);

	[[nodiscard]] float getCurrentPosition() const { return x; }
	[[nodiscard]] float getCurrentVelocity() const { return v; }
	[[nodiscard]] float getDestination() const { return d; }

private:
	float x;     // current value
	float d;     // destination value
	float destV; // destination vel

	float v; // current speed
	float a; // acceleration

	float elapsed;
	float arrive;

	float s; // start value
	float u; // initial speed
	float i; // initial acceleration
	float r; // d3x/dx3
	float k; // d4x/dx4
};

#endif// SMOOTHER_H