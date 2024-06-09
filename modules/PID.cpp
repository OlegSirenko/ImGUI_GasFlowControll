#include "PID.h"
#include <iostream>
#include <cmath>

using namespace std;

PID::PID() {}

PID::~PID() {}

///=============================================================================
///  @brief  PID::Init()
///          Initialize the PID Controller.
///          
///  @param  kp: Proportional control value 
///  @param  ki: Integral control value
///  @param  kd: Derivative control value
///
///  @retval: void
///=============================================================================
void PID::Init( double Kp, double Ki, double Kd ) {

  this->Kp = Kp;
  this->Ki = Ki;
  this->Kd = Kd;

  this->p_error = Kp;
  this->i_error = Ki;
  this->d_error = Kd;

  this->tune_state = INCREASE_P;
  this->next_state = INCREASE_P;
  this->K_index = 0;
  this->next_index = 0;
  this->sample_index = 0;

  cout << __func__ << " Kp: "<<Kp << " Ki: "<<Ki <<" Kd: "<<Kd << endl;
}

///=============================================================================
///  @brief  PID::UpdateError()
///          Update the PID error variables
///          
///  @param  cte: Cross Track Error
///
///  @retval: void
///=============================================================================
void PID::UpdateError( double cte ) {

  double prev_cte = this->p_error;

  double under_windup_limit = ( this->i_error + cte ) < Ki_WINDUP_LIMIT;

  this->p_error = cte;
  this->i_error = ( under_windup_limit ) ? this->i_error+cte : 0;
  this->d_error = cte - prev_cte;
}

///=============================================================================
///  @brief  PID::AutoTuneController()
///          Update the PID error variables
///          
///  @param  cte: Cross Track Error
///
///  @retval: void
///=============================================================================
void PID::AutoTuneController( double cte ) {
  this->p = { this->Kp, this->Ki, this->Kd };
  this->dp= { this->p_error, this->i_error, this->d_error };
  cout << __func__ <<":"<< __LINE__ << " p,i,d errors:" << this->p_error << " "<< this->i_error  << " " << this->d_error << endl;
  cout << __func__ <<":"<< __LINE__ << " P values " << this->p[0] << " "<< this->p[1]  << " " << this->p[2] << endl;
  switch ( this->tune_state ) 
  {
    case INCREASE_P:
      this->p[ this->K_index ] += this->dp[ this->K_index ];
      this->next_state = RESAMPLE_P;
      cout << __LINE__ << " K_index, p[kp]: " << this->K_index << " " << this->p[K_index] << endl;
      cout << __LINE__ << " States (current, next): " << this->tune_state << " " << this->next_state << endl;
      break;
    case RESAMPLE_P:
      if( fabs( cte ) < fabs( this->best_error ) ) {
        this->best_error = cte;
        this->dp[ this->K_index ] *= 1.1;
        this->next_state = INCREASE_P;
        this->next_index = ( this->next_index + 1 ) % 2;
      }
      else {
        this->next_state = DECREASE_2P;
      }
      cout << __LINE__ << " K_index, p[K_index]: " << this->K_index << " " << this->p[K_index] << endl;
      cout << __LINE__ << " States (current, next): " << this->tune_state << " " << this->next_state << endl;
      break;
    case DECREASE_2P:
      this->p[ this->K_index ] -= 2 * this->dp[ this->K_index ];
      this->next_state = RESAMPLE_2P;
      cout << __LINE__ << " K_index, p[K_index]: " << this->K_index << " " << this->p[K_index] << endl;
      cout << __LINE__ << " States (current, next): " << this->tune_state << " " << this->next_state << endl;
      break;
    case RESAMPLE_2P:
      if( fabs( cte ) < fabs( this->best_error ) ) {
        this->best_error = cte;
        this->dp[ this->K_index ] *= 1.1;
      }
      else {
        this->p[ this->K_index ] += this->dp[ this->K_index ];
        this->dp[ this->K_index ] *= 0.9;
      }
      this->next_state = INCREASE_P;
      this->next_index = ( this->next_index + 1 ) % 2;
      cout << __LINE__ << " K_index, p[K_index]: " << this->K_index << " " << this->p[K_index] << endl;
      cout << __LINE__ << " States (current, next): " << this->tune_state << " " << this->next_state << endl;
      break;
    default:
      cout << "Unknown State: " << this->tune_state << endl;
      this->next_state = INCREASE_P;
      break;
  }
  this->p_error = this->dp[0];
  this->i_error = this->dp[1];
  this->d_error = this->dp[2];
  this->Kp = this->p[0];
  this->Ki = this->p[1];
  this->Kd = this->p[2];
  cout << __func__ <<":"<< __LINE__ << " p,i,d errors:" << this->p_error << " "<< this->i_error  << " " << this->d_error << endl;
  cout << __func__ <<":"<< __LINE__ << " P values " << this->p[0] << " "<< this->p[1]  << " " << this->p[2] << endl;
  this->tune_state = this->next_state;
  this->K_index = this->next_index;
}

///=============================================================================
///  @brief  PID::GetSteerValue()
///          Get the PID error aka steering angle value 
///          
///  @param  None
///
///  @retval: Steer Value 
///=============================================================================
double PID::GetSteerValue() {
    double steer_value, value;
    value = -Kp * this->p_error - Kd * this->d_error - Ki * this->i_error;

    if( value > 1 ) {
        steer_value = 1;
    }
    else if ( value < -1 ) {
        steer_value = -1;
    }
    else {
        steer_value = value;
    }
    return steer_value;
}
