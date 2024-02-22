#!/usr/bin/python3 -u

# this requires cython-hidapi:
# pip3 install hidapi
import hid
import os
import time

def send_hid_report(d, x):
    assert(len(x) == 64)
    # Prepend report ID, this is required by the API
    # but it will not go over the wire if the report
    # has no ID in the report descriptor. So we pass
    # 65 bytes to the API but it will send only 64.
    buf = [0] + x
    return d.write(buf)


def send_string(d, text):
    
    # truncate to 63 bytes
    text = text[:63]
    
    # prepare header
    x = [len(text)]
    for c in text:
        x.append(ord(c))
        
    # pad with zero bytes to make
    # bugged windows HID driver happy
    while len(x) < 64:
        x.append(0)
    
    send_hid_report(d, x)
    print("    sent: " + text)

def recv_string(d):
    s = ""
    x = d.read(64)
    if len(x) > 1:
        
        # parse header and
        # extract payload
        size = x[0]
        x = x[1:][:size]
        
        # convert to string
        for c in x:
            s += chr(c)
            
    return s

def send_msg(d, led):
    x = [0] * 64
    x[0] = 255    # magic number
    x[1] = led    # first byte controls blue LED
    send_hid_report(d, x)
    print("    sent: Message packet with {}".format(led))

def main():
    led_toggle = 1
    d = hid.device()
    d.open(0x1FC9, 0x00A2)
       
    while True:
        str = recv_string(d)
        print(str)
        
        if str == "B":
            send_msg(d, 1)
        elif str == "O":
            send_msg(d, 0)

    send_msg(d, 0)
    d.close()

if __name__ == '__main__':
    main()