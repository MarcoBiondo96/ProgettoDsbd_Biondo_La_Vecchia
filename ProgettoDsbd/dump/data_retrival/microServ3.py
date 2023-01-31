from flask import Flask , render_template, render_template_string, request
import mysql.connector 
from mysql.connector import errorcode
import ast
import io
import base64
import numpy
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure

app=Flask(__name__)

import pandas as pd
import os

db_server=(str(os.environ["DB_SERVER"]))

@app.route('/', methods=['GET'])
def home():
    try:
    
        mydb = mysql.connector.connect(host=db_server,user="root",password="root", port="3306" ,database="dsbd")
    except mysql.connector.Error as err:
      if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
        return("Something is wrong with your user name or password")
      elif err.errno == errorcode.ER_BAD_DB_ERROR:
        return("Database does not exist")
      else:
        return str(err)
    else:
        mycursor = mydb.cursor(dictionary=True)
        mycursor.execute("SELECT * FROM Sla;")
        myresult = mycursor.fetchall()     
        mycursor.close()
        template='''<!DOCTYPE html>
<html>
    <head>
        <link rel="stylesheet" href="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css">
    </head>
    <body >
<div class="container">
   <br />
   <h3 align="center">Metriche Disponibili</h3>
   <div class="row">
    <div class="col-md-12">
        <div class="panel-body">
            <div class="table-responsive">
                <table class="table table-bordered">
                    <thead>
                        <tr>
                            
                            <th>Nome Metrica</th>
                            <th>Label</th>
                            <th>Seleziona Metrica Da Visualizzare</th>
                        </tr>
                        </thead> 
                        {% for row in result %}
                            <tr>
                                <td>{{row.metric}}</td>
                                <td>{{row.label}}</td>
                                <td><button onclick="location.href = '/{{row.ID}}';" id="myButton" class="userinfo btn btn-success" >Visualizza</button></td>
                            </tr>
                        {% endfor %}
                </table>
            </div>
        </div>    
    </div>    
    </div>
</div>
        
    </body>
</html>
'''
        return render_template_string(template, result=myresult)

@app.route('/<name>/<metric>')
def my_view(name,metric):
    mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
    mycursor = mydb.cursor(dictionary=True)
    if(metric=="seasonal"):
        sql="Select metric_id,seasonal from metrics where Id_metric="+str(name)+";"
        mycursor.execute(sql)
        myresult = mycursor.fetchall()
        mycursor.close()
        d=""
        p=0
        for asa in myresult:
            p=asa["metric_id"]
            fig = Figure()
            res = ast.literal_eval(asa["seasonal"])
            serie=pd.Series(res)
            axis = fig.add_subplot(1, 1, 1)
            axis.locator_params(axis='y',tight=True, nbins=2)
            axis.grid()
            c=[]
            for ada in serie.values:
                c.append(float(ada))

            axis.set_title("Seasonal")
            axis.set_xlabel("Campioni")
            axis.set_ylabel("Valori")
            axis.plot(c)
            pngImage = io.BytesIO()
            FigureCanvas(fig).print_png(pngImage)
            pngImageB64String = "data:image/png;base64,"
            pngImageB64String += base64.b64encode(pngImage.getvalue()).decode('utf8')
            d=pngImageB64String
        template='''<h1>Stagionalità</h1>
        <img src="{{ image }}"/>
        <button onclick="location.href = '/{{ metric }}';" id="myButton" class="userinfo btn btn-success" >Go Back</button>
        '''
        return render_template_string(template, image=d,metric=p)
        
    if(metric=="autocorr"):
        sql="Select metric_id,autocorr from metrics where Id_metric="+str(name)+";"
        mycursor.execute(sql)
        myresult = mycursor.fetchall()
        mycursor.close()
        d=""
        p=0
        for asa in myresult:
            p=asa["metric_id"]
            fig = Figure()
            res = ast.literal_eval(asa["autocorr"])
            serie=pd.Series(res)
            axis = fig.add_subplot(1, 1, 1)
            axis.locator_params(axis='y',tight=True, nbins=2)
            axis.grid()
            c=[]
            for ada in serie.values:
                c.append(float(ada))

            axis.set_title("Autocorrelazione")
            axis.set_xlabel("Campioni")
            axis.set_ylabel("Valori")
            axis.plot(c)
            pngImage = io.BytesIO()
            FigureCanvas(fig).print_png(pngImage)
            pngImageB64String = "data:image/png;base64,"
            pngImageB64String += base64.b64encode(pngImage.getvalue()).decode('utf8')
            d=pngImageB64String
        template='''<h1>Autocorrelazione</h1>
        <img src="{{ image }}"/>
        <button onclick="location.href = '/{{ metric }}';" id="myButton" class="userinfo btn btn-success" >Go Back</button>
        '''
        return render_template_string(template, image=d,metric=p)
        
        
    return "<h1><center>Errore</center></h1>"
    
@app.route('/<name>')
def my_view_func(name):
    mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
    mycursor = mydb.cursor(dictionary=True)
    sql="Select * from metrics where metric_id="+str(name)+" Order by time_stamp DESC;"
    mycursor.execute(sql)
    myresult = mycursor.fetchall()
    mycursor.close()

    template='''<!DOCTYPE html>
<html>
    <head>
        <link rel="stylesheet" href="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css">
    </head>
    <body >
<div class="container">
   <br />
   <h3 align="center">Metriche Disponibili</h3>
   <div class="row">
    <div class="col-md-12">
        <div class="panel-body">
            <div class="table-responsive">
                <table class="table table-bordered">
                    <thead>
                        <tr>
                            
                            <th>Nome Metrica</th>
                            <th>TimeStamp</th>
                            <th>Max_1h</th>
                            <th>Avg_1h</th>
                            <th>Min_1h</th>
                            <th>Dev_std_1h</th>
                            <th>Max_3h</th>
                            <th>Avg_3h</th>
                            <th>Min_3h</th>
                            <th>Dev_std_3h</th>
                            <th>Max_12h</th>
                            <th>Avg_12h</th>
                            <th>Min_12h</th>
                            <th>Dev_std_12h</th>
                            <th>Pre_min</th>
                            <th>Pre_max</th>
                            <th>Pre_avg</th>
                            <th>Stazionario</th>
                            
                            <th>Stagionalità</th>
                            <th>Autocorrelazione</th>
                            
                        </tr>
                        </thead> 
                        {% for row in result %}
                            <tr>
                                <td><b>{{row.metric}}</b></td>
                                <td>{{row.time_stamp}}</td>
                                <th>{{row.max_1h}}</th>
                                <th>{{row.avg_1h}}</th>
                                <th>{{row.min_1h}}</th>
                                <th>{{row.dev_std_1h}}</th>
                                <th>{{row.max_3h}}</th>
                                <th>{{row.avg_3h}}</th>
                                <th>{{row.min_3h}}</th>
                                <th>{{row.dev_std_3h}}</th>
                                <th>{{row.max_12h}}</th>
                                <th>{{row.avg_12h}}</th>
                                <th>{{row.min_12h}}</th>
                                <th>{{row.dev_std_12h}}</th>
                                <th>{{row.pre_min}}</th>
                                <th>{{row.pre_max}}</th>
                                <th>{{row.pre_avg}}</th>
                                
                                {% if(row.stazionario) %}
                                
                                <td>Si</td>
                                
                                {% endif %}
                                {% if(not(row.stazionario)) %}
                                
                                <td>No</td>
                                
                                {% endif %}
                                <td><button onclick="location.href = '/{{row.ID_metric}}/seasonal';" id="myButton" class="userinfo btn btn-success" >Visualizza</button></td>
                                <td><button onclick="location.href = '/{{row.ID_metric}}/autocorr';" id="myButton" class="userinfo btn btn-success" >Visualizza</button></td>        
                        </tr>
                        {% endfor %}
                </table>
            </div>
        </div>    
    </div>    
    </div>
    <button onclick="location.href = '/';" id="myButton" class="userinfo btn btn-success" >Go Back</button>
</div>
      
    </body>
</html>
'''
    return render_template_string(template, result=myresult)
    
    
if __name__== '__main__':
    app.run(host="0.0.0.0", debug=True)
    