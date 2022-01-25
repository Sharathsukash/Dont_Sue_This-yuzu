// Copyright 2022 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/assert.h"
#include "core/hle/kernel/k_process.h"
#include "core/hle/kernel/k_thread.h"
#include "core/hle/kernel/k_worker_task.h"
#include "core/hle/kernel/k_worker_task_manager.h"
#include "core/hle/kernel/kernel.h"

namespace Kernel {

KWorkerTask::KWorkerTask(KernelCore& kernel_) : KSynchronizationObject{kernel_} {}

void KWorkerTask::DoWorkerTask() {
    if (auto* const thread = this->DynamicCast<KThread*>(); thread != nullptr) {
        return thread->DoWorkerTaskImpl();
    } else {
        auto* const process = this->DynamicCast<KProcess*>();
        ASSERT(process != nullptr);

        return process->DoWorkerTaskImpl();
    }
}

KWorkerTaskManager::KWorkerTaskManager() : m_waiting_thread(1, "yuzu:KWorkerTaskManager") {}

void KWorkerTaskManager::AddTask(KernelCore& kernel, WorkerType type, KWorkerTask* task) {
    ASSERT(type <= WorkerType::Count);
    kernel.WorkerTaskManager().AddTask(kernel, task);
}

void KWorkerTaskManager::AddTask(KernelCore& kernel, KWorkerTask* task) {
    KScopedSchedulerLock sl(kernel);
    m_waiting_thread.QueueWork([task]() {
        // Do the task.
        task->DoWorkerTask();
    });
}

} // namespace Kernel