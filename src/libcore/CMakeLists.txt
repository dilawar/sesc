set(SRCS Processor.cpp GProcessor.cpp LDSTQ.cpp
    SMTProcessor.cpp DInst.cpp Pipeline.cpp 
    FetchEngine.cpp Resource.cpp Cluster.cpp DepWindow.cpp BPred.cpp 
    MemRequest.cpp MemObj.cpp  OSSim.cpp LDSTBuffer.cpp 
    ProcessId.cpp RunningProcs.cpp GMemorySystem.cpp ValueTable.cpp 
    GMemoryOS.cpp VPred.cpp 
    )


if(SESC_INORDER)
    list(APPEND SRCS Signature.cpp)
endif()


if(TLS)
    list(APPEND SRCS Epoch.cpp Checkpoint.cpp AdvancedStats.cpp SysCall.cpp)
endif()

if(TASKSCALAR)
    if(TLS)
        message(FATAL_ERROR " You can not compile TaskScalar simultaneously with TLS")
    endif()
    list(APPEND SRCS TaskContext.cpp TaskHandler.cpp MemBuffer.cpp GLVID.cpp)
endif()

if(ATOMIC)
    list(APPEND SRCS ASVersion.cpp)
else()
    list(APPEND SRCS HVersion.cpp)
endif()

if(TS_PROFILING)
    list(APPEND SRCS Profile.cpp)
endif()

if(TS_RISKLOADPROF)
    list(APPEND SRCS RiskLoadProf.cpp)
endif()

if(SESC_ENERGY)
    list(APPEND SRCS  EnergyMgr.cpp)
endif()

if(SESC_SLICE)
    list(APPEND SRCS  Slicer.cpp)
endif()

add_library(core STATIC ${SRCS})
