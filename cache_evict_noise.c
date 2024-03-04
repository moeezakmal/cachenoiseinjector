#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h> // May need additional system includes
#include <immintrin.h> // For _mm_clflush
#include <stdint.h> // For uintptr_t
#include <stdbool.h> // For true

// Function to simulate random cache eviction
void evict_random_cache_lines(void **ptrs, size_t num_ptrs, size_t cache_line_size) {
    for (size_t i = 0; i < num_ptrs; i++) {
        void *ptr = ptrs[i];

        // Ensure alignment on cache line boundary
        if ((uintptr_t)ptr % cache_line_size != 0) {
            // Handle misaligned pointer (optional, you might raise an error here)
            return; 
        }

        // Flush the cache line
        _mm_clflush(ptr);
    }
}

int main() {
    // Seed random number generator
    srand(time(0));

    // Parameters (may need adjustment)
    size_t cache_line_size = 64; // Typical cache line size (bytes)
    //size_t total_memory = 7 * 1024 * 1024 * 1024; // 7GB (adjust based on system memory)
    size_t total_memory = 7;
    total_memory *= 1024; // Now total_memory is 7KB
    total_memory *= 1024; // Now total_memory is 7MB
    total_memory *= 1024; // Now total_memory is 7GB
    size_t num_cache_lines = 32 * 4096; // Number of cache lines to evict simultaneously

    // Become a daemon process (See additional notes on daemonization)
    if (daemon(0, 0) == -1) {
        perror("daemon() failed");
        exit(1); 
    }

    // Allocate memory for random access
    void *mem = mmap(NULL, total_memory, PROT_READ | PROT_WRITE, 
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap() failed");
        exit(1);
    }

    // Allocate memory for pointers
    void **ptrs = malloc(num_cache_lines * sizeof(void *));
    if (ptrs == NULL) {
        perror("malloc() failed");
        exit(1);
    }

    while (true) {
        // Calculate random offsets within allocated memory
        for (size_t i = 0; i < num_cache_lines; i++) {
            //size_t offset = (rand() % total_memory);
	    size_t offset = (rand() % (total_memory / cache_line_size)) * cache_line_size;
            ptrs[i] = mem + offset;
        }

        // Simulate cache eviction
        evict_random_cache_lines(ptrs, num_cache_lines, cache_line_size);

        // Sleep for a short duration (avoid excessive CPU usage)
        //usleep(10000); // Sleep for 10 milliseconds (adjust as needed)
    }

    // Clean up (optional before daemon exits)
    free(ptrs);
    munmap(mem, total_memory);
    return 0;
}

