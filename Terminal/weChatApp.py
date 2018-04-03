# -*- coding: utf-8 -*-

import time
import json
import urllib
from urllib import parse,request
import requests
import os


'''
Python client SDK for Micro Message Public Platform API.
'''

(_HTTP_GET, _HTTP_POST, _HTTP_FILE) = range(3)

_CONTENT_TYPE_MEDIA = ('image/jpeg', 'audio/amr', 'video/mpeg4')

_MEIDA_TYPE = ('image','voice','video','thumb')

_CONTENT_TYPE_JSON= (
    'application/json; encoding=utf-8',
    'application/json',
    'text/plain',
    )

_API_URLS = {
    'index':'https://api.weixin.qq.com/',
    'access_token':'https://api.weixin.qq.com/cgi-bin/token',
    'create_menu':'https://api.weixin.qq.com/cgi-bin/menu/create',
    'delete_menu':'https://api.weixin.qq.com/cgi-bin/menu/delete',
    'upload_media':'https://api.weixin.qq.com/cgi-bin/media/upload',
    'user_info':'https://api.weixin.qq.com/cgi-bin/user/info',
    'QR_LIMIT_SCENE':'https://api.weixin.qq.com/cgi-bin/qrcode/create',
    'get_qr':'https://mp.weixin.qq.com/cgi-bin/showqrcode',
}

def _http_call(the_url, method, token, **kw):
    '''
    send an http request and return a json object  if no error occurred.
    '''
    body = None
    params = None
    boundary = None
    if 'body' in kw.keys():
        body = kw['body']
        del kw['body']
    if method == _HTTP_FILE:
        filename = kw['file']
        del(kw['file'])
    params = parse.urlencode(kw)
    if token == None:
        http_url = '%s?%s' %(the_url, params)
    else:
        the_url = '%s?access_token=%s' %(the_url,token)
        http_url = '%s&%s' %(the_url, params) if (method == _HTTP_GET or method == _HTTP_FILE) else the_url
    http_body = json.dumps(body,ensure_ascii=False)
    http_body = bytes(http_body, 'utf-8')
    if method == _HTTP_FILE:
        resp = requests.post(http_url, files={'image': open(filename, 'rb')})
        response = resp.text
    else:
        req = request.Request(http_url, data = http_body)
        try:
            resp = request.urlopen(req, timeout = 8)
            response = resp.read().decode()
        except Exception as e:
            print(e)
            return None
    try:
        content_type = resp.headers['Content-Type']
    except KeyError as e:
        content_type = '??'
        resp.headers['Content_Type'] = content_type
    print('request: %s' % http_url)
    print('response: %s' % response)
    if content_type in _CONTENT_TYPE_MEDIA:
        print("content_type:",content_type)
        return resp
    elif content_type in _CONTENT_TYPE_JSON:
        jsondata = json.loads(response)
        return jsondata
    else:
        return response


class FileCache(object):
    '''
    the information is temporarily saved to the file.
    '''

    def __init__(self, path):
        self.path = path
        try:
            fd = open(self.path, 'rb'); data = fd.read().decode(); fd.close()
            self.dict_data = json.loads(data)
        except Exception as e:
            print(e)
            self.dict_data = dict()

    def get(self, key):
        return self.dict_data.get(key)

    def set(self, key, value, time = 0):
        self.dict_data[key] = value

    def delete(self, key, time = 0):
        if key in self.dict_data.keys():
            del self.dict_data[key]

    def save(self):
        data = (repr(self.dict_data)).replace('\'', '\"') #json must to use double quotation marks
        #print(data)
        fd = open(self.path, 'w'); fd.write(data); fd.close()

    def remove(self):
        import os
        try:
            os.remove(self.path)
        except Exception as e:
            pass

    def __str__(self):
        return repr(self.dict_data)


class WeiXinClient(object):
    '''
    API clinet using synchronized invocation.
    >>> fc = True
    'use memcache save access_token, otherwise use FileCache, path=[file_path | ip_addr]'
    '''
    def __init__(self, appID, appsecret, path = 'cache'):
        self.api_url= ''
        self.app_id = appID
        self.app_secret = appsecret
        self.access_token = None
        self.expires = 0
        self.access_token_cache_file = '%s/access_token' %(path)
        self.media_id_cache_file = '%s/media_id'%(path)
        self.user_terminal_cache_file = '%s/user_terminal'%(path)
        self.qr_url_scene_cache_file = '%s/qr_url_scene' % (path)
        #if cache file not exist, create them
        for cache_file in (self.access_token_cache_file,self.media_id_cache_file,self.user_terminal_cache_file,self.qr_url_scene_cache_file):
            if not os.path.isfile(cache_file):
                if not os.path.exists(path):
                    os.mkdir(path)
                print('%s not exist,create...'%cache_file)
                open(cache_file,'w').close()
        #load cache file
        self.access_token_cache = FileCache(self.access_token_cache_file)
        self.media_id_cache = FileCache(self.media_id_cache_file)
        self.user_terminal_cache = FileCache(self.user_terminal_cache_file)
        self.qr_url_scene_cache = FileCache(self.qr_url_scene_cache_file)
        print('Load the cache finished.')
        print(self.access_token_cache)


    def request_access_token(self,force_request = False):
        token_key = 'access_token_%s' %(self.app_id)
        expires_key = 'expires_%s' %(self.app_id)
        access_token = self.access_token_cache.get(token_key)
        expires = self.access_token_cache.get(expires_key)
        self.access_token = str(access_token) if access_token else None
        self.expires = expires if expires else None
        if self.is_expires() or force_request:
            print('access_token is invalid.re-request...')
            rjson =_http_call(_API_URLS['access_token'], _HTTP_GET,
                    None, grant_type = 'client_credential',
                    appid = self.app_id, secret = self.app_secret)
            if not rjson:
                return None
            if 'access_token' in rjson.keys():
                self.set_access_token(str(rjson['access_token']),rjson['expires_in'],True)
            else:
                print('Request access_token failed.')

    def del_access_token(self):
        token_key = 'access_token_%s' %(self.app_id)
        expires_key = 'expires_%s' %(self.app_id)
        self.access_token = None
        self.expires = 0
        self.access_token_cache.delete(token_key)
        self.access_token_cache.delete(expires_key)

    def refurbish_access_token(self):
        self.del_access_token()
        self.request_access_token()

    def set_access_token(self, token, expires_in, persistence=False):
        self.access_token = token
        self.expires = expires_in + int(time.time())
        if persistence:
            token_key = 'access_token_%s' %(self.app_id)
            expires_key = 'expires_%s' %(self.app_id)
            self.access_token_cache.set(token_key, self.access_token, time = expires_in)
            self.access_token_cache.set(expires_key, self.expires, time = expires_in)
            self.access_token_cache.save()

    def is_expires(self):
        if self.access_token and self.expires and (self.expires > (time.time()+60)):
            return False
        else:
            return True

    def create_menu(self):
        menu_setting = {
        "button":[
            {
                "name": "我的家居",
                "sub_button": [
                    {
                        "type": "click",
                        "name": "家居状态",
                        "key": "pzzt"
                    },
                    {
                        "type": "click",
                        "name": "安防图像",
                        "key": "pztx"
                    },
                    {
                        "type": "click",
                        "name": "使用时长",
                        "key": "czts"
                    }]
            },
            {
                "name": "设备控制",
                "sub_button": [
                    {
                        "type": "click",
                        "name": "灯光开关",
                        "key": "dkgz"
                    },
                    {
                        "type": "click",
                        "name": "空调开关",
                        "key": "gbgz"
                    },
                    {
                        "type": "click",
                        "name": "窗帘开关",
                        "key": "js"
                    },
                    {
                        "type": "click",
                        "name": "电视开关",
                        "key": "dskg"
                    },
                    {
                        "type": "click",
                        "name": "遥控面板",
                        "key": "ykyd"
                    }]
            },
            {
                "name": "主控管理",
                "sub_button": [
                    {
                        "type": "scancode_waitmsg",
                        "name": "绑定主控",
                        "key": "bdpz"
                    },
                    {
                        "type": "scancode_waitmsg",
                        "name": "解绑主控",
                        "key": "jbpz"
                    }]
            }
            ]
         }
        _http_call(_API_URLS['create_menu'], _HTTP_POST,self.access_token,body = menu_setting)

    def delete_menu(self):
        _http_call(_API_URLS['delete_menu'], _HTTP_GET,self.access_token)

    def set_media_id(self, filename,type, media_id,created_at):
        type_key = 'type_%s'%filename
        media_id_key = 'media_id_%s'%filename
        expires_key = 'expires_%s'%filename
        expires = created_at + 259200 #3 days
        self.media_id_cache.set(type_key, type, time=created_at)
        self.media_id_cache.set(media_id_key, media_id, time=created_at)
        self.media_id_cache.set(expires_key,expires,time=created_at)
        self.media_id_cache.save()


    def upload_madia(self,TYPE,filename):
        rjson = _http_call(_API_URLS['upload_media'], _HTTP_FILE,self.access_token,type=TYPE,file=filename)
        if 'errcode' in rjson.keys():
            self.refurbish_access_token()
            rjson = _http_call(_API_URLS['upload_media'], _HTTP_FILE, self.access_token, type=TYPE, file=filename)
        if 'media_id' in rjson.keys():
            self.set_media_id(filename,rjson['type'],rjson['media_id'],rjson['created_at'])
        else:
            print('Upload file failed.')

    def get_media_id(self,type,filename,force_upload = False):
        if force_upload :
            if os.path.isfile(filename):
                print('Try to upload %s'%filename)
                self.upload_madia(type,filename)
            else:
                print('File %s not exist.')
                return None
        media_id_key = 'media_id_%s'%filename
        expires_key = 'expires_%s' % filename
        expires = self.media_id_cache.get(expires_key)
        if  not expires or expires < (int(time.time())+20):
            print('%s not exist or invalid.'%media_id_key)
            if not force_upload :
                if os.path.isfile(filename):
                    print('Try to upload %s'%filename)
                    self.upload_madia(type,filename)
                else :
                    print('File %s not exist.')
                    return None
        media = self.media_id_cache.get(media_id_key)
        return media

    def get_userinfo(self,openID):
        rjson = _http_call(_API_URLS['user_info'], _HTTP_GET, self.access_token,openid=openID,lang='zh_CN')
        if 'errcode' in rjson.keys():
            self.request_access_token(True)
            rjson = _http_call(_API_URLS['user_info'], _HTTP_GET, self.access_token, openid=openID, lang='zh_CN')
        return rjson

    def set_user_terminal(self, useropenID,terminalID):
        useropenID_key = 'useropenID_%s'%useropenID
        self.user_terminal_cache.set(useropenID_key, terminalID)
        self.user_terminal_cache.save()

    def del_user_terminal(self, useropenID):
        useropenID_key = 'useropenID_%s'%useropenID
        self.user_terminal_cache.delete(useropenID_key)
        self.user_terminal_cache.save()

    def get_user_terminal(self, useropenID):
        useropenID_key = 'useropenID_%s'%useropenID
        terminalID =self.user_terminal_cache.get(useropenID_key)
        return terminalID

    def create_qr(self,scene_id):
        scene_id =  str(scene_id)
        if scene_id in self.qr_url_scene_cache.dict_data.values():
            print('qr_scene_id %s has existed.'%scene_id)
            return
        action_info = {"action_name": "QR_LIMIT_SCENE", "action_info": {"scene": {"scene_id": scene_id}}}
        rjson = _http_call(_API_URLS['QR_LIMIT_SCENE'], _HTTP_POST, self.access_token, body=action_info)
        url_key = 'url'
        ticket_key = 'ticket_%s'%scene_id
        if url_key in rjson.keys():
            self.qr_url_scene_cache.set(rjson[url_key],scene_id)
            self.qr_url_scene_cache.set(ticket_key, rjson['ticket'])
            self.qr_url_scene_cache.save()
        else:
            self.refurbish_access_token()
            rjson = _http_call(_API_URLS['QR_LIMIT_SCENE'], _HTTP_POST, self.access_token, body=action_info)
            if url_key in rjson.keys():
                self.qr_url_scene_cache.set(rjson[url_key], scene_id)
                self.qr_url_scene_cache.set(ticket_key,_API_URLS['get_qr']+'?ticket=%s'%rjson['ticket'])
                self.qr_url_scene_cache.save()
        print(rjson)

    #not supported
    def get_qr(self,ticket):
        res = _http_call(_API_URLS['get_qr'], _HTTP_GET,None, ticket=ticket)



def test():
    ''' test the API '''

    w = WeiXinClient('wx270e18185d243d54','d824322379b3420848c46e0f320faf7e')
    w.request_access_token()
    print(w.access_token)
    #w.upload_madia('image','test1.png')
    #print(w.get_media_id('image','test1.png',True))
    #w.upload_madia('image','test.jpg')
    #print(w.get_media_id('image','test.jpg',True))
    #rjson = w.get_userinfo('ovYEzt3N9DN6qyOKBkHAQ1Oplo2g')

    #print(rjson)
    #w.create_qr(7777)
    #w.get_qr('gQEU8jwAAAAAAAAAAS5odHRwOi8vd2VpeGluLnFxLmNvbS9xLzAyOWNxOEZxWjE4UFQxMDAwMHcwMzcAAgTJigRZAwQAAAAA')
    #exit(0)
    w.delete_menu()
    w.create_menu()

    print(w.__str__())


if __name__ == '__main__':
    test()