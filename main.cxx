#include <iostream>
#include <string>
#include <cpufreq.h>
extern "C" {
    #include <cpuidle.h>
}
#include <hwloc.h>
#include <time.h>

using namespace std;

// Init
int main() {

	long unsigned int min_freq;
	long unsigned int max_freq;
	
	hwloc_topology_t topology;
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);

	hwloc_const_cpuset_t cpuset;
	cpuset = hwloc_topology_get_complete_cpuset(topology);
	
	int ncpus = hwloc_bitmap_weight(cpuset);

	printf("%d\n", ncpus);

	const long tick_ms = 400; // ~1/2 Hz @ 512
    struct timespec ticks = {0, tick_ms * 1000000};

	int getfreq = cpufreq_get_hardware_limits(0, &min_freq, &max_freq);

	char gov_perf[] = "performance";
	char gov_save[] = "powersave";

	for (int cpu_count = 0; cpu_count < ncpus; cpu_count++) {
		cpufreq_modify_policy_min(cpu_count, min_freq);
		cpufreq_modify_policy_max(cpu_count, max_freq);
	}

	while (true) {

		printf("Looping!\n");
		for (int cpu_count = 0; cpu_count < ncpus; cpu_count++) {
			int current_freq = cpufreq_get_freq_kernel(cpu_count);
			struct cpufreq_policy *policy;
			policy = cpufreq_get_policy(cpu_count);
			printf("%s\n", policy->governor);
			printf ("%d\n", current_freq);
			// below 3633mhz set to powersave
			if (current_freq < max_freq / 1.2) {
				cpufreq_modify_policy_governor(cpu_count, gov_save);
			}
			
			// if below 4.1ghz set cstates to low freq
			if (current_freq < max_freq / 1.2) {
				for (int cstate_num = 0; cstate_num <= 4; ++cstate_num) {
					cpuidle_state_disable(cpu_count, cstate_num, 1);
				}
			}

			// below 4260mhz set to min freq
			if (current_freq < max_freq / 1.15) {
				cpufreq_modify_policy_min(cpu_count, min_freq);
			}

			// above 3633mhz set to performance
			if (current_freq > max_freq / 1.2) {
				cpufreq_modify_policy_governor(cpu_count, gov_perf);
			}

			// if above 4.1ghz set cstates to normal
			if (current_freq > max_freq / 1.2) {
				for (int cstate_num = 0; cstate_num <= 4; ++cstate_num) {
					cpuidle_state_disable(cpu_count, cstate_num, 0);
				}
			}

			// above 3760mhz then set min to 3602mhz
			if (current_freq > max_freq / 1.17) {
				cpufreq_modify_policy_min(cpu_count, max_freq / 3);
			}
		}
		printf("Done!\n");

		nanosleep(&ticks, NULL);

	}

}
