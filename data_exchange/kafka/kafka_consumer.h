#include <string.h>
// Kafka header file
#include <librdkafka/rdkafka.h>

typedef rd_kafka_t* data_conn_ptr; /*!< Pointer to the data connection */
typedef void* data_sub_ptr;        /*!< Pointer to the data subscription */

struct dataconn_props_struct {
    char groupid[128];      /*!< */
    char servers[128];      /*!< Bootstrap servers and ports, e.g. host1:9092,host2:9092 */
    char topic[128];        /*!< Topic, but shd this be topics if we do data and event? List of topics to subscribe to */
};

/* Read parmaeters from the ini file */
int data_connection_readIni(const char *propfilename,
                            const char *group,
                            struct dataconn_props_struct *data_conn_props);

/* Initialize the data connection */
int initialize_data_connection(data_conn_ptr *rk_call,
                               data_sub_ptr *sk_call,
                               const struct dataconn_props_struct* props);

/* Get data from the data connection */
int get_data(data_conn_ptr rk,
             data_sub_ptr sk);

/* Close the data connection */
void close_data_connection(data_conn_ptr rk,
                           data_sub_ptr sk);
