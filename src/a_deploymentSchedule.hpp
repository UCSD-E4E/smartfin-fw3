


struct  DeploymentSchedule_
 {
    //Ensemble properties
    int priority;
    void (*measure)();               //!< measurement function
    void (*init)();                       //!< initialization function
    //EnsembleProccess process;             //!< processing function
    uint32_t startDelay;                 //!< delay after deployment start
    uint32_t ensembleInterval;              //!< time between ensembles
    //void* pData;               //!< Buffer to store measurements temporarily
    uint32_t maxDuration;       //!< store max running time of measurement
    char taskName;  //!< task name of ensemble
    uint32_t maxDelay;
   
    //State Information
    uint32_t nextRunTime;
};

