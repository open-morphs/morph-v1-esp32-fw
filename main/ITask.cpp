#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ITask.hpp>
#include <Utils/ThreadSafeDbg.hpp>
#include "esp_log.h"
#include <InterruptLocker.hpp>

static const char* const TAG {"ITASK"};

ITask::ITask(const std::string& taskName, const uint16_t stackSize, const uint8_t priority, const BaseType_t coreID)
	:	_periodMs(0),
		_isRunning(false),
		_taskName(taskName),
		_stackSize(stackSize),
		_priority(priority),
		_coreID(coreID) {
	_taskParam = nullptr;
	_taskDescr = nullptr;
}

ITask::~ITask() {

}

void ITask::delay(const int ms) {

	vTaskDelay(ms/portTICK_PERIOD_MS);
}

void ITask::setPeriod(const uint32_t period) {
	_periodMs = period;
}

void ITask::runTask(void* pTaskInstance) {
	ITask* pTask = static_cast<ITask*> (pTaskInstance);
	pTask->run(pTask->_taskParam);
	pTask->stop();
}

void ITask::start(void* taskData) {

	if (_taskDescr != nullptr) {
		printf("Task::start - There might be a task with name: %s already running!\r\n", _taskName.c_str());
		delay(500);
		assert(false);
	}
	_taskParam = taskData;
	{
		/*
			Safe updating _isRunning flag, context switches are excluded for a while
		*/
		InterruptLock lock;
		assert(pdPASS == ::xTaskCreatePinnedToCore(&runTask, _taskName.c_str(), _stackSize, this, _priority, &_taskDescr, _coreID));
		_isRunning = true;
	}
}

void ITask::stop() {

	if (nullptr == _taskDescr) {
		return;
	}
	const auto currTask {xTaskGetCurrentTaskHandle()};
	if (_taskDescr != currTask) {
		vTaskSuspend(_taskDescr);
	}
	// UBaseType_t backPri {uxTaskPriorityGet(currTask)};
	// printf("currTask: %u, _task: %u, prio: %u\r\n", (size_t)currTask, (size_t)_taskDescr, backPri);
	// vTaskPrioritySet(currTask, 10);
	// {
	// 	InterruptLock lock;
	// 	const auto taskTemp {xTaskGetCurrentTaskHandle()};
	// 	if (taskTemp != _taskDescr) {
	// 		vTaskPrioritySet(taskTemp, configMAX_PRIORITIES - 1);
	// 	}
	// }
	onStop();
	xTaskHandle temp = _taskDescr;
	// {
		// InterruptLock lock;
		_taskDescr = nullptr;
		_isRunning = false;
		
	// }
	vTaskDelete(temp);
	// if (temp != currTask) {
	// 	vTaskPrioritySet(currTask, backPri);
	// }
}

void ITask::suspend() {
	if (_taskDescr == nullptr) {
		return;
	}
	if (false == _isRunning) {
		return;
	}
	{
		/*
			Safe updating _isRunning flag, context switches are excluded for a while
		*/
		InterruptLock lock;
		vTaskSuspend(_taskDescr);
		_isRunning = false;
	}
}

void ITask::resume() {
	if (_taskDescr == nullptr) {
		return;
	}
	if (true == _isRunning) {
		return;
	}
	{
		/*
			Safe updating _isRunning flag, context switches are excluded for a while
		*/
		InterruptLock lock;
		vTaskResume(_taskDescr);
		_isRunning = true;
	}
}

void ITask::onStop() {

}

void ITask::setStackSize(uint16_t stackSize) {
	_stackSize = stackSize;
}

void ITask::setPriority(uint8_t priority) {
	_priority = priority;
}

void ITask::setName(const std::string& name) {
	_taskName = name;
}

void ITask::setCore(BaseType_t coreId) {
	_coreID = coreId;
}

uint32_t ITask::getPeriod() const {
	return _periodMs; 
}

bool ITask::isRunning()const {
	return _isRunning;
}
