from flask import Flask, render_template, request
import paho.mqtt.client as mqtt
import mysql.connector
import time

app = Flask(__name__)

mqtt_broker = "192.168.98.105"
mqtt_port = 1883
topic1 = "topic/perangkat1"
topic3 = "topic/perangkat3"
avg_data = 0
previous_avg_data = 0  # Variabel untuk menyimpan avg_data sebelumnya

def run_query(query):
    connection = mysql.connector.connect(
        host='localhost',
        user='root',
        password='',
        database='m_c_potato'
    )
    
    cursor = connection.cursor()
    cursor.execute(query)
    result = cursor.fetchone()
    connection.close()
    
    return result

def on_publish(client, userdata, mid):
    print("Pesan berhasil terkirim")

# Inisialisasi MQTT client
mqtt_client = mqtt.Client()
mqtt_client.on_publish = on_publish
mqtt_client.connect(mqtt_broker, mqtt_port)

@app.route('/')
def index():
    global avg_data, previous_avg_data

    query1 = 'SELECT value FROM soil_moisture WHERE id = "sensor1" ORDER BY timestamp DESC LIMIT 1'
    data1 = run_query(query1)
    data1 = data1[0] if data1 is not None else 0

    query2 = 'SELECT value FROM soil_moisture WHERE id = "sensor2" ORDER BY timestamp DESC LIMIT 1'
    data2 = run_query(query2)
    data2 = data2[0] if data2 is not None else 0

    query3 = 'SELECT value FROM soil_moisture WHERE id = "sensor3" ORDER BY timestamp DESC LIMIT 1'
    data3 = run_query(query3)
    data3 = data3[0] if data3 is not None else 0

    query4 = 'SELECT value FROM soil_moisture WHERE id = "sensor4" ORDER BY timestamp DESC LIMIT 1'
    data4 = run_query(query4)
    data4 = data4[0] if data4 is not None else 0

    avg_data = float(data1 + data2 + data3 + data4) / 4
    avg_data = "{:.2f}".format(avg_data)

    query5 = 'SELECT value FROM water_level WHERE id = "sensor_ultrasonic" ORDER BY timestamp DESC LIMIT 1'
    data5 = run_query(query5)
    data5 = data5[0] if data5 is not None else 0

    if float(avg_data) > 50 and float(previous_avg_data) <= 50:
        mqtt_client.publish(topic1, 'OFF1')
        mqtt_client.publish(topic3, 'OFF3')
        mqtt_client.publish(topic3, 'OFF4')
    elif float(avg_data) <= 50 and float(previous_avg_data) > 50:
        mqtt_client.publish(topic1, 'ON1')
        mqtt_client.publish(topic3, 'ON3')
        mqtt_client.publish(topic3, 'ON4')
    elif float(avg_data) < 50:  # Kondisi baru untuk menyalakan status3 pada topik 3 jika rata-rata dibawah 50%
        mqtt_client.publish(topic3, 'ON3')

    previous_avg_data = avg_data

    return render_template('home.html', data1=data1, data2=data2, data3=data3, data4=data4, avg_data=avg_data, data5=data5)


@app.route('/control', methods=['GET', 'POST'])
def control():
    if request.method == 'POST':
        status1 = request.form.get('status1')
        status3 = request.form.get('status3')
        status4 = request.form.get('status4')
        
        if status1 == 'on':
            mqtt_client.publish(topic1, 'ON1')
        elif status1 == 'off':
            mqtt_client.publish(topic1, 'OFF1')

        if status3 == 'on':
            mqtt_client.publish(topic3, 'ON3')
        elif status3 == 'off':
            mqtt_client.publish(topic3, 'OFF3')

        if status4 == 'on':
            mqtt_client.publish(topic3, 'ON4')
        elif status4 == 'off':
            mqtt_client.publish(topic3, 'OFF4')

    return render_template('control.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True)
