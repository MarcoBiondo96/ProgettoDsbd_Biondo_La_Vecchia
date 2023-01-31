from prometheus_api_client import PrometheusConnect, MetricsList, MetricSnapshotDataFrame, MetricRangeDataFrame
from datetime import timedelta
from matplotlib import pyplot 
import numpy
import statsmodels.api as sm
from statsmodels.tsa.seasonal import seasonal_decompose 
from pmdarima.arima import auto_arima, ADFTest
from prometheus_api_client.utils import parse_datetime
from statsmodels.graphics.tsaplots import plot_acf,plot_pacf
from confluent_kafka import Producer
import sys
from statsmodels.tsa.arima.model import ARIMA
from statsmodels.tsa.stattools import adfuller
from statsmodels.tsa.holtwinters import SimpleExpSmoothing,ExponentialSmoothing
from statsmodels.tools.eval_measures import rmse
import pandas as pd
import time
from math import isclose
import json
import datetime
import mysql.connector 
import ast
from datetime import datetime
from mysql.connector import errorcode
import os
import warnings 

warnings.filterwarnings("ignore")
kafka_server=(str(os.environ["KAFKA_SERVER"]))

db_server=(str(os.environ["DB_SERVER"]))
prom = PrometheusConnect(url="http://15.160.61.227:29090", disable_ssl=True)




def compute_time(start_time,control_type,metric_name):
    end_time=time.time()
    total_time_exe=end_time-start_time
    return metric_name+":Tempo esecuzione per "+control_type+" ="+str(total_time_exe)      



def get_metric(metric_name,start_t,label_config):
    label_config_=ast.literal_eval(label_config)
    start_time = parse_datetime(start_t)
    end_time = parse_datetime("now")
    chunk_size = timedelta(minutes=5) #sto dicendo che a partire da 12h fa fino ad ora, prendo pezzi di dati a 5 a 5 minuti.
    metric_range_data = prom.get_metric_range_data(
        metric_name=metric_name,
        label_config=label_config_,
        start_time=start_time,
        end_time=end_time,
        chunk_size=chunk_size,
    )
    return metric_range_data

def adfuller_test(isStationary):
    result=adfuller(isStationary)
    if result[1] <= 0.05:
        return True
        #print("strong evidence against the null hypothesis(Ho), reject the null hypothesis. Data is stationary. Dunque d=0 e posso usare il metodo ARIMA")
        #vai a funzione per ARIMA
        
    else:
        return False
        #print("weak evidence against null hypothesis,indicating it is non-stationary. Uso SimpleExpSmoothing per le predizioni. Processo: ",i)#invece di rendere stazionario il dato usando le differenze (che vogliamo evitare), cambio il metodo di predizione ed uso smoothing
        #vai a funzione per expsmoothing
        
        


    

class tipo_arima:
    def __init__(self,p,q,errore):
        self.p=p
        self.q=q
        self.errore=errore

def arima(forPrediction):
    forPrediction=forPrediction.resample(rule='T').mean()
    train = forPrediction.iloc[0:len(forPrediction)-10]
    test = forPrediction.iloc[len(forPrediction)-10:]
    errori=[]
    start_ts=len(train)
    end_ts=len(train)+len(test)-1
    for p in range(7):
        for q in range(7):
                model=ARIMA(train,order=(p,0,q))
                results=model.fit()
                predictions = results.predict(start=start_ts,end=end_ts,dynamic=False,typ="levels")
                error = rmse(test, predictions)
                
                arima_object=tipo_arima(p,q,error)
                errori.append(arima_object)
    oggettominimo=tipo_arima(0,0,0.00000000)
    for i in range(len(errori)):
        if(errori[i].errore<oggettominimo.errore or i==0):
            oggettominimo=errori[i]

    model=ARIMA(train,order=(oggettominimo.p,0,oggettominimo.q))
    results=model.fit()
    predictions = results.predict(start=start_ts,end=end_ts+9,typ="levels")
    return predictions

def exponential_smoothing(forPrediction):
    forPrediction=forPrediction.resample(rule='T').mean()
    model=ExponentialSmoothing(forPrediction,trend="additive",seasonal="mul",seasonal_periods=12)
    model=model.fit()
    predictions=model.predict(start=len(forPrediction),end=len(forPrediction)+9)
    return predictions   




num_metric=["1h","3h","12h"]

while True:
    
    try:
        myresult=[]
        mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
    except mysql.connector.Error as err:
      if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
        print("Something is wrong with your user name or password")
      elif err.errno == errorcode.ER_BAD_DB_ERROR:
        print("Database does not exist")
      
    else:
        mycursor = mydb.cursor(dictionary=True)
        mycursor.execute("SELECT * FROM Sla;")
        myresult = mycursor.fetchall()     
        mycursor.close()

    for i in myresult:
        print("Inizio Logs:")
        dict_dati={}
        dict_dati["metric"]=i["metric"]
        dict_dati["metric_id"]=str(i["ID"])
        timestamp = datetime.now()
        
        dict_dati["timestamp"]=str(timestamp)
        
        for j in num_metric:
            
            start_time=time.time()
            metric_range_data_df=MetricRangeDataFrame(get_metric(i["metric"],j,i["label"]))
            print(compute_time(start_time,"prelievo dati da promethues "+j,i["metric"]))
            vm=metric_range_data_df['value']
            if(i["sla_set"]):
                incr=0
                for med in vm:
                    if (med<i["range_min"] or med>i["range_max"]):
                            incr=incr+1
                dict_dati["violazione_"+j]=str(incr)
            else:
                dict_dati["violazione_"+j]=0
            if(j=="1h"): 
                ######################AUTOCORRELAZIONE##################################
                start_time=time.time()
                media=vm.mean()
                varianza=vm.var()
                dati_normalizzati=metric_range_data_df['value']-media
                acorr=numpy.correlate(dati_normalizzati,dati_normalizzati,'full')[len(dati_normalizzati)-1:]
                acorr = acorr / varianza / len(dati_normalizzati)
                print(compute_time(start_time,"calcolo autocorrelazione ",i["metric"]))
                acorr = pd.Series(acorr)
                c=acorr.to_string(index=False)
                arr = c.split()
                accdic={}
                z=0
                for ar in arr:
                    accdic["value"+str(z)]=ar
                    z=z+1
                dict_dati["autocorr"]=accdic   
                start_time=time.time()
                sd=seasonal_decompose(vm,model="additive",period=6)
                print(compute_time(start_time,"calcolo seasonal ",i["metric"]))
                fg=sd.seasonal.index
                fg=fg.to_series().to_string(index=False)
                fg=fg.replace('timestamp','')
                fg=fg.replace(' ','')
                fg = fg.split()
                arr=sd.seasonal.to_string(index=False)
                arr=arr.replace('timestamp','')
                arr = arr.split()
                dictionary = dict(zip(fg, arr))
                dict_dati["seasonal"]=dictionary
                start_time=time.time()
                stazionario=adfuller_test(vm.dropna())
                print(compute_time(start_time,"calcolo stazionarieta' ",i["metric"]))
                dict_dati["stazionario"]=str(stazionario)
                
                if(i["sla_set"]):
                    if(stazionario):
                        start_time=time.time()
                        predizione=arima(vm.dropna())
                        print(compute_time(start_time,"calcolo predizione ARIMA ",i["metric"]))
                        dict_dati["pre_min"]=str(predizione.min())
                        dict_dati["pre_max"]=str(predizione.max())
                        dict_dati["pre_avg"]=str(predizione.mean())
                        if(predizione.min()<i["range_min"] or predizione.max()>i["range_max"]):
                            dict_dati["violazione_pre"]=str(True)
                        else:
                            dict_dati["violazione_pre"]=str(False)
                    else:
                        start_time=time.time()
                        predizione=exponential_smoothing(vm.dropna())
                        print(compute_time(start_time,"calcolo predizione exponential_smoothing ",i["metric"]))
                        dict_dati["pre_min"]=str(predizione.min())
                        dict_dati["pre_max"]=str(predizione.max())
                        dict_dati["pre_avg"]=str(predizione.mean())
                        if(predizione.min()<i["range_min"] or predizione.max()>i["range_max"]):
                            dict_dati["violazione_pre"]=str(True)
                        else:
                            dict_dati["violazione_pre"]=str(False)
                else:
                    dict_dati["pre_min"]=str(0.0)
                    dict_dati["pre_max"]=str(0.0)
                    dict_dati["pre_avg"]=str(0.0)
                    dict_dati["violazione_pre"]=str(False)
            start_time=time.time()
            dict_dati["max_"+j]=str(vm.max())
            dict_dati["min_"+j]=str(vm.min())
            dict_dati["avg_"+j]=str(vm.mean())
            dict_dati["dev_std_"+j]=str(vm.std())
            print(compute_time(start_time,"calcolo min,max,avg,dev_std "+j,i["metric"]))
        
        result = str(dict_dati)
        conf = {'bootstrap.servers': kafka_server+':29092'}
        topic="promethuesdata"
        # Create Producer instance
        p = Producer(**conf)
        p.produce(topic, result)
        p.poll()
      
    