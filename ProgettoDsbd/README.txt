Struttura Progetto:

.
├── docker-compose.yml
└──dump
	├── data_retrival
	│	├── Dockerfile
	│	├── microServ3.py
	│	└── requirements.txt
	├── data_storage
	│	├── Dockerfile
	│	├── microServ2.py
	│	└──requirements.txt
	├── etl
	│	├── Dockerfile
	│	├── microServ1.py
	│	└── requirements.txt
	├── mysql
	│	├── Dockerfile
	│	└── initdb.sql
	└── sla_manager
		├── Dockerfile
		├── microServ4.py
		└── requirements.txt
		
docker-compose.yml:

version: '3.1'

services:

    db:
        build: ./dump/mysql
        container_name: db
        stdin_open: true
        tty: true
        container_name: mysql
        
        ports:
            - 3306:3306
        environment:
            - MYSQL_ROOT_PASSWORD=root
        volumes:
            - ./dump/mysql:/docker-entrypoint-initdb.d
            
        
          
    zookeeper:
        image: confluentinc/cp-zookeeper:latest
        environment:
          ZOOKEEPER_CLIENT_PORT: 2181
          ZOOKEEPER_TICK_TIME: 2000
    kafka:
    
        image: confluentinc/cp-kafka:latest
        container_name: kafka
        depends_on:
            - zookeeper
        ports:
            - 29092:29092
        
        environment:
            KAFKA_BROKER_ID: 1
            KAFKA_ZOOKEEPER_CONNECT: zookeeper:2181
            KAFKA_ADVERTISED_LISTENERS: PLAINTEXT://kafka:29092,PLAINTEXT_HOST://localhost:9092
            KAFKA_LISTENER_SECURITY_PROTOCOL_MAP: PLAINTEXT:PLAINTEXT,PLAINTEXT_HOST:PLAINTEXT
            KAFKA_INTER_BROKER_LISTENER_NAME: PLAINTEXT
            KAFKA_OFFSETS_TOPIC_REPLICATION_FACTOR: 1
        command: sh -c "((sleep 17 && kafka-topics --create --bootstrap-server localhost:29092 --if-not-exists --replication-factor 1 --partitions 1 --topic promethuesdata)&) && /etc/confluent/docker/run "
        
    etl_data_pipeline:
        build: ./dump/etl
        depends_on:
            - kafka
            - db
        environment:
      
            - KAFKA_SERVER=kafka
            - DB_SERVER=db
            
    data_retrival:
        build: ./dump/data_retrival
        ports:
            - "5000:5000"
            
        depends_on:
            - db
        environment:
      
            - DB_SERVER=db
    
    data_storage:
        build: ./dump/data_storage
        depends_on:
            - kafka
            - db
        environment:
            - KAFKA_SERVER=kafka
            - DB_SERVER=db       

    sla_manager:
        build: ./dump/sla_manager
        ports:
            - "5001:5001"
            
        depends_on:
            - db
        environment:
      
            - DB_SERVER=db
			
Il file compose definisce un applicazione che presenta 7 microServizi tra cui un db, zookeeper, kafka, e i 4 microServizi richiesti per l'elaborato.
Quando l'applicazione sarà deployata, il docker compose mappera le porte 5000 e 5001 dei container con quelle del host.
Si assicuri che tali porte del host non siano usate da altre applicazioni.
Per accedere al microServizio3 si dovrà andare al seguente indirizzo http://localhost:5000 mentre per il microServizio4 al http://localhost:5001 .

Deploy con docker compose:

Andare nella cartella ProgettoDsbd e tramite prompt digitare il seguente comando: 

docker-compose up 

Risultati del docker-compose:

docker-compose ps

NAME                               COMMAND                  SERVICE             STATUS              PORTS
kafka                              "sh -c '((sleep 17 &…"   kafka               running             9092/tcp, 0.0.0.0:29092->29092/tcp
mysql                              "docker-entrypoint.s…"   db                  running             0.0.0.0:3306->3306/tcp, 33060/tcp
progettodsbd-data_retrival-1       "python ./microServ3…"   data_retrival       running             0.0.0.0:5000->5000/tcp
progettodsbd-data_storage-1        "python ./microServ2…"   data_storage        running
progettodsbd-etl_data_pipeline-1   "python -u ./microSe…"   etl_data_pipeline   running
progettodsbd-sla_manager-1         "python ./microServ4…"   sla_manager         running             0.0.0.0:5001->5001/tcp
progettodsbd-zookeeper-1           "/etc/confluent/dock…"   zookeeper           running             2181/tcp, 2888/tcp, 3888/tcp



Utilizza questo comando per fermare e cancellare il docker-compose:

docker-compose down