#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <iostream>

namespace FreeRTOSpp {
/** @class Task
    @brief FreeRTOSのタスクのクラス．
*/
template <typename T> class Task {
public:
  Task() : pxCreatedTask(NULL) {}
  ~Task() { terminate(); }
  /** @function start
      @brief タスクを生成し，実行開始する関数
  */
  bool start(
      T *obj,                   //< thisポインタ
      void (T::*func)(),        //< メンバ関数ポインタ
      const char *const pcName, //< タスク名文字列
      unsigned short usStackDepth = configMINIMAL_STACK_SIZE, //< スタックサイズ
      unsigned portBASE_TYPE uxPriority = 0,    //< タスク優先度
      const BaseType_t xCoreID = tskNO_AFFINITY //< 実行コア
  ) {
    this->obj = obj;
    this->func = func;
    if (pxCreatedTask != NULL)
      terminate();
    BaseType_t result =
        xTaskCreatePinnedToCore(entry_point, pcName, usStackDepth, this,
                                uxPriority, &pxCreatedTask, xCoreID);
    return result == pdPASS;
  }
  bool start(
      TaskFunction_t pvTaskCode, //< メンバ関数ポインタ
      const char *const pcName,  //< タスク名文字列
      unsigned short usStackDepth = configMINIMAL_STACK_SIZE, //< スタックサイズ
      void *const pvParameters = NULL,
      unsigned portBASE_TYPE uxPriority = 0,    //< タスク優先度
      const BaseType_t xCoreID = tskNO_AFFINITY //< 実行コア
  ) {
    if (pxCreatedTask != NULL)
      terminate();
    BaseType_t result =
        xTaskCreatePinnedToCore(pvTaskCode, pcName, usStackDepth, pvParameters,
                                uxPriority, &pxCreatedTask, xCoreID);
    return result == pdPASS;
  }
  /** @function terminate
      @brief タスクを終了し，削除する関数
  */
  void terminate() {
    if (pxCreatedTask == NULL)
      return;
    vTaskDelete(pxCreatedTask);
    pxCreatedTask = NULL;
  }

private:
  TaskHandle_t pxCreatedTask = NULL; //< タスクのハンドル
  T *obj = NULL;                     //< thisポインタ
  void (T::*func)() = NULL;          //< メンバ関数ポインタ

  /** @function task
      @brief FreeRTOSにより実行される関数ポインタ
  */
  static void entry_point(void *arg) {
    auto task_obj = static_cast<Task *>(arg);
    (task_obj->obj->*task_obj->func)();
  }
};

/** @class Task
    @brief FreeRTOSのタスクのクラス．
*/
class CTask {
public:
  CTask() : pxCreatedTask(NULL) {}
  ~CTask() { terminate(); }
  /** @function start
      @brief タスクを生成し，実行開始する関数
  */
  bool start(
      TaskFunction_t pvTaskCode, //< メンバ関数ポインタ
      const char *const pcName,  //< タスク名文字列
      unsigned short usStackDepth = configMINIMAL_STACK_SIZE, //< スタックサイズ
      void *const pvParameters = NULL,
      unsigned portBASE_TYPE uxPriority = 0,    //< タスク優先度
      const BaseType_t xCoreID = tskNO_AFFINITY //< 実行コア
  ) {
    if (pxCreatedTask != NULL)
      terminate();
    BaseType_t result =
        xTaskCreatePinnedToCore(pvTaskCode, pcName, usStackDepth, pvParameters,
                                uxPriority, &pxCreatedTask, xCoreID);
    return result == pdPASS;
  }
  /** @function terminate
      @brief タスクを終了し，削除する関数
  */
  void terminate() {
    if (pxCreatedTask == NULL)
      return;
    vTaskDelete(pxCreatedTask);
    pxCreatedTask = NULL;
  }

private:
  TaskHandle_t pxCreatedTask = NULL; //< タスクのハンドル
};

/** @class TaskBase
    @brief FreeRTOSのタスクのベースとなるクラス．
    実行したい関数をもつクラスでこのクラスを継承して使用する．
*/
class TaskBase {
public:
  /** @function Constructor
      このクラス単体を宣言することはないだろう
  */
  TaskBase() : pxCreatedTask(NULL) {}
  /** @function Destructor
      もしタスクが実行中なら削除する
  */
  ~TaskBase() { deleteTask(); }
  /** @function createTask
      @brief タスクを生成する関数
  */
  bool createTask(const char *pcName, UBaseType_t uxPriority = 0,
                  const uint16_t usStackDepth = configMINIMAL_STACK_SIZE,
                  const BaseType_t xCoreID = tskNO_AFFINITY) {
    if (pxCreatedTask != NULL) {
      log_w("task %s is already created", pcName);
      return false;
    }
    // Taskを生成
    BaseType_t res =
        xTaskCreatePinnedToCore(pxTaskCode, pcName, usStackDepth, this,
                                uxPriority, &pxCreatedTask, xCoreID);
    if (res != pdPASS) {
      log_w("couldn't create the task %s", pcName);
      return false;
    }
    return true;
  }
  /** @function deleteTask
      @brief タスクを削除する関数
  */
  void deleteTask() {
    if (pxCreatedTask == NULL) {
      log_w("task is not created");
      return;
    }
    vTaskDelete(pxCreatedTask);
    pxCreatedTask = NULL;
  }

protected:
  TaskHandle_t pxCreatedTask; //< タスクのハンドル

  /** @function task
      @brief FreeRTOSにより実行される関数名
  */
  virtual void task() = 0;
  /** @function task
      @brief FreeRTOSにより実行される関数ポインタ
  */
  static void pxTaskCode(void *const pvParameters) {
    static_cast<TaskBase *>(pvParameters)->task();
  }
};

class Semaphore {
public:
  Semaphore() {
    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
      std::cerr << "xSemaphoreCreateBinary() failed" << std::endl;
    }
  }
  ~Semaphore() { vSemaphoreDelete(xSemaphore); }
  bool giveFromISR() {
    return pdTRUE == xSemaphoreGiveFromISR(xSemaphore, NULL);
  }
  bool give() { return pdTRUE == xSemaphoreGive(xSemaphore); }
  bool take(portTickType xBlockTime = portMAX_DELAY) {
    return pdTRUE == xSemaphoreTake(xSemaphore, xBlockTime);
  }

private:
  SemaphoreHandle_t xSemaphore = NULL;
};

class Mutex {
public:
  Mutex() {
    xSemaphore = xSemaphoreCreateMutex();
    if (xSemaphore == NULL) {
      std::cerr << "xSemaphoreCreateBinary() failed" << std::endl;
    }
  }
  ~Mutex() { vSemaphoreDelete(xSemaphore); }
  bool giveFromISR() {
    return pdTRUE == xSemaphoreGiveFromISR(xSemaphore, NULL);
  }
  bool give() { return pdTRUE == xSemaphoreGive(xSemaphore); }
  bool take(portTickType xBlockTime = portMAX_DELAY) {
    return pdTRUE == xSemaphoreTake(xSemaphore, xBlockTime);
  }

private:
  SemaphoreHandle_t xSemaphore = NULL;
};
} // namespace FreeRTOSpp