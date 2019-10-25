/****************************************************************************
 *  Copyright (C) 2018 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/
/** @file shoot_task.c
 *  @version 1.1
 *  @date June 2017
 *
 *  @brief shoot bullet task
 *
 *  @copyright 2017 DJI RoboMaster. All rights reserved.
 *
 */

#include "shoot_task.h"
#include "gimbal_task.h"
#include "detect_task.h"
#include "comm_task.h"
#include "modeswitch_task.h"
#include "remote_ctrl.h"
#include "bsp_io.h"
#include "bsp_can.h"
#include "bsp_uart.h"
#include "bsp_io.h"
#include "keyboard.h"
#include "pid.h"
#include "sys_config.h"
#include "cmsis_os.h"
#include "string.h"

/* stack usage monitor */
UBaseType_t shoot_stack_surplus;

/* shoot task global parameter */
shoot_t   shoot;
trigger_t trig;

uint32_t shoot_time_last;
int shoot_time_ms;
float heat_max = 0.0f;
float heat_gun1 = 0.0f;
float gun_speed = 0.0f;
void shoot_task(void const *argu)
{
  osEvent event;
  
  while (1)
  {
    event = osSignalWait(SHOT_TASK_EXE_SIGNAL, osWaitForever);
    
    if (event.status == osEventSignal)
    {
      if (event.value.signals & SHOT_TASK_EXE_SIGNAL)
      {
        shoot_time_ms = HAL_GetTick() - shoot_time_last;
        shoot_time_last = HAL_GetTick();
        
        fric_wheel_ctrl();
        
        if (!shoot.fric_wheel_run)
        {
          shoot.shoot_cmd   = 0;
          shoot.c_shoot_cmd = 0;
        }

        trig.key = get_trigger_key_state();
					
        
        if (shoot.fric_wheel_run)
        {
				  shoot_bullet_handler();
        }
        else
        {
          pid_trigger_spd.out = 0;
        }
        
        trig.key_last = trig.key;
      }
    }
    
    shoot_stack_surplus = uxTaskGetStackHighWaterMark(NULL);
  }
}



void block_bullet_handler(void)
{
  uint32_t stall_count = 0;
  uint32_t stall_inv_count = 0;
  uint8_t  stall_f = 0;
  
  if (pid_trigger_spd.out <= -4000)
  {
    if (stall_f == 0)
      stall_count ++;
  }
  else
    stall_count = 0;
  
  if (stall_count >= 50)         //0.25s
  {
    stall_f = 1;
    stall_count = 0;
  }
  
  if (stall_f == 1)
  {
    stall_inv_count++;
    
    if (stall_inv_count >= 100)  //0.5s
    {
      stall_f = 0;
      stall_inv_count = 0;
    }
    else
      trig.spd_ref = 2000;
  }
}

static void fric_wheel_ctrl(void)
{
  if (shoot.fric_wheel_run)
  {
    turn_on_friction_wheel(shoot.fric_wheel_spd);
    turn_on_laser();
  }
  else
  {
    turn_off_friction_wheel();
    turn_off_laser();
  }
}



int debug_tri_spd = 0;
int shoot_cmd;
static void shoot_bullet_handler(void)
{
//  shoot_cmd = shoot.shoot_cmd;
//  if (shoot.shoot_cmd)
//  {
//    if (trig.one_sta == TRIG_INIT)
//    {
//      if (trig.key == 0)
//      {
//        trig.one_sta = TRIG_PRESS_DOWN;
//        trig.one_time = HAL_GetTick();
//      }
//    }
//    else if (trig.one_sta == TRIG_PRESS_DOWN)
//    {
//      if (HAL_GetTick() - trig.one_time >= 2000) //before the rising
//      {
//        trig.one_sta = TRIG_ONE_DONE;
//      }

//      if ((trig.key_last == 0) && (trig.key))    //Rising edge trigger button bounce
//      {
//        trig.one_sta = TRIG_BOUNCE_UP;
//        trig.one_time = HAL_GetTick();
//      }
//    }
//    else if (trig.one_sta == TRIG_BOUNCE_UP)
//    {
//      if (HAL_GetTick() - trig.one_time >= 2000)
//      {
//        trig.one_sta = TRIG_ONE_DONE;
//      }
//      
//      if ((trig.key_last) && (trig.key == 0))    //Falling edge trigger button be press
//      {
//        trig.one_sta = TRIG_ONE_DONE;
//      }
//    }
//    else
//    {
//    }
//    
//    if (trig.one_sta == TRIG_ONE_DONE)
//    {
//      trig.spd_ref = 0;
//      trig.one_sta = TRIG_INIT;
//      
//      shoot.shoot_cmd = 0;
//      shoot.shoot_bullets++;
//    }
//    else
//      trig.spd_ref = debug_tri_spd;//trig.feed_bullet_spd;
//    
//  }
//  else if (shoot.c_shoot_cmd)
//  {
//    trig.one_sta = TRIG_INIT;
//    trig.spd_ref = trig.c_shoot_spd;
//    
//    if ((trig.key_last == 0) && (trig.key == 1))
//      shoot.shoot_bullets++;
//    
//    //block_bullet_handler();
//  }
//  else
//  {
//    if (trig.key)       //not trigger
//      trig.spd_ref = debug_tri_spd;//trig.feed_bullet_spd;
//    else
//      trig.spd_ref = 0;
//  }
	
	heat_max = (float)judge_rece_mesg.robot_state.shooter_heat0_cooling_limit;
	heat_gun1 = (float)judge_rece_mesg.power_heat.shooter_heat0;
	gun_speed = (float)judge_rece_mesg.shoot_data.bullet_speed;
	
	if ((rc.mouse.l == 1 & (heat_max-heat_gun1) >= 30)||(rc.sw1 == RC_UP & (heat_max-heat_gun1) >= 30))
  {
    trig.one_sta = TRIG_INIT;
    trig.spd_ref = trig.c_shoot_spd;
    
//    if ((trig.key_last == 0) && (trig.key == 1))
//      shoot.shoot_bullets++;
    //block_bullet_handler();
  }
  else
  {
    if (trig.key)       //not trigger
      trig.spd_ref = debug_tri_spd;//trig.feed_bullet_spd;
    else
      trig.spd_ref = 0;
  }
  
  pid_calc(&pid_trigger_spd, moto_trigger.speed_rpm, trig.spd_ref);
}

void shoot_param_init(void)
{
  memset(&shoot, 0, sizeof(shoot_t));
  
  shoot.ctrl_mode      = SHOT_DISABLE;
  shoot.fric_wheel_spd = DEFAULT_FRIC_WHEEL_SPEED;
  //shoot.remain_bullets = 0;
  
  memset(&trig, 0, sizeof(trigger_t));
  
  trig.dir             = 0;
  trig.feed_bullet_spd = -1500;
  trig.c_shoot_spd      = -1500;
  trig.one_sta         = TRIG_INIT;
  
}
