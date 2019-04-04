#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>


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

#define FUNC(x) (x)*(x)
#define START 3
#define FINISH 9

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

long long parse_n (char* str);
void sys_info(unsigned n, SysInfo * sysInfo);
void* calculate(void* args);


int main(int argc, char** argv) {
    CHECK(argc != 2, invalid argument, EINVAL);
    unsigned n = parse_n(argv[1]);

    SysInfo sysInfo;
    sysInfo.use_hyper = 0;
    sys_info(n, &sysInfo);

    pthread_t threads[10000];
    CalculateData data[10000];
    
    if (!sysInfo.use_hyper) {
        double  interval_start = START,
                interval_len = (double)(FINISH - START) / (double) n;
        unsigned j = 0, i = 0;

        for (i = 0; i < sysInfo.online_cpus_num; ++i) {
            
            while(!sysInfo.is_cpu_online[j]) ++j;

            data[i] = (CalculateData) {
                interval_start,
                interval_start + interval_len,
                0,
                sysInfo.cpu_sets[j],
            };

            interval_start += interval_len;
            j++;

        }

        for (i = 0; i < sysInfo.online_cpus_num; i++)
            CHECK(pthread_create(&threads[i], NULL, &calculate, &data[i]), pthread_create, 0);

        double sum = 0;
        for (i = 0; i < sysInfo.online_cpus_num; ++i) {
            CHECK(pthread_join(threads[i], NULL), pthread_join, 0);
            if (i < n)
                sum += data[i].result;
        }

        printf ("%.5f\n", sum);
    }
    else {
        double  interval_start = START,
                interval_len = (double)(FINISH - START) / (double) n;
        unsigned i;

        for (i = 0; i < n; ++i) {

            data[i] = (CalculateData) {
                interval_start,
                interval_start + interval_len,
                0,
                sysInfo.virtual_cpu_sets[i % sysInfo.online_virtual_cpus_num]
            };
            interval_start += interval_len;
        }

        for (i = 0; i < n; i++)
            CHECK(pthread_create(&threads[i], NULL, &calculate, &data[i]), pthread_create, 0);

        double sum = 0;
        for (i = 0; i < n; ++i) {
            CHECK(pthread_join(threads[i], NULL), pthread_join, 0);
            sum += data[i].result;
        }

        printf ("%.5f\n", sum);
    }

    return 0;
}

long long parse_n (char* str){
    char* endptr = 0;
    long long val = 0;
    errno = 0;

    val = strtoll(str, &endptr, 10);

    CHECK(*endptr != '\0' || endptr == str, parse_n, EINVAL);

    CHECK((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))  ||
          (errno != 0 && val == 0), parse_n, 0);

    return val;
}

void sys_info(unsigned n, SysInfo * sysInfo) {
    memset(&(sysInfo -> is_virtual_cpu_online), 0, sizeof(sysInfo -> is_virtual_cpu_online));

    int fd = open("/sys/devices/system/cpu/online", O_RDONLY);
    CHECK(fd == -1, open, 0);
    char online_str[MAX_SYS_FILE_LENGTH];
    int bytes_read = read(fd, online_str, MAX_SYS_FILE_LENGTH-1);
    CHECK(bytes_read == -1, read, 0);
    close(fd);
    online_str[bytes_read] = '\0';

    unsigned left_bound, right_bound, i;
    char *saveptr1 = NULL,
         *saveptr2 = NULL,
         *str = strtok_r(online_str, ",", &saveptr1);

    do {
        str = strtok_r(str, "-", &saveptr2);
        CHECK(!sscanf(str, "%u", &left_bound), sscanf, 0);
        right_bound = left_bound;

        str = strtok_r(NULL, "-", &saveptr2);
        if (str)
            CHECK(!sscanf(str, "%u", &right_bound), sscanf, 0);

        for (i = left_bound; i <= right_bound; i++)
            sysInfo -> is_virtual_cpu_online[i] = 1;
    } while ((str = strtok_r(NULL, ",", &saveptr1)));


    memset(&(sysInfo -> is_cpu_online), 0, sizeof(sysInfo -> is_cpu_online));
    for (i = 0; i < MAX_CPUS; i++) {
        CPU_ZERO(&(sysInfo -> cpu_sets[i]));
        CPU_ZERO(&(sysInfo -> virtual_cpu_sets[i]));
    }
    
    char filename[256],
         core_str[11];
    int  core_id;

    for (i = 0; i < MAX_CPUS; i++) {
        if (!sysInfo -> is_virtual_cpu_online[i])
            continue;

        CPU_SET(i, &(sysInfo -> virtual_cpu_sets[sysInfo -> online_virtual_cpus_num]));
        sysInfo -> online_virtual_cpus_num++;

        sprintf(filename, "/sys/devices/system/cpu/cpu%d/topology/core_id", i);
        fd = open(filename, O_RDONLY);
        CHECK(fd == -1, open, 0);
        int bytes_read = read(fd, core_str, 10);
        CHECK(bytes_read == -1, read, 0);
        close(fd);
        core_str[bytes_read] = '\0';

        CHECK(!sscanf(core_str, "%d", &core_id), sscanf, 0);

        if (!sysInfo -> is_cpu_online[core_id]) {
            sysInfo -> is_cpu_online[core_id] = 1;
            sysInfo -> online_cpus_num++;

            CPU_SET(i, &(sysInfo -> cpu_sets[core_id]));
        }
    }

    if (sysInfo -> online_cpus_num < n) sysInfo -> use_hyper = 1;
}


void* calculate(void* args) {
    cpu_set_t cpuset = ((CalculateData*)args)->cpuset;
    pthread_t current_thread = pthread_self();
    CHECK(pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset), pthread_setaffinity_np, 0);

    double start = ((CalculateData*)args) -> start,
            finish = ((CalculateData*)args) -> finish,
            dx = 0.000000001,
            result = 0;

    while (start < finish){
        result += FUNC(start) * dx;
        start += dx;
    }

    ((CalculateData*)args)->result = result;
    return NULL;
}
