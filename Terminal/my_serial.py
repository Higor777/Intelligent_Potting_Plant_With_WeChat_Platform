import serial
import struct
import time


cmd={
    'forward':0x01,
    'backward':0x02,
    'right':0x03,
    'left':0x04,
    'pause':0x05,
    'anticw':0x06,
    'cw':0x07,
    'dkgz':0x08,
    'gbgz':0x09,
    'js':0x0A,
    'dskg':0x0B,
}
class MySerial(object):
    def __init__(self,serial_device=None,baud_rate=None):
        self.serial_device = serial_device
        self.baud_rate = baud_rate
        self.isOpened = False
        if serial_device!=None and baud_rate!=None :
            try :
                self.ser = serial.Serial(serial_device,baud_rate)
            except :
                print('Open: '+serial_device+' failed!')
                print ('Please make sure that device exists!')
            else:
                print ('Open: '+self.ser.portstr+' succeed')
                self.flushInput()
                self.isOpened = True
        else :
            print ('Please input information of serial_device!')

    def open(self,serial_device=None,baud_rate=None):
        if serial_device != None:
            self.serial_device = serial_device
        if baud_rate != None:
            self.baud_rate  = baud_rate
        if self.serial_device!=None and self.baud_rate!=None :
            try :
                self.ser = serial.Serial(serial_device,baud_rate)
            except :
                pass
            else:
                print ('Open: '+self.ser.portstr+' succeed')
                self.flushInput()
                self.isOpened = True

    def send(self,data_list):
        data = bytes(data_list)
        self.ser.write(data)

    def sendSpeedPack(self,float0,float1,float2):
        head = 0x55
        length  = 0x1C
        self.checksum = 0
        list_temp = []
        list_temp.append(head)
        self.checksum = self.checksum + head
        list_temp.append(length)
        self.checksum = self.checksum + length
        data = struct.pack('fff',float0,float1,float2)
        for i in range(0,len(data)):
            item = data[i]
            list_temp.append(item)
            self.checksum = self.checksum + item
        self.checksum = self.checksum%255
        list_temp.append(self.checksum)
        self.send(list_temp)

    def sendCmdPack(self,rjson):
        if 'cmd' in rjson.keys():
            head = 0x55
            length = 0x21
            self.checksum = 0
            list_temp = []
            list_temp.append(head)
            self.checksum = self.checksum + head
            list_temp.append(length)
            self.checksum = self.checksum + length
            if rjson['cmd'] in cmd.keys():
                list_temp.append(cmd[rjson['cmd']])
                self.checksum = self.checksum + cmd[rjson['cmd']]
            else:
                print('Unkown cmd.')
                return 0
            self.checksum = self.checksum % 255
            list_temp.append(self.checksum)
            self.send(list_temp)
            return True
        else:
            return False

    def read(self):
        self.count = self.ser.inWaiting()
        if self.count != 0:
            self.recv = self.ser.read(self.count)
            self.ser.flushInput()
            return self.recv
        else:
            return None

    def flushInput(self):
        self.ser.flushInput()

    def close(self):
        print('Close: %s' % self.serial_device)
        self.isOpened = False
        self.ser.close()



#serial_device = '/dev/ttyUSB0'
serial_device = 'COM10'
baud_rate = 115200
#test
if __name__ == '__main__':
    ser = MySerial(serial_device,baud_rate)
    if ser.isOpened :
        ser.send([1,2,3,4,5,6,7,8,0xff])
        ser.sendSpeedPack(1.23,0.0,0.0)
        time.sleep(1)
        data = ser.read()
        print(data)
        ser.close()

