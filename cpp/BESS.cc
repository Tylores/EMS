#include "BESS.h"

BESS::BESS() :
    m_energy(0),
    m_importControl(0),
    m_exportControl(0),
    m_importWattHours(0),
    m_exportWattHours(0),
    m_importWatts(0),
    m_exportWatts(0),
    m_importChar(0),
    m_exportChar(0),
    m_idleChar(0),
    m_timescale(60)
{
    BESS::Random();
}; // END BESS

void BESS::Random(){
    m_energy = (rand() % 10000 + 1000)*(rand() % 10 + 1);
    m_importWattHours = 0.5*m_energy;
    m_exportWattHours = 0.5*m_energy;
    m_importWatts = (rand() % 10000 + 1000);
    m_exportWatts = (rand() % 10000 + 1000);
    m_importChar = m_importWatts/(rand() % 3600 + 60);
    m_exportChar = -m_exportWatts/(rand() % 3600 + 60);
    m_idleChar = m_exportChar/(rand() % 100 + 10);
}; // END RANDOM

void BESS::Charge(double t_watts){
    if (m_importWattHours >=0){
        m_importWattHours = m_importWattHours - m_importWatts/m_timescale;
        m_exportWattHours = m_exportWattHours + m_importWatts/m_timescale;
    } else {
        m_importWattHours = 0;
    }
}; // END CHARGE

void BESS::Discharge(double t_watts){
    if (m_exportWattHours > 0){
        m_importWattHours = m_importWattHours + m_exportWatts/m_timescale;
        m_exportWattHours = m_exportWattHours - m_exportWatts/m_timescale;
    } else {
        m_exportWattHours = 0;
    }
}; // END DISCHARGE

void BESS::Idle(){
    if (m_exportWattHours > 0){
        m_exportWattHours = m_exportWattHours - m_exportWatts/m_timescale;
    } else {
        m_exportWattHours = 0;
    }
}; // END IDLE

void BESS::Loop(){
    if (m_importControl > 0 && m_exportControl == 0){
        BESS::Charge(m_importControl);
    } else if (m_exportControl > 0 && m_importControl == 0){
        BESS::Discharge(m_exportControl);
    } else {
        BESS::Idle();
    }
}; // END LOOP


