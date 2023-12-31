/*******************************************************************************
* Copyright © 2016 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2023 Analog Devices Inc. All Rights Reserved. This software is
* proprietary & confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/**
  This file contains functions for using the encoder interface of the TMC5160.
*/

#include <limits.h>
#include <stdlib.h>
#include <math.h>
#if defined(MK20DX128)
  #include "derivative.h"
#elif defined(GD32F425)
  #include "gd32f4xx.h"
#endif
#include "bits.h"
#include "stealthRocker.h"
#include "TMC5160.h"
#include "Globals.h"


/*********************************************//**
  \fn InitEncoder(void)
  \brief Initialize the encoder interface

  This function initalizes the encoder interface
  of the TMC5160.
*************************************************/
void InitEncoder(void)
{
  UCHAR i;

  for(i=0; i<N_O_MOTORS; i++) WriteTMC5160Int(WHICH_5160(i), TMC5160_ENC_CONST, 65536);  //1.0
}


/**********************************************************//**
  \fn CalculateEncoderParameters(UCHAR Axis)
  \brief Calculate the encoder multiplier
  \param Axis   Axis number (always 0 with stealthRocker)

  This calculates the encoder multiplier of the TMC5160 in
  such a way that the encoder resolution matches the motor
  microstep resolution.
**************************************************************/
void CalculateEncoderParameters(UCHAR Axis)
{
  int MotorSteps;
  double EncFactor;
  double EncFactorFp;
  double EncFactorIp;
  int EncPrescaler;

  if(MotorConfig[Axis].EncoderResolution==0 || MotorConfig[Axis].MotorFullStepResolution==0)
  {
    //Select 1:1 if encoder resolution or motor resolution parameter should be zero.
    WriteTMC5160Int(WHICH_5160(Axis), TMC5160_ENCMODE, ReadTMC5160Int(WHICH_5160(Axis), TMC5160_ENCMODE) & ~TMC5160_EM_DECIMAL);
    WriteTMC5160Int(WHICH_5160(Axis), TMC5160_ENC_CONST, 65536);
    return;
  }

  MotorSteps = (1<<(8-GetTMC5160ChopperMStepRes(WHICH_5160(Axis))))*MotorConfig[Axis].MotorFullStepResolution;
  EncFactor=(double) MotorSteps / (double) abs(MotorConfig[Axis].EncoderResolution);

  if(modf(EncFactor*65536.0, &EncFactorIp)==0.0)
  {
    EncPrescaler=(int) (EncFactor*65536.0);
    if(MotorConfig[Axis].EncoderResolution<0) EncPrescaler=-EncPrescaler;
    WriteTMC5160Int(WHICH_5160(Axis), TMC5160_ENCMODE, ReadTMC5160Int(WHICH_5160(Axis), TMC5160_ENCMODE) & ~TMC5160_EM_DECIMAL);
    WriteTMC5160Int(WHICH_5160(Axis), TMC5160_ENC_CONST, EncPrescaler);
  }
  else
  {
    EncFactorFp=modf(EncFactor, &EncFactorIp);
    if(MotorConfig[Axis].EncoderResolution>0)
    {
      EncPrescaler=((int) EncFactorIp) << 16;
      EncPrescaler|=(int) (EncFactorFp*10000.0);
    }
    else
    {
      EncPrescaler=(-((int) EncFactorIp)-1) << 16;
      EncPrescaler|=10000-((int) (EncFactorFp*10000.0));
    }
    WriteTMC5160Int(WHICH_5160(Axis), TMC5160_ENCMODE, ReadTMC5160Int(WHICH_5160(Axis), TMC5160_ENCMODE)|TMC5160_EM_DECIMAL);
    WriteTMC5160Int(WHICH_5160(Axis), TMC5160_ENC_CONST, EncPrescaler);
  }
}

/*********************************************************//**
  \fn GetEncoderPosition(UCHAR Axis)
  \brief Read encoder position
  \param Axis  Motor number (always 0 with stealthRocker)
  \return Encoder position

  This function returns the value of the encoder
  position register in the TMC5160.
*************************************************************/
int GetEncoderPosition(UCHAR Axis)
{
  return ReadTMC5160Int(WHICH_5160(Axis), TMC5160_XENC);
}

/*********************************************************//**
  \fn SetEncoderPosition(UCHAR Axis, int Value)
  \brief Change encoder position
  \param Axis  Motor number (always 0 with stealthRocker)
  \param Value  New encoder position value

  This function position register in the TMC5160 to a new
  value.
*************************************************************/
void SetEncoderPosition(UCHAR Axis, int Value)
{
  WriteTMC5160Int(WHICH_5160(Axis), TMC5160_XENC, Value);
}
