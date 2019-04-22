void vh_pmu_ic_handler(void);
void vh_pmu_init(void);

/*******************************************************************
	Peformance Monitoring counters - Cortex-A9
*******************************************************************/
#define	PMUCNT0	0
#define	PMUCNT1	1
#define	PMUCNT2 2
#define	PMUCNT3 3
#define	PMUCNT4 4
#define	PMUCNT5 5

/*******************************************************************
	Peformance Monitoring events - Cortex-A9
*******************************************************************/
#define PMUEVT10	0x10	// Branch mispredicted or not predicted
#define PMUEVT12	0x12	// Branch or other change in program flow that could have been predicted
#define PMUEVT68	0x68	// Instructions coming out of the core renaming stage


