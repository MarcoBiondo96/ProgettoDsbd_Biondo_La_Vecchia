from confluent_kafka import Consumer
import json
import ast
import mysql.connector
import os
kafka_server=(str(os.environ["KAFKA_SERVER"]))

db_server=(str(os.environ["DB_SERVER"]))

def str2bool(v):
  return v.lower() in ( "true")
c = Consumer({
    'bootstrap.servers': kafka_server+':29092',
    'group.id': 'groupdsbd',
    'auto.offset.reset': 'earliest'  # 'auto.offset.reset=earliest' to start reading from the beginning
})

c.subscribe(['promethuesdata'])
total_count = 0
try:
        while True:
            msg = c.poll(1.0)
            if msg is None:
                # No message available within timeout.
                # Initial message consumption may take up to
                # session.timeout.ms for the consumer group to
                # rebalance and start consuming
                print("Waiting for message or event/error in poll()")
                continue
            elif msg.error():
                print('error: {}'.format(msg.error()))
            else:
                # Check for Kafka message
                
                record_key = msg.key()
                record_value = msg.value()
                
                record_value=record_value.decode("utf-8")
                dict_dati=ast.literal_eval(record_value)
                mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
                mycursor = mydb.cursor(dictionary=True)
                sql = """INSERT INTO metrics (metric, max_1h, min_1h, avg_1h  , dev_std_1h ,max_3h , min_3h , avg_3h ,dev_std_3h , max_12h , min_12h , avg_12h  , dev_std_12h , pre_min , pre_max , pre_avg  , stazionario , autocorr , seasonal ,violazione_1h ,violazione_3h ,violazione_12h ,violazione_pre ,time_stamp  ,  metric_id ) VALUES (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s);"""
                val = (dict_dati["metric"], float(dict_dati["max_1h"]), float(dict_dati["min_1h"]), float(dict_dati["avg_1h"]), float(dict_dati["dev_std_1h"]), float(dict_dati["max_3h"]), float(dict_dati["min_3h"]), float(dict_dati["avg_3h"]), float(dict_dati["dev_std_3h"]), float(dict_dati["max_12h"]), float(dict_dati["min_12h"]), float(dict_dati["avg_12h"]), float(dict_dati["dev_std_12h"]), float(dict_dati["pre_min"]), float(dict_dati["pre_max"]), float(dict_dati["pre_avg"]), str2bool(dict_dati["stazionario"]), str(dict_dati["autocorr"]), str(dict_dati["seasonal"]), int(dict_dati["violazione_1h"]), int(dict_dati["violazione_3h"]), int(dict_dati["violazione_12h"]), str2bool(dict_dati["violazione_pre"]), dict_dati["timestamp"], int(dict_dati["metric_id"]))
                mycursor.execute(sql,val)
                mydb.commit()
                mycursor.close()
                print(mycursor.rowcount, "was inserted.")

except KeyboardInterrupt:

    pass
finally:
    # Leave group and commit final offsets
    c.close()