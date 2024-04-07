#ifndef THEPROJECTMAIN_BACKGROUNDJOB_H
#define THEPROJECTMAIN_BACKGROUNDJOB_H
namespace threading {
class BackgroundJob
{
  public:
  virtual void Run() = 0;
  /// Override this to execute last steps in the main thread.
  virtual void FinalizeInMainThread() = 0;
  virtual ~BackgroundJob()            = default;
};

} // namespace threading
#endif // THEPROJECTMAIN_BACKGROUNDJOB_H
