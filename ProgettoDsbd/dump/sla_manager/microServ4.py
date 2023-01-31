from flask import Flask , render_template, render_template_string, request,redirect, url_for
import mysql.connector 
from mysql.connector import errorcode
import ast
import io
import base64
import numpy

app=Flask(__name__)

import os

db_server=(str(os.environ["DB_SERVER"]))


@app.route('/', methods=['GET'])
def home():
    mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
    mycursor = mydb.cursor(dictionary=True)
    mycursor.execute("select * from Sla order by sla_set DESC;")
    myresult = mycursor.fetchall()
    mycursor.close()
    i=0
    for res in myresult:
        if(res["sla_set"]==1):
            i=i+1
    
    
    template='''<!DOCTYPE html>
<html>
    <head>
        <link rel="stylesheet" href="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css">
    </head>
    <body >
<div class="container">
   <br />
   <h2 align="center">Metriche Disponibili</h2>
   <h4 align="center">Numero Metriche Sla Set {{num_s}}/5</h4>
   <div class="row">
    <div class="col-md-12">
        <div class="panel-body">
            <div class="table-responsive">
                <table class="table table-bordered">
                    <thead>
                        <tr>
                            
                            <th>Nome Metrica</th>
                            <th>Label</th>
                            <th>Sla Set</th>
                            <th>Range Min</th>
                            <th>Range Max</th>
                            <th>Operazioni Sulle Metriche</th>
                        </tr>
                        </thead> 
                        {% for row in result %}
                            <tr>
                                <td>{{row.metric}}</td>
                                <td>{{row.label}}</td>
                                <td>{{row.sla_set}}</td>
                                <td>{{row.range_min}}</td>
                                <td>{{row.range_max}}</td>
                                <td>
                                {%if num_s<5 or row.sla_set==1 %} 
                                <button onclick="location.href = '/update/{{row.ID}}';" id="myButton" class="userinfo btn btn-success" >Update</button>
                                {% endif %}
                                {%if row.sla_set==1 %} 
                                    <button onclick="location.href = '/remove/{{row.ID}}';" id="myButton" class="userinfo btn btn-success" >Remove</button>
                                    <button onclick="location.href = '/n_violazioni/{{row.ID}}';" id="myButton" class="userinfo btn btn-success" >Numero Violazioni</button>
                                    <button onclick="location.href = '/p_violazioni/{{row.ID}}';" id="myButton" class="userinfo btn btn-success" >Possibili Violazioni</button></td>
                                  {% endif %}
                                  
                                
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
    return render_template_string(template, result=myresult,num_s=i)
    

@app.route('/Update_', methods =["POST"])
def gfg():
    range_min = 0.0
    range_max = 0.0
    if request.method == "POST":
        id_=request.form.get("ID")
        range_min = request.form.get("range_min")
        range_max = request.form.get("range_max")
        mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
        mycursor = mydb.cursor(dictionary=True)
        sql="Update Sla Set sla_set=1, range_min="+str(range_min)+" ,range_max="+str(range_max)+" Where ID="+str(id_)+";"
        mycursor.execute(sql)
        mydb.commit()
        mycursor.close()
        return redirect(url_for('home'))
    return "ciao"
    

        
@app.route('/<name>/<metric>')
def my_view(name,metric):  
    template='''<h1>Errore</h1>'''
    if(name=="update"):
        mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
        mycursor = mydb.cursor(dictionary=True)
        mycursor.execute("select * from Sla order by sla_set DESC;")
        myresult = mycursor.fetchall()
        mycursor.close()
        i=0
        
        c={}
        for res in myresult:
            
            if(res["sla_set"]==1):
                i=i+1
            if(res["ID"]==int(metric)):
                c=res
        if(i<5 or c["sla_set"]==1):
            template='''<h1>Inserisci range di valori ammessi per la Metrica : {{result["metric"]}}</h1>
<form action="{{ url_for("gfg")}}" method="post">
<input type="hidden" id="ID" name="ID" value="{{result["ID"]}}" readonly><br><br>
<h3>Valori Attuali</h3>
<label for="Range_min">Range_min:</label>
<input type="text" id="range_min" name="range_min" value="{{result["range_min"]}}">
<label for="range_max">Range_max:</label>
<input type="text" id="range_max" name="range_max" value="{{result["range_max"]}}">
<button type="submit">Update</button>'''
        return render_template_string(template,result=c)
    if(name=="remove"):
        mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
        mycursor = mydb.cursor(dictionary=True)
        sql="Update Sla Set sla_set=0, range_min=0.0 ,range_max=0.0 Where ID="+str(metric)+";"
        mycursor.execute(sql)
        mydb.commit()
        mycursor.close()
        return redirect(url_for('home'))
    if(name=="n_violazioni"):
        mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
        mycursor = mydb.cursor(dictionary=True)
        sql="Select metric,violazione_1h,violazione_3h,violazione_12h,time_stamp from metrics Where metric_id="+str(metric)+" order by time_stamp DESC ;"
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
   <h2 align="center">Violazioni Metrica </h2>
   
   <div class="row">
    <div class="col-md-12">
        <div class="panel-body">
            <div class="table-responsive">
                <table class="table table-bordered">
                    <thead>
                        <tr>
                            
                            <th>Nome Metrica</th>
                            <th>Violazione_1h</th>
                            <th>Violazione_3h</th>
                            <th>Violazione_12h</th>
                            <th>TimeStamp</th>
                            
                        </tr>
                        </thead> 
                        {% for row in result %}
                            <tr>
                                <td>{{row.metric}}</td>
                                <td>{{row.violazione_1h}}</td>
                                <td>{{row.violazione_3h}}</td>
                                <td>{{row.violazione_12h}}</td>
                                <td>{{row.time_stamp}}</td>
                               
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
        return render_template_string(template,result=myresult)
    if(name=="p_violazioni"):
        mydb = mysql.connector.connect(host=db_server,user="root",password="root",  database="dsbd")
        mycursor = mydb.cursor(dictionary=True)
        sql="Select metric,violazione_pre,time_stamp from metrics Where metric_id="+str(metric)+" order by time_stamp DESC ;"
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
   <h2 align="center">Violazioni Metrica </h2>
   
   <div class="row">
    <div class="col-md-12">
        <div class="panel-body">
            <div class="table-responsive">
                <table class="table table-bordered">
                    <thead>
                        <tr>
                            
                            <th>Nome Metrica</th>
                            <th>Violazione_Predette</th>
                            
                            <th>TimeStamp</th>
                            
                        </tr>
                        </thead> 
                        {% for row in result %}
                            <tr>
                                <td>{{row.metric}}</td>
                                
                                {% if(row.violazione_pre) %}
                                
                                <td>Si</td>
                                
                                {% endif %}
                                {% if(not(row.violazione_pre)) %}
                                
                                <td>No</td>
                                
                                {% endif %}
                                <td>{{row.time_stamp}}</td>
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
        return render_template_string(template,result=myresult)
        
        
    return template;
if __name__== '__main__':
    app.run(host="0.0.0.0",port=5001 ,debug=True)
    