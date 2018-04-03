# -*- coding: utf-8 -*-

import time
import json
import urllib
from urllib import parse,request
import requests
import os
import xml.etree.ElementTree as ET
import socket
import threading
import datetime
from my_serial import MySerial
from weChatApp import WeiXinClient,FileCache

#import traceback

#The terminal_ID should be unique.
terminal_ID = '7777'
first_time = 0
timeout = 5
socket.setdefaulttimeout(timeout)

global serial
global sensor_data
serial = None
sensor_data = {
    'update':False,
    't温度':'未更新',
    's湿度':'未更新',
    'i光照':'未更新',
}

cmd_info={
    'dkgz':'灯光开关',
    'gbgz':'空调开关',
    'js':'窗帘开关',
    'dskg':'电视开关',
}





def clientTask():
    global serial
    wechat_client = WeiXinClient('wx270e18185d243d54', 'd824322379b3420848c46e0f320faf7e')
    wechat_client.request_access_token()
    connect_flag = False
    while not connect_flag:
        try:
            conn = socket.socket()
            print('connect ... ')
            conn.connect(("60.205.189.203", 7777))
            conn.sendall(bytes('{"heartbeat":"%s"}' % terminal_ID, encoding='utf-8'))
            connect_flag = True
        except Exception as e:
            print(e)
    connected = True
    beat_time = time.time() + timeout
    last_upload_time = 0
    while connected:
        try:
            try:
                ret_bytes = None
                ret_bytes = conn.recv(1024)
            except Exception as e:
                print('socket:', e)
            if ret_bytes:
                set_str = ret_bytes.decode()
                print(set_str)
                rjson = None
                set_list =  set_str.split('}')
                for jsonstr in set_list:
                    if jsonstr:
                        jsonstr = jsonstr + '}'
                    else :
                        continue
                    print(jsonstr)
                    try:
                        rjson = json.loads(jsonstr)
                    except Exception as e:
                        print('json.loads error:', e)
                    if rjson:
                        if 'cmd' in rjson.keys():
                            if rjson['cmd'] == 'pzzt':
                                if serial.isOpened or sensor_data['update']:
                                    msg = '传感器在线。\\n温度:%s ℃\\n湿度:%s\\n光照强度:%s'%(sensor_data['t温度'],sensor_data['s湿度'],sensor_data['i光照'])
                                    print(msg)
                                    conn.sendall(bytes('{"msg":"text","text":"%s"}'%msg, encoding='utf-8'))
                                else:
                                    conn.sendall(bytes('{"msg":"text","text":"家庭主控在线，但传感器未连接。"}',encoding='utf-8'))
                            elif rjson['cmd'] == 'czts':
                                sztime = int(time.time()) - first_time
                                day = 24*60*60
                                hour = 60*60
                                minute = 60
                                if sztime > day:
                                    reply = '%d天%d时%d分%d秒'%(sztime/day,(sztime%day)/hour,(sztime%hour)/minute,sztime%60)
                                elif sztime > hour:
                                    reply = '%d时%d分%d秒'%(sztime/hour,(sztime%hour)/minute,sztime%60)
                                elif sztime > minute:
                                    reply = '%d分%d秒'%(sztime/minute,sztime%60)
                                else:
                                    reply = '%d秒'%(sztime%60)
                                conn.sendall(bytes('{"msg":"text","text":"您已使用智能家居%s。"}'%reply, encoding='utf-8'))
                            elif rjson['cmd'] == 'ykyd':
                                if serial.isOpened:
                                    conn.sendall(bytes('{"msg":"on"}', encoding='utf-8'))
                                else:
                                    conn.sendall(bytes('{"msg":"off"}',encoding='utf-8'))
                            elif rjson['cmd'] == 'pztx':
                                try:
                                    #from VideoCapture import Device
                                    #cam = Device(1)
                                    #cam.saveSnapshot('test.jpg',1,0,'br')
                                    #del cam

                                    #--no-banner
                                    #--no-timestamp
                                    #Intelligent potting plant
                                    #-s contrast=50%
                                    #--skip 10
                                    min_upload_time_interval = 3.0
                                    if time.time() - last_upload_time > min_upload_time_interval:
                                        last_upload_time = time.time()
                                        time_str = str(datetime.datetime.fromtimestamp(time.time()+28800)).replace(' ','-')[:-7]
                                        print(time_str)
                                        #cmd = '''sudo fswebcam -d /dev/video0 -r 640x480 --bottom-banner --title "Intelligent potting plant" --timestamp "%s" --save test.jpg -s brightness=100%%  --jpeg 95 --skip 10'''%(time_str)
                                        #os.system(cmd)#>/dev/null 2>&1
                                        os.system('cp -f test.jpg upload/%s.jpg' % time_str)
                                        media_id = wechat_client.get_media_id('image', 'test.jpg', True)
                                    else:
                                        print('Upload time interval less then %d s'%min_upload_time_interval)
                                        media_id = wechat_client.get_media_id('image', 'test.jpg', False)
                                    if media_id:
                                        print(media_id)
                                        conn.sendall(bytes('{"msg":"image","image":"%s"}'%media_id,encoding = 'utf-8'))
                                        #os.system('rm -f test.jpg')
                                        os.system('mv -f test.jpg test_last.jpg')
                                    else:
                                        conn.sendall(bytes('{"msg":"text","text":"家庭主控在线，但摄像头未连接。"}', encoding='utf-8'))
                                except Exception as e:
                                    print('cam error: ',e)
                                    conn.sendall(bytes('{"msg":"text","text":"家庭主控在线，但摄像头未连接。"}',encoding= 'utf-8'))
                            elif serial.isOpened:
                                if serial.sendCmdPack(rjson):
                                    if rjson['cmd'] in cmd_info.keys():
                                        conn.sendall(bytes('{"msg":"text","text":"家庭主控在线，已向控制器发送%s命令。"}'%cmd_info[rjson['cmd']], encoding='utf-8'))
                                else:
                                    pass
                                    #move cmd
                                    #conn.sendall(bytes('{"msg":"text","text":"家庭主控在线，系统没有此命令。"}',encoding='utf-8'))
                            else:
                                conn.sendall(bytes('{"msg":"text","text":"家庭主控在线，但控制器未连接。"}', encoding='utf-8'))
                    else:
                        print(None)
            if beat_time < time.time():
                conn.sendall(bytes('{"heartbeat":"%s"}' % terminal_ID, encoding='utf-8'))
                beat_time = time.time() + timeout
                try:
                    os.system('mv -f test.jpg test_last.jpg')
                    time_str = time.ctime(time.time() + 28800)
                    cmd = '''sudo fswebcam -d /dev/video0 -r 1920x1080 --bottom-banner --title "Higor Smart Home" --timestamp "%s" --save test.jpg -s brightness=100%%  --jpeg 100 --skip 10''' % (
                    time_str)
                    os.system(cmd)  # >/dev/null 2>&1
                except Exception as e:
                    print (e)
        except Exception as e:
            print('socket:',e)
            #traceback.print_exc()
            conn.close()
            connect_flag = False
            while not connect_flag:
                try:
                    conn = socket.socket()
                    print('connect ... ')
                    conn.connect(("60.205.189.203", 7777))
                    conn.sendall(bytes('{"heartbeat":"%s"}' % terminal_ID, encoding='utf-8'))
                    connect_flag = True
                except Exception as e:
                    print(e)

def serialTask():
    global serial
    serial_device = '/dev/ttyACM0'
    #serial_device = 'COM3'
    baud_rate = 115200
    serial = MySerial(serial_device, baud_rate)
    while 1 :
        try:
            if serial.isOpened:
                while 1:
                    sensor_data_str = serial.read()
                    if sensor_data_str:
                        sensor_data_str = sensor_data_str.decode("utf-8")
                        print(sensor_data_str)
                        sensor_data_list = sensor_data_str.split('|')
                        for data_str in sensor_data_list:
                            data_list = data_str.split(':')
                            if(len(data_list)==2):
                                for sensor_item in sensor_data.keys():
                                    if data_list[0] == sensor_item[0]:
                                        sensor_data[sensor_item] = data_list[1]
                                        sensor_data['update'] = True
            else:
                serial.open(serial_device, baud_rate)
        except Exception as e:
            print(e)
            serial.isOpened = False

if __name__ == '__main__':
    file_dir = os.path.split(os.path.realpath(__file__))[0]
    print(file_dir)
    os.chdir(file_dir)
    terminal_info_cache_file = 'terminal_info'
    # if cache file not exist, create them
    for cache_file in (terminal_info_cache_file,):
        if not os.path.isfile(cache_file):
            print('%s not exist,create...' % cache_file)
            open(cache_file, 'w').close()
    # load cache file
    terminal_info_cache = FileCache(terminal_info_cache_file)
    terminal_id_t = terminal_info_cache.get('terminal_id')
    first_time_t = terminal_info_cache.get('first_time')
    if terminal_id_t:
        terminal_ID = terminal_id_t
    else:
        terminal_info_cache.set('terminal_id', terminal_ID)
        terminal_info_cache.save()
    if first_time_t:
        first_time = first_time_t
    else:
        terminal_info_cache.set('first_time', int(time.time()))
        terminal_info_cache.save()
        first_time = terminal_info_cache.get('first_time')
    while 1:
        try:
            thread_serialTask = threading.Thread(target=serialTask)
            thread_serialTask.setDaemon(True)
            thread_serialTask.start()
            clientTask()
        except Exception as e :
            pass