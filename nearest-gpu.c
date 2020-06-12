/**
 * @file
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include <hwloc.h>
#include <hwloc/cudart.h>

/**
 * @brief check if HWLOC command has failed
 *
 * HWLOC functions return 0 for true, and -1 (or other non-zero) value for
 * false. This is opposite to normal C conditional checks, as such we define a
 * macro to make this distinction less confusing.
 *
 * @param ret the return value (assumed to be signed int)
 */
#define HWLOC_IS_TRUE(ret) ret == 0

static int
printlog (const char * fmt, ...)
{
    time_t current_time;
    struct tm *ltime;
    char *fmt_buf;
    char time_str[32], outbuf[1024];
    va_list ap;
    int size;

    current_time = time (NULL);
    ltime = localtime (&current_time);
    strftime (time_str, 32, "%F %T", ltime);
    snprintf (outbuf, 1024, "[%s] %s\n", time_str, fmt);

    va_start(ap, fmt);
    size = vfprintf(stderr, outbuf, ap);
    va_end(ap);

    return size;
}

/**
 * @brief snprintf equivalent function for printing HWLOC object information.
 *
 * @param str An allocated string pointer
 * @param size Length of string, not include `\0`
 * @param topo HWLOC topology structure
 * @param cpuset HWLOC cpuset structure
 * @param bind HWLOC bind structure
 * @returns EXIT_SUCCESS or EXIT_FAILURE
 */
static int
hwloc_info_snprintf (char *str, size_t size, hwloc_topology_t topo,
                         hwloc_cpuset_t cpuset, hwloc_cpuset_t bind)
{
    char st1[128];
    char st2[128];
    hwloc_obj_t obj1 = NULL;
    hwloc_obj_t obj2 = NULL;
    int exits = EXIT_SUCCESS;

    // There should only be one obj - as this cpuset was singlefied before...
    // obj1 = hwloc_get_next_obj_inside_cpuset_by_type(topo, bind, HWLOC_OBJ_CORE, obj1);
    obj1 = hwloc_get_obj_inside_cpuset_by_type (topo, bind, HWLOC_OBJ_CORE, 0);
    if (!obj1) {
        obj1 = hwloc_get_obj_inside_cpuset_by_type (topo, bind, HWLOC_OBJ_PU, 0);
        if (!obj1)
            fprintf (stderr, "-! Unable to find bind object!\n");
    }
    // this should return the package - cpusocket
    // obj2 = hwloc_get_next_obj_inside_cpuset_by_type(topo, cpuset, HWLOC_OBJ_NODE,
    // obj2);
    obj2 = hwloc_get_obj_inside_cpuset_by_type (topo, cpuset, HWLOC_OBJ_NODE, 0);
    if (!obj2) {
        obj2 = hwloc_get_obj_inside_cpuset_by_type (topo, cpuset, HWLOC_OBJ_SOCKET, 0);
        if (!obj2)
            fprintf (stderr, "-! Unable to find cpuset object!\n");
    }

    if (!obj1 || !obj2) {
        exits = EXIT_FAILURE;
    } else {
        hwloc_obj_type_snprintf (st1, sizeof (st1), obj1, 0);
        hwloc_obj_type_snprintf (st2, sizeof (st2), obj2, 0);
        snprintf (str, size, "%s #%d in %s #%d", st1, obj1->logical_index, st2,
                  obj2->logical_index);
    }

    return exits;
}

/**
 * From a HWLOC object representing a CPU, find the first listed real core (not hyperthreaded)
 * and return a copy of its HWLOC object representation.
 *
 * @param cpuset HWLOC CPU object to search in
 * @returns A HWLOC cpuset representing a single real core
 */
static hwloc_bitmap_t
get_core (hwloc_cpuset_t cpuset, hwloc_topology_t topology)
{
    hwloc_bitmap_t res;
    hwloc_obj_t obj_core = NULL;

    res = hwloc_bitmap_alloc ();
    // this will get the first core...
    obj_core = hwloc_get_next_obj_inside_cpuset_by_type (topology, cpuset,
                                                         HWLOC_OBJ_CORE, obj_core);

    if (obj_core) {
        // get a copy of its cpuset that we may modify
        res = hwloc_bitmap_dup (obj_core->cpuset);
        // get only one logical processor (in case the core is SMT/hyperthreaded)
        hwloc_bitmap_singlify (res);
    } else {
        printlog ("Unable to find a core within the given HWLOC cpuset!");
        exit (1);
    }

    return res;
}

static void
get_nearest_cpu (unsigned cuda_ordinal)
{
    hwloc_bitmap_t tmp_hw_cpuset;
    hwloc_bitmap_t nearest_cpu_sets;
    hwloc_topology_t tmp_topology;
    char buf[1028];


    hwloc_topology_init (&tmp_topology);
#if HWLOC_API_VERSION < 0x00020000
    hwloc_topology_set_flags (tmp_topology,
                              HWLOC_TOPOLOGY_FLAG_IO_DEVICES); // include access to IO
                                                               // devices
#else
    hwloc_topology_set_io_types_filter (tmp_topology,
                                        HWLOC_TYPE_FILTER_KEEP_ALL);
#endif
    hwloc_topology_load (tmp_topology);

    tmp_hw_cpuset = hwloc_bitmap_alloc (); // empty map on cpuset
    if (HWLOC_IS_TRUE (hwloc_cudart_get_device_cpuset (tmp_topology,
                                                       cuda_ordinal,
                                                       tmp_hw_cpuset))) {
        // get nearest CPU core
        nearest_cpu_sets = get_core (tmp_hw_cpuset, tmp_topology);

        hwloc_info_snprintf (buf, sizeof (buf), tmp_topology, tmp_hw_cpuset, nearest_cpu_sets);

        printf ("%s\n", buf);
    } else {
        printlog ("Unable to find CPU/core nearest CUDA device %d!\n", cuda_ordinal);
        exit (1);
    }

    hwloc_bitmap_free (tmp_hw_cpuset);
    hwloc_bitmap_free (nearest_cpu_sets);
}

int main (int argc, char ** argv)
{
    int cuda_ordinal = 0;
    int opt;

    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
        case 'h':
        default:
            fprintf(stderr, "Usage: %s [-h] [cuda device idx]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (argc > 1) {
        sscanf (argv[optind], "%d", &cuda_ordinal);
    }

    get_nearest_cpu (cuda_ordinal);

    return EXIT_SUCCESS;
}
