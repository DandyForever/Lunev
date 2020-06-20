#define DEBUG
#define BROADCAST_PORT  31415
#define FUNC(x)         (x)*(x)
#define SPLIT           2000000000

#define MAX_CPUS 256
#define MAX_SYS_FILE_LENGTH 256


typedef struct CalculateData {
    double start;
    double finish;
    double result;
    cpu_set_t cpuset;
} CalculateData;

typedef struct SysInfo {
    unsigned is_virtual_cpu_online[MAX_CPUS];
    unsigned is_cpu_online[MAX_CPUS];
    unsigned online_cpus_num;
    unsigned online_virtual_cpus_num;
    unsigned use_hyper;
    cpu_set_t cpu_sets[MAX_CPUS];
    cpu_set_t virtual_cpu_sets[MAX_CPUS];
} SysInfo;

#define CHECK(cond, msg, err)				\
	do {						            \
		if (cond)				            \
		{					                \
			if (err) errno = err;		    \
			printf ("%d:", __LINE__);	    \
			perror (#msg);			        \
			exit (-1);		            	\
		}					                \
	} while (0)

typedef struct net_msg {
    int tcp_port;
    unsigned cores, steps;
    long double interval_start,
                interval_end,
                h;
} net_msg;
