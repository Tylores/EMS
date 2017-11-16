#ifndef BESS_INC
#define BESS_INC

#include <iostream>
#include <stdlib.h>

using namespace std;

class BESS 
{
protected:
	double m_energy, m_importControl, m_exportControl, m_importWattHours, m_exportWattHours,
	m_importWatts, m_exportWatts, m_importChar, m_exportChar, m_idleChar, m_timescale;

public:
	BESS();
	void Charge(double t_watts);
	void Discharge(double t_watts);
	void Idle();
	void Random();
	void Loop();
};
#endif //BESS_INC

