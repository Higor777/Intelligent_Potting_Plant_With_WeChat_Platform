# -*- coding: utf-8 -*-
from flask import Flask, request, redirect, url_for,send_from_directory,session,render_template,abort,make_response
import hashlib
import xml.etree.ElementTree as ET
import json
from werkzeug import secure_filename
import os,time
from weChatApp import WeiXinClient
import threading
import  socketserver,socket
import base64


from gevent import monkey
monkey.patch_all()
from gevent import pywsgi


timeout = 20


global wechat_client
global socket_connection
global online_terminal
global terminal_msg
global timestamp_list

socket_connection = {}
online_terminal = {}
terminal_msg = {}
timestamp_list = []
class Myserver(socketserver.BaseRequestHandler):
    def handle(self):
        global socket_connection
        global terminal_msg
        socket_connection[self.client_address] = self.request
        print(socket_connection)
        conn = self.request
        conn.settimeout(timeout)
        print(self.client_address)
        connected = True
        cur_terminal_id = None
        while connected:
            try:
                global terminal_msg
                global online_terminal
                ret_bytes = conn.recv(1024)
                if ret_bytes:
                    set_str = ret_bytes.decode()
                    print(set_str)
                    rjson = None
                    try:
                        rjson = json.loads(set_str)
                    except Exception as e :
                        print('json.loads error:',e)
                    if rjson:
                        if 'heartbeat' in rjson.keys():
                            if rjson['heartbeat'] not in online_terminal.keys():
                                cur_terminal_id = rjson['heartbeat']
                                online_terminal[rjson['heartbeat']] = self.client_address
                            elif online_terminal[rjson['heartbeat']] != self.client_address:
                                socket_connection[self.client_address].close()
                                del socket_connection[self.client_address]
                                online_terminal[rjson['heartbeat']] = self.client_address
                        if 'msg' in rjson.keys():
                            terminal_msg[cur_terminal_id] = rjson
                    else:
                        print(None)

                #print(ret_bytes)
                #When the terminal offline,sendall will raise a except
                conn.sendall(b'{"heartbeat":0}')
            except Exception as e:
                conn.close()
                print(self.client_address,e)
                if self.client_address in socket_connection.keys():
                    del socket_connection[self.client_address]
                keys = list(online_terminal.keys())
                #print(keys)
                for key in keys:
                    if online_terminal[key] == self.client_address:
                        del online_terminal[key]
                #print(socket_connection)
                #print(online_terminal)
                connected = False





UPLOAD_FOLDER = 'uploads'
ALLOWED_EXTENSIONS = set(['txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif'])

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

if not os.path.exists(app.config['UPLOAD_FOLDER']):
    os.mkdir(app.config['UPLOAD_FOLDER'])

first = 1
@app.route('/',methods=['GET','POST'])
def wechat_auth():
    global wechat_client
    global socket_connection
    global online_terminal
    global terminal_msg
    if request.method == 'GET':
        print('Coming Get')
        '''
        print(online_terminal)
        if not online_terminal:
            print(13246134564516544566666666666666666666666666666666666)
        timet = time.time() + 3
        while timet > time.time():
            pass
        print('time  .')
        '''
        data = request.args
        token = 'Higor'
        signature = data.get('signature','')
        timestamp = data.get('timestamp','')
        nonce = data.get('nonce','')
        echostr = data.get('echostr','')
        if not signature or not timestamp or not nonce or not echostr:
            abort(404)
        s = [timestamp,nonce,token]
        s.sort()
        s = ''.join(s).encode("utf-8")
        if (hashlib.sha1(s).hexdigest() == signature):
            return echostr
    if request.method == 'POST':
        global wechat_client
        print(socket_connection)
        if wechat_client.is_expires():
            wechat_client.request_access_token()
        xml_str = request.stream.read()
        xml = ET.fromstring(xml_str)
        msgType = xml.find('MsgType')
        msgType = msgType.text if hasattr(msgType, 'text') else None
        print(msgType)
        if msgType == 'text':
            response = response_text(xml)
        elif msgType == 'event':
            response = response_event(xml)
        else:
            response = response_unknow(xml)
        return response



def generate_reply(ToUserName,FromUserName,CreateTime,MsgType,**kw):
    reply_base = '''
                <xml>
                <ToUserName><![CDATA[%s]]></ToUserName>
                <FromUserName><![CDATA[%s]]></FromUserName>
                <CreateTime>%s</CreateTime>
                <MsgType><![CDATA[%s]]></MsgType>
                '''%(ToUserName,FromUserName,CreateTime,MsgType)
    if MsgType == 'text' and MsgType in kw.keys():
        reply = '''
                <Content><![CDATA[%s]]></Content>
                </xml>
                 '''%(kw[MsgType])
    elif MsgType == 'image' and MsgType in kw.keys():
        reply = '''
        <Image>
        <MediaId><![CDATA[%s]]></MediaId>
        </Image>
        </xml>
        ''' % (kw[MsgType])
    else:
        reply = '''
                <Content><![CDATA[参数错误]]></Content>
                </xml>
                '''
    reply = reply_base + reply
    return reply

def get_baseinfo(xml):
    return xml.find('ToUserName').text,xml.find('FromUserName').text,xml.find('CreateTime').text
def response_text(xml):
    toUserName,fromUserName,createTime = get_baseinfo(xml)
    content = xml.find('Content').text
    msgId = xml.find('MsgId').text
    print(fromUserName,' text:',content)
    reply = generate_reply(fromUserName,toUserName,createTime,'text',text = '已收到您的消息:%s'%content)
    return reply



def response_event(xml):
    global socket_connection
    global online_terminal
    global terminal_msg
    global wechat_client
    Event = xml.find('Event').text
    if Event == 'CLICK':
        reply = response_event_click(xml)
    elif  Event == 'subscribe' :
        toUserName, fromUserName, createTime = get_baseinfo(xml)
        print(fromUserName, ' evnet_subscribe')
        rjson = wechat_client.get_userinfo(fromUserName)
        EventKey = xml.find('EventKey').text
        if EventKey:
            EventKey = EventKey.replace('qrscene_','')
            scene_id = EventKey
            wechat_client.set_user_terminal(fromUserName, scene_id)
            if 'nickname' in rjson.keys():
                reply = generate_reply(fromUserName,toUserName,createTime,'text',text = 'Hello %s,欢迎订阅智能家居。\n已为您绑定家庭主控：%s'%(rjson['nickname'],scene_id))
            else:
                reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='欢迎订阅智能家居。\n已为您绑定家庭主控：%s'%scene_id)
        else:
            if 'nickname' in rjson.keys():
                reply = generate_reply(fromUserName,toUserName,createTime,'text',text = 'Hello %s,欢迎订阅智能家居。'%rjson['nickname'])
            else:
                reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='欢迎订阅智能家居。')
    elif Event == 'unsubscribe' :
        toUserName, fromUserName, createTime = get_baseinfo(xml)
        print(fromUserName, ' evnet_unsubscribe')
        wechat_client.del_user_terminal(fromUserName)
        reply = 'None'
    elif Event == 'scancode_waitmsg':
        reply = response_evnet_scancode_waitmsg(xml)
    elif Event == 'SCAN':
        toUserName, fromUserName, createTime = get_baseinfo(xml)
        print(fromUserName, ' evnet_SCAN')
        rjson = wechat_client.get_userinfo(fromUserName)
        EventKey = xml.find('EventKey').text
        scene_id = str(EventKey)
        wechat_client.set_user_terminal(fromUserName, scene_id)
        if 'nickname' in rjson.keys():
            reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='Hello %s,\n已为您绑定家庭主控：%s' % (rjson['nickname'], scene_id))
        else:
            reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='已为您绑定家庭主控：%s' % scene_id)
        #reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='二维码参数为：%s' % scene_id)
    else:
        reply = response_unknow(xml)
    return reply

def response_evnet_scancode_waitmsg(xml):
    toUserName, fromUserName, createTime = get_baseinfo(xml)
    print(fromUserName, ' evnet_scancode_waitmsg')
    EventKey = xml.find('EventKey').text
    ScanResult = xml.find('ScanCodeInfo').find('ScanResult').text
    scene_id = wechat_client.qr_url_scene_cache.get(ScanResult)
    if EventKey == 'bdpz':
        if scene_id:
            rjson = wechat_client.get_userinfo(fromUserName)
            wechat_client.set_user_terminal(fromUserName, scene_id)
            print(wechat_client.user_terminal_cache)
            if 'nickname' in rjson.keys():
                reply = generate_reply(fromUserName, toUserName, createTime, 'text',
                                       text='Hello %s。\n已为您绑定家庭主控：%s' % (rjson['nickname'], scene_id))
            else:
                reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='已为您绑定家庭主控：%s' % scene_id)
        else:
            reply = generate_reply(fromUserName, toUserName, createTime, 'text',
                                   text='该二维码不是家庭主控二维码，二维码信息：%s。' % str(ScanResult))
    elif EventKey == 'jbpz':
        if scene_id:
            if scene_id == wechat_client.get_user_terminal(fromUserName):
                rjson = wechat_client.get_userinfo(fromUserName)
                wechat_client.del_user_terminal(fromUserName)
                print(wechat_client.user_terminal_cache)
                if 'nickname' in rjson.keys():
                    reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='Hello %s。\n已为您解绑家庭主控：%s。' % (rjson['nickname'], scene_id))
                else:
                    reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='已为您解绑家庭主控：%s。' % scene_id)
            else:
                reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='您尚未绑定家庭主控：%s。无需解绑。' % scene_id)
        else:
            reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='该二维码不是家庭主控二维码，二维码信息：%s。' % str(ScanResult))
    else:
        reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='该二维码不是家庭主控二维码，二维码信息：%s。' % str(ScanResult))
    return reply

def response_event_click(xml):
    global terminal_msg
    toUserName, fromUserName, createTime = get_baseinfo(xml)
    EventKey = xml.find('EventKey').text
    print(fromUserName,' evnet_key:', EventKey)
    terminal_id = wechat_client.get_user_terminal(fromUserName)
    if not  terminal_id :
        reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='你还未绑定家庭主控，请在主控管理中绑定。')
    elif terminal_id in online_terminal.keys():
        terminal_msg[terminal_id] = None
        socket_connection[online_terminal[terminal_id]].sendall(bytes('''{"cmd":"%s"}''' % EventKey, encoding='utf-8'))
        break_time = time.time() + 4.5
        while not terminal_msg[terminal_id] and break_time > time.time():
            time.sleep(0.1)
        if not terminal_msg[terminal_id]:
            print('timeout')
            reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='网络传输超时，您的家庭主控未及时响应，请检查家庭主控网络是否良好。')
        else:
            if EventKey == 'ykyd':
                if terminal_msg[terminal_id]['msg'] == 'on':
                    timestamp = int(time.time() + 10 * 60)
                    for t in timestamp_list:
                        if t < time.time():
                            timestamp_list.remove(t)
                    timestamp_list.append(timestamp)
                    timestamp_str = str(timestamp)
                    id_str = '%s:%s' % (terminal_id, timestamp_str)
                    id_bytes = id_str.encode(encoding="utf-8")
                    id_b64encode = base64.b64encode(id_bytes).decode()
                    control_url = 'http://60.205.189.203/control?id=%s' % id_b64encode
                    # print(control_url)
                    reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='请点击链接进行遥控，链接有效时间10分钟：\n%s' % control_url)
                elif terminal_msg[terminal_id]['msg'] == 'off':
                    reply = generate_reply(fromUserName, toUserName, createTime, 'text',text='家庭主控在线，但控制器未连接。')
                else:
                    reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='家庭主控终端回复了未知信息。')
            elif terminal_msg[terminal_id]['msg'] == 'text':
                reply = generate_reply(fromUserName, toUserName, createTime, 'text', text=terminal_msg[terminal_id]['text'])
            elif terminal_msg[terminal_id]['msg'] == 'image':
                reply = generate_reply(fromUserName, toUserName, createTime, 'image', image=terminal_msg[terminal_id]['image'])
            else:
                reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='家庭主控终端回复了未知信息。')
    else:
        print('tarminal offlinee')
        reply = generate_reply(fromUserName, toUserName, createTime, 'text', text='您的家庭主控处于离线状态，请检查家庭主控网络状态。')
    return reply



def response_unknow(xml):
    toUserName, fromUserName, createTime = get_baseinfo(xml)
    reply = '''
        <xml>
        <ToUserName><![CDATA[%s]]></ToUserName>
        <FromUserName><![CDATA[%s]]></FromUserName>
        <CreateTime>%s</CreateTime>
        <MsgType><![CDATA[%s]]></MsgType>
        <Content><![CDATA[%s]]></Content>
        </xml>
        ''' % (
        fromUserName,
        toUserName,
        createTime,
        'text',
        'Unknow Format, Please check out'
    )
    return reply


'''
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS
'''

@app.route('/upload', methods=['POST','GET'])
def upload_file():
    abort(404)
    if request.method == 'POST':
        reply = '''{"code":-1}'''
        file = request.files['file']
        if file and allowed_file(file.filename):
            try:
                filename = secure_filename(file.filename)
                file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
                #return redirect(url_for('uploaded_file',filename=filename))
                reply = '''{"code":0}'''
            except Exception as e:
                print(e)
        return reply
    return '''
    <!doctype html>
    <title>Upload new File</title>
    <h1>Upload new File</h1>
    <form action="" method=post enctype=multipart/form-data>
      <p><input type=file name=file>
         <input type=submit value=Upload>
    </form>
    '''

'''
@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return send_from_directory(app.config['UPLOAD_FOLDER'],filename)
'''


@app.route('/control', methods=['GET'])
def control():
    id_b64encode = request.args.get('id', '')
    if id_b64encode:
        id_b64encode_bytes = id_b64encode.encode(encoding='utf-8')
        id = base64.b64decode(id_b64encode_bytes).decode()
        print(id)
        id_args = id.split(':')
        if len(id_args) == 2:
            terminal_id = id_args[0]
            try:
                time_stamp = int(id_args[1])
            except Exception as e:
                print(e)
                reply = '参数错误。'
            else:
                if time_stamp < time.time() or time_stamp not in timestamp_list:
                    reply = '链接已失效，请重新获取。'
                else:
                    if terminal_id in online_terminal.keys():
                        reply = make_response(render_template("control.html",terminal_id=id_b64encode))
                    else:
                        reply = '您的家庭主控已离线。'
        else:
            reply = '参数错误。'
    else:
        abort(404)
    return reply




@app.route('/handle', methods=['GET'])
def handle():
    try:
        id_cmd_b64encode = request.args.get('type', '')
    except ValueError:
        abort(404)
    else:
        id_cmd_args = id_cmd_b64encode.split(':')
        if len(id_cmd_args) == 2:
            id_b64encode = id_cmd_args[0]
            cmd = id_cmd_args[1]
            if id_b64encode:
                id_b64encode_bytes = id_b64encode.encode(encoding='utf-8')
                id = base64.b64decode(id_b64encode_bytes).decode()
                id_args = id.split(':')
                if len(id_args) == 2:
                    terminal_id = id_args[0]
                    try:
                        time_stamp = int(id_args[1])
                    except Exception as e:
                        print(e)
                        reply = '参数错误。'
                    else:
                        if time_stamp < time.time():
                            reply = '链接已失效，请重新获取。'
                        else:
                            if terminal_id in online_terminal.keys():
                                socket_connection[online_terminal[terminal_id]].sendall(bytes('''{"cmd":"%s"}''' % cmd, encoding='utf-8'))
                                reply = 'ok'
                            else:
                                reply = '您的家庭主控已离线。'
                else:
                    reply = '参数错误。'
            else:
                reply = '参数错误。'
        else:
            reply = '参数错误。'
    return reply



_ALL_TERMINAL_ID = [7777]
if __name__ == '__main__':
    wechat_client = WeiXinClient('wx270e18185d243d54','d824322379b3420848c46e0f320faf7e')
    wechat_client.request_access_token()
    for terminalID in _ALL_TERMINAL_ID:
        wechat_client.create_qr(terminalID)
    print('Create socketserver')
    server = socketserver.ThreadingTCPServer(("0.0.0.0",7777),Myserver)
    #server.serve_forever()
    thread_socketserver = threading.Thread(target=server.serve_forever)
    thread_socketserver.setDaemon(True)
    thread_socketserver.start()

    #app.debug=True
    #app.run(host='0.0.0.0',port=80)#,processes=17)

    server = pywsgi.WSGIServer(('0.0.0.0', 80), app)
    print('* Running on http://0.0.0.0:80/ (Press CTRL+C to quit)')
    server.serve_forever()


    print('Close connection ...')
    for conn in socket_connection.values():
        conn.close()
    print('exit ...')