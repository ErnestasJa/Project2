#ifndef THEPROJECTMAIN_BACKGROUNDJOBRUNNER_H
#define THEPROJECTMAIN_BACKGROUNDJOBRUNNER_H

#include "BackgroundJob.h"
#include <mutex>
#include <thread>

namespace threading {

template <int ConcurrencyLevel = 4> class BackgroundJobRunner
{
  public:
  BackgroundJobRunner()
  {
    m_killAllThreads = false;
    for (int i = 0; i < ConcurrencyLevel; i++)
    {
      m_threads[i] = std::thread(&ThreadRunner, this);
    }
  }

  ~BackgroundJobRunner()
  {
    JoinAllRunners();
  }

  void EnqueueBackgroundJob(BackgroundJob* backgroundJob)
  {
    m_jobQueueMutex.lock();
    m_backgroundJobQueue.push(core::UniquePtr<BackgroundJob>(backgroundJob));
    m_jobQueueMutex.unlock();
  }

  void Run()
  {
    if (m_mainThreadFinalizationQueue.empty() == false)
    {
      core::UniquePtr<BackgroundJob> backgroundJob = nullptr;

      m_mainThreadFinalizationQueueMutex.lock();
      backgroundJob = core::Move(m_mainThreadFinalizationQueue.front());
      m_mainThreadFinalizationQueue.pop();
      m_mainThreadFinalizationQueueMutex.unlock();

      ASSERT(backgroundJob);
      backgroundJob->FinalizeInMainThread();
    }
  }

  private:
  static void ThreadRunner(BackgroundJobRunner* runner)
  {
    while (runner->m_killAllThreads == false)
    {
      core::UniquePtr<BackgroundJob> backgroundJobToRun = nullptr;

      if (runner->m_jobQueueMutex.try_lock())
      {
        if (runner->m_backgroundJobQueue.empty() == false)
        {
          backgroundJobToRun = core::Move(runner->m_backgroundJobQueue.front());
          runner->m_backgroundJobQueue.pop();
        }
        runner->m_jobQueueMutex.unlock();
      }

      if (backgroundJobToRun)
      {
        backgroundJobToRun->Run();

        runner->m_mainThreadFinalizationQueueMutex.lock();
        runner->m_mainThreadFinalizationQueue.push(core::Move(backgroundJobToRun));
        runner->m_mainThreadFinalizationQueueMutex.unlock();
      }
      else
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }

  void JoinAllRunners()
  {
    m_killAllThreads = true;
    for (int i = 0; i < ConcurrencyLevel; i++)
    {
      elog::LogInfo(core::string::format("Joining thread <{}>", i));
      m_threads[i].join();
      elog::LogInfo(core::string::format("Joined thread <{}>", i));
    }
  }

  void UpdateJobs() {}

  private:
  bool                                        m_killAllThreads;
  core::Array<std::thread, ConcurrencyLevel>  m_threads;
  std::mutex                                  m_jobQueueMutex;
  std::mutex                                  m_mainThreadFinalizationQueueMutex;
  core::Queue<core::UniquePtr<BackgroundJob>> m_backgroundJobQueue;
  core::Queue<core::UniquePtr<BackgroundJob>> m_mainThreadFinalizationQueue;
};

} // namespace threading
#endif // THEPROJECTMAIN_BACKGROUNDJOBRUNNER_H
