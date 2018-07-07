#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

namespace FreeRTOSpp {

/**
 * @brief C++ のメンバ関数を実行することができるタスクのクラス
 * 
 * @tparam T 実行するメンバ関数のクラス
 */
template <typename T> class Task {
public:
  /**
   * @brief Construct a new Task object
   */
  Task() : pxCreatedTask(NULL) {}
  /**
   * @brief Destroy the Task object
   */
  ~Task() { terminate(); }
  /**
   * @brief タスクを生成し，実行開始する関数
   *
   * @oaram obj this ポインタ
   * @param func メンバ関数ポインタ，`&T::func` のように渡す
   * @param pcName タスク名文字列
   * @param usStackDepth スタックサイズ
   * @param uxPriority 優先度
   * @param xCoreID 実行させるCPUコア番号
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
    if (pxCreatedTask != NULL) {
      ESP_LOGW(tag, "task %s is already created", pcName);
      return false;
    }
    BaseType_t result =
        xTaskCreatePinnedToCore(entry_point, pcName, usStackDepth, this,
                                uxPriority, &pxCreatedTask, xCoreID);
    return result == pdPASS;
  }
  /**
   * @brief タスクを終了し，削除する関数
   */
  void terminate() {
    if (pxCreatedTask == NULL)
      return;
    vTaskDelete(pxCreatedTask);
    pxCreatedTask = NULL;
  }

private:
  const char *tag = "Task";
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

/** @class TaskBase
    @brief FreeRTOSのタスクのベースとなるクラス．
    実行したい関数をもつクラスは，このクラスを継承する．
*/
class TaskBase {
public:
  /**
   * @brief Construct a new Task Base object
   * このコンストラクタを呼ぶことはない
   */
  TaskBase() : pxCreatedTask(NULL) {}
  /**
   * @brief Destroy the Task Base object
   * もしタスクが実行中なら削除する
   */
  ~TaskBase() { deleteTask(); }
  /**
   * @brief Create a Task object
   *
   * @param pcName
   * @param uxPriority
   * @param usStackDepth
   * @param xCoreID
   * @return true
   * @return false
   */
  bool createTask(const char *pcName, UBaseType_t uxPriority = 0,
                  const uint16_t usStackDepth = configMINIMAL_STACK_SIZE,
                  const BaseType_t xCoreID = tskNO_AFFINITY) {
    if (pxCreatedTask != NULL) {
      ESP_LOGW(tag, "task \"%s\" is already created", pcName);
      return false;
    }
    BaseType_t res =
        xTaskCreatePinnedToCore(pxTaskCode, pcName, usStackDepth, this,
                                uxPriority, &pxCreatedTask, xCoreID);
    if (res != pdPASS) {
      ESP_LOGW(tag, "couldn't create the task \"%s\"", pcName);
      return false;
    }
    return true;
  }
  /**
   * @brief タスクを削除する関数
   */
  void deleteTask() {
    if (pxCreatedTask == NULL) {
      ESP_LOGW(tag, "task is not created");
      return;
    }
    vTaskDelete(pxCreatedTask);
    pxCreatedTask = NULL;
  }

protected:
  const char *tag = "TaskBase";
  TaskHandle_t pxCreatedTask; //< タスクのハンドル

  /**
   * @brief FreeRTOS
   * により実行される関数の宣言．実体は継承クラスで定義すること．
   */
  virtual void task() = 0;
  /**
   * @brief FreeRTOS により実行される静的関数ポインタ
   * @param pvParameters this ポインタ
   */
  static void pxTaskCode(void *const pvParameters) {
    static_cast<TaskBase *>(pvParameters)->task();
  }
};

/**
 * @brief C++ Wrapper for Semaphore function
 */
class Semaphore {
public:
  Semaphore() {
    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
      ESP_LOGE(tag, "xSemaphoreCreateBinary() failed");
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
  const char *tag = "Semaphore";
  SemaphoreHandle_t xSemaphore = NULL;
};

/**
 * @brief C++ Wrapper for Mutex function
 */
class Mutex {
public:
  Mutex() {
    xSemaphore = xSemaphoreCreateMutex();
    if (xSemaphore == NULL) {
      ESP_LOGE(tag, "xSemaphoreCreateMutex() failed");
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
  const char *tag = "Mutex";
  SemaphoreHandle_t xSemaphore = NULL;
};

} // namespace FreeRTOSpp