#ifndef CONNECT_SERVICES__GRID_WORKER_IMPL__HPP
#define CONNECT_SERVICES__GRID_WORKER_IMPL__HPP

/*  $Id: grid_worker_impl.hpp 637653 2021-09-15 19:05:18Z sadyrovr $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *   Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Dmitry Kazimirov
 *
 * File Description:
 *    Common NetSchedule Worker Node declarations
 */


#include "wn_commit_thread.hpp"
#include "wn_cleanup.hpp"
#include "netschedule_api_impl.hpp"

#include <connect/services/grid_rw_impl.hpp>

#include <atomic>
#include <unordered_map>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
//

///@internal
struct SWorkerNodeJobContextImpl : public CObject
{
    SWorkerNodeJobContextImpl(SGridWorkerNodeImpl* worker_node);

    void ResetJobContext();

    void MarkJobAsLost()
    {
        m_JobCommitStatus = CWorkerNodeJobContext::eCS_JobIsLost;
    }
    void CheckIfJobIsLost();

    void x_PrintRequestStop();

    virtual void PutProgressMessage(const string& msg,
        bool send_immediately, bool overwrite);
    virtual CNetScheduleAdmin::EShutdownLevel GetShutdownLevel();
    virtual void JobDelayExpiration(unsigned runtime_inc);
    virtual void x_RunJob();

    const CDeadline GetTimeout() const { return m_Deadline; }
    void ResetTimeout(unsigned seconds) { m_Deadline = CDeadline(seconds, 0); }

    CNcbiIstream& GetIStream();
    CNcbiOstream& GetOStream();

    SGridWorkerNodeImpl* m_WorkerNode;
    CNetScheduleJob m_Job;
    CWorkerNodeJobContext::ECommitStatus m_JobCommitStatus;
    bool m_DisableRetries;
    size_t m_InputBlobSize;
    unsigned int m_JobNumber;
    bool m_ExclusiveJob;

    CRef<CWorkerNodeCleanup> m_CleanupEventSource;

    CRef<CRequestContext> m_RequestContext;
    CRequestRateControl m_StatusThrottler;
    CRequestRateControl m_ProgressMsgThrottler;
    CNetScheduleExecutor m_NetScheduleExecutor;
    CNetCacheAPI m_NetCacheAPI;
    SGridRead m_GridRead;
    SGridWrite m_GridWrite;

    // Used for the job "pullback" mechanism.
    unsigned m_JobGeneration;

    CDeadline m_CommitExpiration;
    bool      m_FirstCommitAttempt;

private:
    CDeadline m_Deadline;
};

class CJobRunRegistration;

class CRunningJobLimit
{
public:
    CRunningJobLimit() : m_MaxNumber(0) {}

    void ResetJobCounter(unsigned max_number) {m_MaxNumber = max_number;}

    bool CountJob(const string& job_group,
            CJobRunRegistration* job_run_registration);

private:
    friend class CJobRunRegistration;

    unsigned m_MaxNumber;

    CFastMutex m_Mutex;

    typedef map<string, unsigned> TJobCounter;
    TJobCounter m_Counter;
};

class CJobRunRegistration
{
public:
    CJobRunRegistration() : m_RunRegistered(false) {}

    void RegisterRun(CRunningJobLimit* job_counter,
            CRunningJobLimit::TJobCounter::iterator job_group_it)
    {
        m_JobCounter = job_counter;
        m_JobGroupCounterIt = job_group_it;
        m_RunRegistered = true;
    }

    ~CJobRunRegistration()
    {
        if (m_RunRegistered) {
            CFastMutexGuard guard(m_JobCounter->m_Mutex);

            if (--m_JobGroupCounterIt->second == 0)
                m_JobCounter->m_Counter.erase(m_JobGroupCounterIt);
        }
    }

private:
    CRunningJobLimit* m_JobCounter;
    CRunningJobLimit::TJobCounter::iterator m_JobGroupCounterIt;
    bool m_RunRegistered;
};

struct SSuspendResume
{
    SSuspendResume() : m_Event(eNoEvent), m_IsSuspended(false), m_CurrentJobGeneration(0) {}

    void Suspend(bool pullback, unsigned timeout);
    void Resume() volatile;
    void SetJobPullbackTimer(unsigned seconds);
    bool IsJobPullbackTimerExpired();

    enum EState { eRunning, eSuspending, eSuspended };
    EState CheckState() volatile;
    bool IsSuspended() const volatile { return m_IsSuspended.load(); }
    unsigned GetCurrentJobGeneration() const volatile { return m_CurrentJobGeneration.load(); }
    unsigned GetDefaultPullbackTimeout() const { return m_DefaultPullbackTimeout; }
    void SetDefaultPullbackTimeout(unsigned seconds) { m_DefaultPullbackTimeout = seconds; }

private:
    enum EEvent { eNoEvent, eSuspend, eResume };
    atomic<EEvent> m_Event;
    atomic<bool> m_IsSuspended;
    atomic<unsigned> m_CurrentJobGeneration;
    unsigned m_DefaultPullbackTimeout = 0;
    CDeadline m_JobPullbackTime = 0;
};

///@internal
struct SGridWorkerNodeImpl : public CObject, IWorkerNodeInitContext
{
    SGridWorkerNodeImpl(CNcbiApplicationAPI& app,
            IWorkerNodeJobFactory* job_factory);

    void AddJobWatcher(IWorkerNodeJobWatcher& job_watcher,
                          EOwnership owner = eNoOwnership);

    void Init();

    int Run(
#ifdef NCBI_OS_UNIX
            ESwitch daemonize,
#endif
            string procinfo_file_name);

    void x_WNCoreInit();
    void x_StartWorkerThreads();
    void x_StopWorkerThreads();
    void x_ClearNode();
    int x_WNCleanUp();

    const string& GetQueueName() const
    {
        return m_NetScheduleAPI.GetQueueName();
    }
    const string& GetClientName() const
    {
        return m_NetScheduleAPI->m_Service->GetClientName();
    }
    const string& GetServiceName() const
    {
        return m_NetScheduleAPI->m_Service->m_ServiceName;
    }

    string GetAppName() const { return m_App.GetProgramDisplayName(); }

    bool EnterExclusiveMode();
    void LeaveExclusiveMode();
    bool IsExclusiveMode() const {return m_IsProcessingExclusiveJob;}
    bool WaitForExclusiveJobToFinish();

    int OfflineRun();

    // IWorkerNodeInitContext implementation
    const IRegistry&               GetConfig()             const override;
    const CArgs&                   GetArgs()               const override;
    const CNcbiEnvironment&        GetEnvironment()        const override;
    IWorkerNodeCleanupEventSource* GetCleanupEventSource() const override;
    CNetScheduleAPI                GetNetScheduleAPI()     const override;
    CNetCacheAPI                   GetNetCacheAPI()        const override;

    unique_ptr<IWorkerNodeJobFactory> m_JobProcessorFactory;

    CNetCacheAPI m_NetCacheAPI;
    CNetScheduleAPI m_NetScheduleAPI;
    CNetScheduleExecutor m_NSExecutor;
    CStdPoolOfThreads* m_ThreadPool;

    unsigned int                 m_MaxThreads;
    unsigned int                 m_NSTimeout;
    mutable CFastMutex           m_JobProcessorMutex;
    unsigned                     m_CommitJobInterval;
    unsigned                     m_CheckStatusPeriod;
    CSemaphore                   m_ExclusiveJobSemaphore;
    bool                         m_IsProcessingExclusiveJob;
    Uint8                        m_TotalMemoryLimit;
    unsigned                     m_TotalTimeLimit;
    time_t                       m_StartupTime;
    unsigned                     m_QueueTimeout;

    typedef map<IWorkerNodeJobWatcher*,
            AutoPtr<IWorkerNodeJobWatcher> > TJobWatchers;
    CFastMutex m_JobWatcherMutex;
    TJobWatchers m_Watchers;

    CRunningJobLimit m_JobsPerClientIP;
    CRunningJobLimit m_JobsPerSessionID;

    CRef<CWorkerNodeCleanup> m_CleanupEventSource;

    IWorkerNodeJob* GetJobProcessor();

    void x_NotifyJobWatchers(const CWorkerNodeJobContext& job_context,
                            IWorkerNodeJobWatcher::EEvent event);

    set<SSocketAddress> m_Masters;
    set<unsigned int> m_AdminHosts;

    SThreadSafe<SSuspendResume> m_SuspendResume;

    bool x_AreMastersBusy() const;

    CRef<CJobCommitterThread> m_JobCommitterThread;
    CRef<CWorkerNodeIdleThread>  m_IdleThread;

    unique_ptr<IGridWorkerNodeApp_Listener> m_Listener;

    CNcbiApplicationAPI& m_App;
    CSynRegistry::TPtr m_SynRegistry;
    CRef<IRegistry> m_Registry;
    bool m_SingleThreadForced;
    bool m_LogRequested;
    bool m_ProgressLogRequested;
    size_t m_QueueEmbeddedOutputSize;
    unsigned m_ThreadPoolTimeout;

    /// Bookkeeping of jobs being executed (to prevent simultaneous runs of the same job)
    struct SJobsInProgress
    {
        bool Add(const CNetScheduleJob& job)
        {
            TFastMutexGuard lock(m_Mutex);
            auto it = m_Jobs.find(job.job_id);

            if (it == m_Jobs.end()) {
                return m_Jobs.emplace(job.job_id, job.auth_token).second;
            }

            it->second = job.auth_token;
            return false;
        }

        void Update(CNetScheduleJob& job)
        {
            TFastMutexGuard lock(m_Mutex);
            auto it = m_Jobs.find(job.job_id);

            _ASSERT(it != m_Jobs.end());
            job.auth_token = it->second;
        }

        void Remove(const CNetScheduleJob& job)
        {
            TFastMutexGuard lock(m_Mutex);
            auto it = m_Jobs.find(job.job_id);

            _ASSERT(it != m_Jobs.end());
            m_Jobs.erase(it);
        }

    private:
        CFastMutex m_Mutex;
        unordered_map<string, string> m_Jobs;
    };

    SJobsInProgress m_JobsInProgress;
};


///@internal
class CWorkerNodeRequest : public CStdRequest
{
public:
    CWorkerNodeRequest(SWorkerNodeJobContextImpl* context) :
        m_JobContext(context)
    {
    }

    virtual void Process();

private:
    CWorkerNodeJobContext m_JobContext;
};


/////////////////////////////////////////////////////////////////////////////
//

bool g_IsRequestStartEventEnabled();
bool g_IsRequestStopEventEnabled();

/////////////////////////////////////////////////////////////////////////////
//
/// @internal
class CMainLoopThread : public CThread
{
public:
    CMainLoopThread(SGridWorkerNodeImpl* worker_node) :
        m_WorkerNode(worker_node),
        m_Impl(worker_node),
        m_Timeline(m_Impl),
        m_ThreadName(worker_node->GetAppName() + "_mn")
    {
    }

    virtual void* Main();

private:
    class CImpl : public CNetScheduleGetJob
    {
    public:
        CImpl(SGridWorkerNodeImpl* worker_node) :
            m_API(worker_node->m_NetScheduleAPI),
            m_Timeout(worker_node->m_NSTimeout),
            m_WorkerNode(worker_node)
        {
        }

        void Main();

        EState CheckState();
        CNetServer ReadNotifications();
        CNetServer WaitForNotifications(const CDeadline& deadline);
        bool MoreJobs(const SEntry& entry);
        bool CheckEntry(
                SEntry& entry,
                const string& prio_aff_list,
                bool any_affinity,
                CNetScheduleJob& job,
                CNetScheduleAPI::EJobStatus* job_status);
        void ReturnJob(CNetScheduleJob& job);

        CNetScheduleAPI m_API;
        const unsigned m_Timeout;

    private:
        SGridWorkerNodeImpl* m_WorkerNode;

        CNetServer x_ProcessRequestJobNotification();
    };

    bool x_GetNextJob(CNetScheduleJob& job, const CDeadline& deadline);

    SGridWorkerNodeImpl* m_WorkerNode;
    CImpl m_Impl;
    CNetScheduleGetJobImpl<CImpl> m_Timeline;
    const string m_ThreadName;
};

END_NCBI_SCOPE

#endif /* CONNECT_SERVICES__GRID_WORKER_IMPL__HPP */
