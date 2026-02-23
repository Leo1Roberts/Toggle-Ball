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

struct Smoother {
	float X;    // current value
	float D;    // destination value
	float DestV;// destination vel

	float V;// current speed
	float A;// acceleration

	float Elapsed;
	float Arrive;

	float S;// start value
	float U;// initial speed
	float I;// initial acceleration
	float R;// d3x/dx3
	float K;// d4x/dx4

	void Reset();

	void SetPosition(float x, float v);

	void SetDestination(float dest, float dest_v, float time);

	float GetCurrentValue() const { return X; }

	float GetDestination() const { return D; }

	void Update(float f_t_inc);
};

#endif// SMOOTHER_H