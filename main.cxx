extern "C" {
    #include <cpuidle.h>
}
#include <cpufreq.h>
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

	const long tick_ms = 360; // ~1/2 Hz @ 512
    struct timespec ticks = {0, tick_ms * 1000000};

	cpufreq_get_hardware_limits(0, &min_freq, &max_freq);

	char gov_perf[] = "performance";
	char gov_save[] = "powersave";

	for (int cpu_count = 0; cpu_count < ncpus; cpu_count++) {
		cpufreq_modify_policy_min(cpu_count, min_freq);
		cpufreq_modify_policy_max(cpu_count, max_freq);
		for (int cstate_num = 0; cstate_num <= 15; ++cstate_num) {
			cpuidle_state_disable(cpu_count, cstate_num, 0);
		}
	}

	while (true) {

		for (int cpu_count = 0; cpu_count < ncpus; cpu_count++) {
			int current_freq = cpufreq_get_freq_kernel(cpu_count);
			struct cpufreq_policy *policy;
			policy = cpufreq_get_policy(cpu_count);
			printf("%s\n", policy->governor);
			printf ("%d\n", current_freq);
			// below 3633mhz set to powersave
			if (current_freq < max_freq / 1.22) {
				cpufreq_modify_policy_governor(cpu_count, gov_save);
			}
			
			// if below 4.1ghz set cstates to all enable
			if (current_freq < max_freq / 1.36) {
				for (int cstate_num = 0; cstate_num <= 15; ++cstate_num) {
					cpuidle_state_disable(cpu_count, cstate_num, 0);
				}
			}
			
			// above 3633mhz set to performance
			if (current_freq > max_freq / 1.2) {
				cpufreq_modify_policy_governor(cpu_count, gov_perf);
			}

			// if above 4.1ghz set cstates 7-15 disable
			if (current_freq > max_freq / 1.36) {
				for (int cstate_num = 4; cstate_num <= 15; ++cstate_num) {
					cpuidle_state_disable(cpu_count, cstate_num, 1);
				}
			}

		}

		nanosleep(&ticks, NULL);

	}

}
