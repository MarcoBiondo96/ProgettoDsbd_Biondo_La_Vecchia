
CREATE DATABASE dsbd;

USE dsbd;



CREATE TABLE IF NOT EXISTS Sla (ID INT AUTO_INCREMENT,PRIMARY KEY (ID), metric varchar(255), label varchar(255), sla_set TINYINT(1),range_min DOUBLE,range_max DOUBLE);


CREATE TABLE IF NOT EXISTS  metrics ( ID_metric INT AUTO_INCREMENT, metric varchar(255), max_1h DOUBLE, min_1h DOUBLE, avg_1h DOUBLE , max_3h DOUBLE, min_3h DOUBLE, avg_3h DOUBLE, max_12h DOUBLE, min_12h DOUBLE, avg_12h DOUBLE , dev_std_1h DOUBLE, dev_std_3h DOUBLE, dev_std_12h DOUBLE, pre_min DOUBLE, pre_max DOUBLE, pre_avg DOUBLE , stazionario TINYINT(1), autocorr TEXT, seasonal TEXT,violazione_1h INT,violazione_3h INT,violazione_12h INT,violazione_pre TINYINT(1),time_stamp TIMESTAMP , PRIMARY KEY (ID_metric), metric_id int, FOREIGN KEY (metric_id) REFERENCES Sla(ID));




INSERT INTO Sla (metric,label,sla_set,range_min,range_max) VALUES ('container_memory_cache',"{ 'container_label_com_docker_compose_config_hash':'f385fc0add1d8b0c83552e677825b867c418fabe736ac9e37b2ca9f5257748e1'}",False,0.0,0.0);


INSERT INTO Sla (metric,label,sla_set,range_min,range_max) VALUES ('container_memory_usage_bytes',"{ 'container_label_com_docker_compose_config_hash':'0f7a1d84f0c361a7039742066786300d6788ae924918299405ccbff872d505b8'}",False,0.0,0.0);


INSERT INTO Sla (metric,label,sla_set,range_min,range_max) VALUES ('container_fs_reads_bytes_total',"{ 'container_label_com_docker_compose_config_hash':'f385fc0add1d8b0c83552e677825b867c418fabe736ac9e37b2ca9f5257748e1'}",False,0.0,0.0);


INSERT INTO Sla (metric,label,sla_set,range_min,range_max) VALUES ('container_memory_working_set_bytes',"{ 'container_label_com_docker_compose_config_hash':'0f7a1d84f0c361a7039742066786300d6788ae924918299405ccbff872d505b8'}",False,0.0,0.0);


INSERT INTO Sla (metric,label,sla_set,range_min,range_max) VALUES ('container_memory_failures_total',"{ 'container_label_com_docker_compose_config_hash':'0f7a1d84f0c361a7039742066786300d6788ae924918299405ccbff872d505b8', 'scope':'container','failure_type':'pgfault'}",False,0.0,0.0);





