import wx
import glob
import serial
import struct
import os
from threading import Thread,Lock
import time
from wx.lib.pubsub import pub
BUFFER_SIZE=128
class myframe(wx.Frame):
    def __init__(self,*args,**kw):
        super(myframe, self).__init__(*args, **kw)
        self.port=''
        self.dev=''
        self.temp_list = glob.glob ('/dev/tty[A-Za-z]*')
        self.baudrate=[9600,57600,115200,222222]
        self.baudrate=[str(i) for i in self.baudrate]
        self.Init()
        
    def Init(self):
	width, height = wx.GetDisplaySize()
        menu=wx.MenuBar()
        fileit1=wx.Menu()
        new_it=fileit1.Append(wx.ID_NEW,'&New','New File')
        open_it=fileit1.Append(wx.ID_OPEN,'&Open','Open File')
        save_it=fileit1.Append(wx.ID_SAVE,'&Save','Save File')
        quit_it=fileit1.Append(wx.ID_EXIT, '&Quit', 'Quit application')
        menu.Append(fileit1,'&File')
        self.Bind(wx.EVT_MENU,self.Quit,quit_it)
        self.Bind(wx.EVT_MENU,self.New,new_it)
        self.Bind(wx.EVT_MENU,self.Open,open_it)
        self.Bind(wx.EVT_MENU,self.Save,save_it)
        self.Bind(wx.EVT_CLOSE,self.close)
        self.pan=wx.Panel(self)
        self.pan.SetBackgroundColour((42,42,42))
	font1 = wx.Font(12,wx.DEFAULT,wx.NORMAL, wx.BOLD)
        vbox=wx.BoxSizer(wx.VERTICAL)
	
        image1 = wx.Image('bootimg.png', wx.BITMAP_TYPE_ANY)
	imageBitmap1 = wx.StaticBitmap(self.pan, wx.ID_ANY, wx.Bitmap(image1)) 
	hbox=wx.BoxSizer(wx.HORIZONTAL)
	
        hbox.Add(imageBitmap1,1,flag=wx.LEFT|wx.RIGHT|wx.EXPAND|wx.ALIGN_CENTRE,border=20)
	hbox1=wx.BoxSizer(wx.HORIZONTAL)
        btn2 = wx.Button(self.pan, label='Refresh', size=(70, 30))
	v_code_text1=wx.StaticText(self.pan, label='Baudrate:', pos=(65, 100))
	v_code_text1.SetForegroundColour((255,255,255))
	v_code_text1.SetFont(font1)
        v_code_text=wx.StaticText(self.pan, label='Port:', pos=(65, 100))
	v_code_text.SetForegroundColour((255,255,255))
	v_code_text.SetFont(font1)
        self.cb = wx.ComboBox(self.pan, pos=(50, 30),choices=self.temp_list,style=wx.CB_READONLY)
        self.cbaudrate = wx.ComboBox(self.pan,pos=(50, 80),choices=self.baudrate,style=wx.CB_READONLY)
	
        hbox1.Add(btn2,0,wx.ALL,border=5)
	hbox1.Add(v_code_text,0,wx.ALL|wx.ALIGN_CENTRE,border=5)
        hbox1.Add(self.cb,0,wx.ALL,border=5)
	hbox1.Add(v_code_text1,0,wx.ALL|wx.ALIGN_CENTRE,border=5)
        hbox1.Add(self.cbaudrate,0,wx.ALL,border=5)
	
             
	hbox2=wx.BoxSizer(wx.HORIZONTAL)
        self.selbtn=wx.Button(self.pan,label="select file",pos=(25,80))
        self.selbtn.Bind(wx.EVT_BUTTON,self.selectfile)
        self.text=wx.TextCtrl(self.pan,value ="",size=(-1,1),style = wx.TE_WORDWRAP)
        hbox2.Add(self.selbtn,0,wx.ALL,border=5)
        hbox2.Add(self.text,1,wx.EXPAND|wx.ALL,border=5)
        hbox3=wx.BoxSizer(wx.HORIZONTAL)
        self.uploadbtn=wx.ToggleButton(self.pan,label="upload",pos=(25,80))
        self.uploadbtn.Bind(wx.EVT_TOGGLEBUTTON,self.upload)
	v_code_text2=wx.StaticText(self.pan, label='Progress Bar:', pos=(65, 100))
	v_code_text2.SetForegroundColour((255,255,255))
	v_code_text2.SetFont(font1)
        self.probar=wx.Gauge(self.pan,range=100,pos=(65,100),size =(200,0), style = wx.GA_HORIZONTAL)
        hbox3.Add(self.uploadbtn,0,wx.ALL,border=5)
	hbox3.Add(v_code_text2,0,wx.ALL|wx.ALIGN_CENTRE_VERTICAL,border=5)
        hbox3.Add(self.probar,0,wx.ALL|wx.EXPAND|wx.ALIGN_CENTRE_HORIZONTAL,border=5)
        self.probar.SetValue(0)
        vbox.Add(hbox,0,wx.EXPAND,border=5)
        vbox.Add(hbox1,0,wx.EXPAND,border=5)
        vbox.Add(hbox2,0,wx.EXPAND,border=5)
	vbox.Add(hbox3,0,wx.EXPAND,border=5)
        self.sb = self.CreateStatusBar()
        self.pan.SetSizer(vbox)
        
        self.Center()
        self.Show()
    def selectfile(self,e):
        with wx.FileDialog(self, "Open python file", wildcard="Binary files (*.bin)|*.bin",style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as fileDialog:
            if fileDialog.ShowModal() == wx.ID_CANCEL:
                return
            pathname = fileDialog.GetPath()
            try:
                self.text.write(pathname)
            except:
                print ("open error file")
    def upload(self,e):
        value=self.uploadbtn.GetValue()
        if value:
	    self.probar.SetValue(0)
            self.dev=self.cb.GetStringSelection()
            self.port=self.cbaudrate.GetStringSelection()
            self.filename=self.text.GetLineText(0)
            
            if not (len(self.dev) and len(self.port) and len(self.filename)):
                message=wx.MessageDialog(None,'select device,port and file', 'Info', wx.OK | wx.ICON_INFORMATION)
                i=message.ShowModal()
                message.Destroy()
                self.uploadbtn.SetValue(False)
            else:
                self.ser=serial.Serial(self.dev,self.port,timeout=2)
		message=wx.MessageDialog(None,'Remove power, press ok before reconnecting power ', 'Info', wx.OK | wx.ICON_INFORMATION)
                i=message.ShowModal()
                message.Destroy()
		s=self.ser.read(10)
	        print ("received")
		if len(s):
		 print ("received data ")
              	 data="".join(i for i in struct.unpack('c'*len(s),s))
		 print data
               	 if data =="up":
		        self.ser.write("do")
	    
		        self.thread1=1
		        try:
		            threadoutput=Thread(target=self.uploadthread)
		            threadoutput.daemon=True
		            threadoutput.start()
		        
		        except:
		            print("thread not created")
		            self.thread1=0
		 else:
		    wx.CallAfter(self.updatestatusbar,"didnt receive upload command ")
		    return
		else:
		    wx.CallAfter(self.updatestatusbar,"didnt receive upload command ")
		    return 
                
             
        else:
            try:
                self.ser.close()
            except:
                print ("in except upload")
                pass
            self.probar.SetValue(0)
    def updatestatusbar(self,e):
        self.sb.SetStatusText(e,0)
   
    def updateprobar(self):
        self.curval=self.probar.GetValue()
        if self.curval+1 < self.probar.GetRange():
                self.probar.SetValue(self.curval+1)
	else:
		self.probar.SetValue(self.probar.GetRange())
        self.updatestatusbar("Current postion of file being uploaded: "+str(self.curval*256))
    def uploadthread(self):
	time.sleep(5)
        f=open(self.filename,"rb")
        statinfo = os.stat(self.filename)
        filesize=statinfo.st_size
	print ("File Size in byte",filesize)
        curpos=0
        pagesize=256
        totalpage=filesize/pagesize
        extrabyte=totalpage%pagesize
	print ("Total Pages",totalpage)
        if extrabyte:
            totalpage+=1
        data=''
        self.probar.SetRange(totalpage)
        
        start_add=134230016
        while(self.thread1 and curpos<filesize):
            time.sleep(.1)
	    self.ser.write([255,255,255,255,start_add&255,start_add>>8&255,start_add>>16&255,start_add>>24&255])
            if(filesize-curpos>255):
                buf=f.read(256)
                #self.ser.write(buf)
            else:
                buf=f.read(filesize-curpos)
		test=struct.pack("B",255) *(256-len(buf))
                buf=buf+test
	    nu_of_retry=0
	    while(nu_of_retry<5):
		    self.ser.write(buf)
		    print (len(buf),curpos,filesize)
		    s=self.ser.read(BUFFER_SIZE)
		    if len(s):
		       data="".join(i for i in struct.unpack('c'*len(s),s))
		       print (curpos)
		       if data =="ok":    
		  
			    curpos+=len(buf)
			    
			    start_add+=256
			    
			   
		            wx.CallAfter(self.updateprobar)
			    break
		       else:
			   print ("error in reveive",data)
		           nu_of_retry+=1
			   time.sleep(.2)
		           wx.CallAfter(self.updatestatusbar,"error in receive ack")

		    else:
		        nu_of_retry+=1	
			time.sleep(.2)
            if nu_of_retry>=5:
                wx.CallAfter(self.updatestatusbar,"Error in upload:max retry occured")
                break
        if curpos>=filesize and self.thread1:
	    
            wx.CallAfter(self.updatestatusbar,"upload complete")
                
        self.uploadbtn.SetValue(False)  
    def close(self,e):
        try:
            self.thread1=0
            time.sleep(.5)
            if self.ser:
                self.ser.close()
        except:
            pass
        self.Destroy()
    def Open(self,e):
        print ("in open")
    def New(self,e):
        print("in new file")
    def Save(self,e):
        print("in save file")
    def Quit(self,e):
        message=wx.MessageDialog(None,'Do You want to Quit', 'Info', 
            wx.YES_NO | wx.ICON_INFORMATION)
        i=message.ShowModal()
        if i == wx.ID_YES:
            try:
                self.thread1=0
                time.sleep(.5)
                if self.ser:
                    self.ser.close()
                
            except:
                pass
            self.Close()
            
        message.Destroy()
app=wx.App()
width, height = wx.GetDisplaySize()
myframe(None,-1,"Sirena Gui",pos=wx.DefaultPosition,size=(640,480))
app.MainLoop()

























