#pragma once

#include "main.h" // HAVE TO BE FIRST since it includes STM32 HAL and CMSIS
#include "status.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "list.h"
#include "Timing.hpp"
#include "gpio.hpp"


/// @brief stmepic namespace
namespace stmepic {}