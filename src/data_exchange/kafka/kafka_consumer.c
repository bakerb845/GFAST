/*
 * librdkafka - Apache Kafka C library
 *
 * Copyright (c) 2019-2022, Magnus Edenhill
 *               2023, Confluent Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Simple high-level balanced Apache Kafka consumer
 * using the Kafka driver from librdkafka
 * (https://github.com/confluentinc/librdkafka)
 */

/**
 * Modified for Kafka consumer for GFAST, using abstraction to facilitate different types of
 * connection.
 * First version author: Jen Andrews
 */

#ifdef WINNT
#include <window.h>
#include <winuser.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <iniparser.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "iscl/os/os.h"
#include "gfast_dataexchange.h"
#include "gfast_core.h"
#include <librdkafka/rdkafka.h>

// TODO:
// Prepare the configuration in the calling program
// Check the header files included

/**
 * @returns 1 if all bytes are printable, else 0.
 */
static int is_printable(const char *buf, size_t size) {
        size_t i;

        for (i = 0; i < size; i++)
                if (!isprint((int)buf[i]))
                        return 0;

        return 1;
}


/*!
 * @brief Initializes kafka consumer.
 * @param[in] props         Data connection properties.
 * @param[out] connection    Data connection pointer (void *)
 * @result 0 indicates success.
 */
int dataexchange_kafka_connect(
    struct dataconn_props_struct *props,
    void **connection) {

    rd_kafka_conf_t *conf;    /* Temporary configuration object */
    rd_kafka_resp_err_t err;  /* librdkafka API error code */
    char errstr[512];         /* librdkafka API error reporting buffer */
    rd_kafka_topic_partition_list_t *subscription; /* Subscribed topics */

    /*
     * Create Kafka client configuration place-holder
     */
    conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", props->servers, errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            LOG_ERRMSG("%s\n", errstr);
            rd_kafka_conf_destroy(conf);
            return 1;
    }

    /* Set the consumer group id.
     * All consumers sharing the same group id will join the same
     * group, and the subscribed topic' partitions will be assigned
     * according to the partition.assignment.strategy
     * (consumer config property) to the consumers in the group. */
    srand(time(NULL));
    int r = rand();
    char str[12];
    sprintf(str, "%d", r);
    //LOG_MSG("JADEBUG: setting group.id to %s", str);
    if (rd_kafka_conf_set(conf, "group.id", str, errstr,
    //if (rd_kafka_conf_set(conf, "group.id", props->groupid, errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            LOG_ERRMSG("%s\n", errstr);
            rd_kafka_conf_destroy(conf);
            return 1;
    }

    /* If there is no previously committed offset for a partition
     * the auto.offset.reset strategy will be used to decide where
     * in the partition to start fetching messages.
     * By setting this to earliest the consumer will read all messages
     * in the partition if there was no previously committed offset. 
     * By setting to latest, it should start with the latest logged offset.
     * */
    if (rd_kafka_conf_set(conf, "auto.offset.reset", "latest", errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            LOG_ERRMSG("%s\n", errstr);
            rd_kafka_conf_destroy(conf);
            return 1;
    }

    /*
     * Create consumer instance.
     *
     * NOTE: rd_kafka_new() takes ownership of the conf object
     *       and the application must not reference it again after
     *       this call.
     */
    rd_kafka_t* rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!rk) {
            LOG_ERRMSG("%% Failed to create new consumer: %s\n",
                    errstr);
            return 1;
    }

    conf = NULL; /* Configuration object is now owned, and freed,
                  * by the rd_kafka_t instance. */

    /* Redirect all messages from per-partition queues to
     * the main queue so that messages can be consumed with one
     * call from all assigned partitions.
     *
     * The alternative is to poll the main queue (for events)
     * and each partition queue separately, which requires setting
     * up a rebalance callback and keeping track of the assignment:
     * but that is more complex and typically not recommended. */
    rd_kafka_poll_set_consumer(rk);


    /* Convert the list of topics to a format suitable for librdkafka */
    subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_t * rktp = rd_kafka_topic_partition_list_add(subscription, 
        props->topic,
        /* the partition is ignored by subscribe() */
        RD_KAFKA_PARTITION_UA);

    /* Subscribe to the list of topics */
    err = rd_kafka_subscribe(rk, subscription);
    if (err) {
            LOG_ERRMSG("%% Failed to subscribe to %d topics: %s\n",
                    subscription->cnt, rd_kafka_err2str(err));
            rd_kafka_topic_partition_list_destroy(subscription);
            rd_kafka_destroy(rk);
            return 1;
    }

    LOG_MSG("%% Subscribed to %d topic(s), "
            "waiting for rebalance and messages...\n",
            subscription->cnt);

    rd_kafka_topic_partition_list_destroy(subscription);

    /* Subscribing to topics will trigger a group rebalance
     * which may take some time to finish, but there is no need
     * for the application to handle this idle period in a special way
     * since a rebalance may happen at any time.
     * Start polling for messages. */

    // assign connection to data_conn_ptr
    *connection = rk; 
    return 0;
}


/*!
 * @brief Closes kafka consumer.
 * @param[in] connection    Data connection pointer (void *)
 * @result 0 indicates success.
 */
int dataexchange_kafka_close(void **connection) {
    rd_kafka_t* rk = (rd_kafka_t*) (*connection);
    /* Close the consumer: commit final offsets and leave the group. */
    LOG_MSG("%s", "%% Closing consumer\n");
    rd_kafka_consumer_close(rk);

    /* Destroy the consumer */
    rd_kafka_destroy(rk);

    return 0;
}


/*! 
 * @brief Reads all available messages from the kafka server/topic and returns data in expected
 * geojson format. Loop is synchronous/blocking.
 * @param[in] connection          Data connection pointer (void *)
 * @param[in] max_payload_size    Max per-message size in char (int)
 * @param[in] message_block       Max number of messages (int)
 * @param[in] n_messages          Data connection pointer (int *)
 * @param[in] ierr                Error state (int *)
 */
char *dataexchange_kafka_getMessages(
    void **connection,
    const int max_payload_size,
    const int message_block,
    int *n_messages,
    int *ierr) {

    rd_kafka_t* rk = (rd_kafka_t*) (*connection);
    rd_kafka_message_t *rkm;
    int message_size = 0;
    char* msgs = NULL;
    *ierr = 0;
    *n_messages = 0;
    //LOG_MSG("START: max_payload_size (max chars in msg) = %d, message_block (n messages max) = %d", max_payload_size, message_block);

    msgs = (char *)malloc(max_payload_size * message_block * sizeof(char));

    while (true) {
      /* Timeout: no message within 100ms,
       *  return. This short timeout allows
       *  checking at frequent intervals.
       */
      rkm = rd_kafka_consumer_poll(rk, 200);
      if (!rkm) {
          LOG_MSG(" unpacked and copied %d messages", *n_messages);
          *ierr = 1;
          return NULL;
      }

      /* consumer_poll() will return either a proper message
       * or a consumer error (rkm->err is set). */
      if (rkm->err) {
          /* Consumer errors are generally to be considered
           * informational as the consumer will automatically
           * try to recover from all types of errors. */
          LOG_ERRMSG("%% Consumer error: %s\n",
                  rd_kafka_message_errstr(rkm));
          rd_kafka_message_destroy(rkm);
          continue;
      }

      /* Proper message. */
      // The function rd_kafka_message_leader_epoch() is only available for librdkafka versions
      // 2.1 or higher, currently not available for standard debian Docker containers (to bookworm)
      //printf("Message on %s [%" PRId32 "] at offset %" PRId64
      //       " (leader epoch %" PRId32 "):\n",
      //       rd_kafka_topic_name(rkm->rkt), rkm->partition,
      //       rkm->offset, rd_kafka_message_leader_epoch(rkm));

      /* Print the message key.
      if (rkm->key && is_printable(rkm->key, rkm->key_len)) {
          LOG_MSG(" Key: %.*s\n", (int)rkm->key_len, (const char *)rkm->key);
      } else if (rkm->key) {
          LOG_MSG(" Key: (%d bytes)\n", (int)rkm->key_len);
      } */

      /* Print the message value/payload.
      if (rkm->payload && is_printable(rkm->payload, rkm->len)) {
          LOG_MSG(" Value: %.*s\n", (int)rkm->len, (const char *)rkm->payload);
      } else if (rkm->payload) {
          LOG_MSG(" Value: (%d bytes)\n", (int)rkm->len);
      } */

      //LOG_MSG(" message char size %d", (int)rkm->len);
      /* Put payload into buffer for parsing */
      if ((int)rkm->len > max_payload_size) { // check char lengths
          LOG_WARNMSG(" Skipping message %d in queue, message too long (%d)",
              *n_messages, (int)rkm->len);
          rd_kafka_message_destroy(rkm);
          continue;
      }
      //LOG_MSG(" current message char size %d", *n_messages * max_payload_size);
      if (*n_messages + 1 > message_block) { // check char lengths
          LOG_ERRMSG(" Unread messages in queue, exceeded message limit (%d)",
              message_block);
          rd_kafka_message_destroy(rkm);
          break;
      }
      memcpy(&msgs[*n_messages * max_payload_size], (const char *)rkm->payload, rkm->len * sizeof(char));
      //LOG_MSG("%s\n", message_buffer);
      //LOG_MSG("%.1024s\n", &message_buffer[*n_messages * max_payload_size]);
      *n_messages += 1;

      rd_kafka_message_destroy(rkm);
    }

    LOG_MSG(" unpacked and copied %d messages", *n_messages);
    return msgs;
}

/*! 
 * @brief support function for readIni
static void setVarName(const char *group, const char *variable,
                       char *var) {
  memset(var, 0, 256*sizeof(char));
  sprintf(var, "%s:%s", group, variable);
  return;
}

 * @brief Reads the DataConn properties from the initialization file.
 *
 * @param[in] propfilename     Name of properties file.
 * @param[in] group            Group in ini file.  Likely "DataConn".
 *
 * @param[out] dataconn_props  Data Connection properties.
 *
 * @result 0 indicates success.
 *
int data_connection_readIni(const char *propfilename,
                     const char *group,
                     struct dataconn_props_struct* data_conn_props) {
  const char *s;
  char var[256];
  int ierr;
  dictionary *ini;
  ierr = 1;
  memset(data_conn_props, 0, sizeof(struct dataconn_props_struct));
  if (!os_path_isfile(propfilename)) {
    LOG_ERRMSG("%s: Properties file: %s does not exist\n",
		__func__, propfilename);
    return ierr;
  }
  ini = iniparser_load(propfilename);
  // Read the properties
  setVarName(group, "servers\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL) {
    LOG_ERRMSG("%s: Could not find Kafka servers string!\n", __func__);
    goto ERROR;
  } else {
    strcpy(data_conn_props->servers, s);
  }

  setVarName(group, "topic\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL) {
    LOG_ERRMSG("%s: Could not find Kafka topic string!\n", __func__);
    goto ERROR;
  } else {
    strcpy(data_conn_props->topic, s);
  }

  setVarName(group, "groupid\0", var);
  s = iniparser_getstring(ini, var, NULL);
  if (s == NULL) {
      LOG_ERRMSG("%s: Could not find Kafka groupid string!\n", __func__);
      goto ERROR;
  } else {
      strcpy(data_conn_props->groupid, s);
  }
  iniparser_freedict(ini);
  return ierr;
} */
