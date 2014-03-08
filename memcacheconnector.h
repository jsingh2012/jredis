// compile: gcc -w -g  memcacheconnector.c -lmemcached  -o memcacheconnector
#include <unistd.h>
#include <libmemcached/memcached.h>

int update_cache(const char key[], const char value[])
{
    memcached_server_st *servers = NULL;
    memcached_st *memc;
    memcached_return rc;
    memc = memcached_create(NULL);
    servers = memcached_server_list_append(servers, "75.126.75.208", 11211, &rc);
   // servers = memcached_server_list_append(servers, "localhost", 11211, &rc);
    rc= memcached_server_push(memc, servers);
    if (!(rc == MEMCACHED_SUCCESS))
        return -1;
    rc= memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);
    if (!(rc == MEMCACHED_SUCCESS))
        return -2;
    return 0;
}
