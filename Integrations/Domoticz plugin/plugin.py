"""
<plugin key="HeishamonMQTT" name="Heishamon MQTT" version="0.1.2">
    <description>
      Simple plugin to manage Heishamon through MQTT
      <br/>
    </description>
    <params>
        <param field="Address" label="MQTT Server address" width="300px" required="true" default="127.0.0.1"/>
        <param field="Port" label="Port" width="300px" required="true" default="1883"/>
        <param field="Username" label="Username" width="300px"/>
        <param field="Password" label="Password" width="300px" default="" password="true"/>
        <param field="Mode6" label="Debug" width="75px">
            <options>
                <option label="Verbose" value="Verbose"/>
                <option label="True" value="Debug"/>
                <option label="False" value="Normal" default="true" />
            </options>
        </param>
    </params>
</plugin>
"""
errmsg = ""
try:
 import Domoticz
except Exception as e:
 errmsg += "Domoticz core start error: "+str(e)
try:
 import json
except Exception as e:
 errmsg += " Json import error: "+str(e)
try:
 import time
except Exception as e:
 errmsg += " time import error: "+str(e)
try:
 import re
except Exception as e:
 errmsg += " re import error: "+str(e)
try:
 from mqtt import MqttClientSH2
except Exception as e:
 errmsg += " MQTT client import error: "+str(e)
 
def getEnergyNames(pUnitname):
 if pUnitname.startswith('Heat_Energy_') :
  return "Heat_Energy_Consumption","Heat_Energy_Production","Heat_Energy_COP"
 if pUnitname.startswith('Cool_Energy_') :
  return "Cool_Energy_Consumption","Cool_Energy_Production","Cool_Energy_COP"
 if pUnitname.startswith('DHW_Energy_') :
  return "DHW_Energy_Consumption","DHW_Energy_Production","DHW_Energy_COP"
 
def getSplitVal(sValue, index):
 try:
  prevdata = sValue.split(";")
 except:
  prevdata = []
 if len(prevdata)<2:
  prevdata.append(0)
  prevdata.append(0)
 return str(prevdata[index])

def calcCOP(pUnitname):
 lConsName, lProdName, lCopname = getEnergyNames(pUnitname) 
 curCOP = 0
 try:         
  if ( (pUnitname == lConsName) or (pUnitname == lProdName) ):
   curCons = float(getSplitVal(Devices[getDevice(lConsName)].sValue, 0))   
   if (curCons > 0):
    curProd = float(getSplitVal(Devices[getDevice(lProdName)].sValue, 0))    
    try:             
     curCOP = round( (curProd / curCons) , 2)
    except Exception as e:
     curCOP = 0
    Devices[getDevice(lCopname)].Update(nValue=0,sValue=str(curCOP))  
    
 except Exception as e:
  Domoticz.Debug(str(e))  
     
 
def getSelCommand(pUnitname):
    Switcher={
        "Quiet_Mode_Level": "SetQuietMode",
        "Powerful_Mode_Time": "SetPowerfulMode", 
        "Operating_Mode_State": "SetOperationMode",
        "Force_DHW_State": "SetForceDHW",       
        "Heatpump_State": "SetHeatpump",
        "Defrosting_State": "SetForceDefrost",
        "Sterilization_State": "SetForceSterilization",
        "Z1_Heat_Request_Temp": "SetZ1HeatRequestTemperature",
        "Z1_Cool_Request_Temp": "SetZ1CoolRequestTemperature",
        "Z2_Heat_Request_Temp": "SetZ2HeatRequestTemperature",
        "Z2_Cool_Request_Temp": "SetZ2CoolRequestTemperature", 
        "DHW_Target_Temp": "SetDHWTemp"
        }
    return Switcher.get(pUnitname, "")
    
def getSelSwitchLevelNames(pUnitname):
    Switcher={
        "Quiet_Mode_Level": "Off|Silent 1|Silent 2|Silent 3",
        "Force_DHW_State": "Unknown|On|Off",
        "Powerful_Mode_Time": "Off|30 Min|60 Min|90 Min",
        "Operating_Mode_State": "Heat only|Cool only|Auto|DHW only|Heat+DHW|Cool+DHW|Auto+DHW",
        "ThreeWay_Valve_State": "Room|DWH",
        "Holiday_Mode_State": "Off|Scheduled|Active",
        "Cooling_Mode": "Curve|Direct",
        "Heating_Mode": "Curve|Direct"
        }
    return Switcher.get(pUnitname, "")

def getSelSwitchImage(pUnitname):
    Switcher={
        "Quiet_Mode_Level": 8,
        "Force_DHW_State": 0,
        "Powerful_Mode_Time": 0,
        "Operating_Mode_State": 0,
        "ThreeWay_Valve_State": 11,
        "Holiday_Mode_State": 19,
        "Cooling_Mode": 16,
        "Heating_Mode": 15
        }
    return Switcher.get(pUnitname, "")
    
def getDevice(pUnitname):
 iUnit = -1
 for Device in Devices:
  try:
   if (Devices[Device].DeviceID.strip() == pUnitname):
    iUnit = Device
    break
  except:
   pass        
 return iUnit

def createDevice(pUnitname, pTypeName, pOptions=''):
 try:  
  iUnit = 0
  for x in range(1,256):
   if x not in Devices:
    iUnit=x
    break
  if iUnit==0:
   iUnit=len(Devices)+1
  if (pTypeName=="Counter"):
   Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=113, Subtype=0, Switchtype=3, Used=0, DeviceID=pUnitname).Create() # create Counter 
  elif (pTypeName=="Thermostat"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=242, Subtype=1, Used=0, DeviceID=pUnitname).Create() # create Speed counter   
  elif (pTypeName=="Speed"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Subtype=31, Used=0, Options={"Custom": "1;R/Min"}, Image=7,DeviceID=pUnitname).Create() # create Speed counter
  elif (pTypeName=="Text"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Subtype=19, Used=0,DeviceID=pUnitname).Create() # create text device
  elif (pTypeName=="COP"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Subtype=31, Used=0, Options={"Custom": "1;COP"}, DeviceID=pUnitname).Create() # create Speed counter    
  elif (pTypeName=="Pressure"):          
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Subtype=9, Used=0, DeviceID=pUnitname).Create() # create Pressure counter  
  elif (pTypeName=="Kelvin"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Subtype=31, Used=0, Options={"Custom": "1;K"},  DeviceID=pUnitname).Create() # create Kelvin counter  
  elif (pTypeName=="Flow"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Subtype=30, Used=0, DeviceID=pUnitname).Create() # create Kelvin counter
  elif (pTypeName=="Current"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Subtype=23, Used=0, DeviceID=pUnitname).Create() # 
  elif (pTypeName=="Freq"):     
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=243, Options={"Custom": "1;Hz"}, Subtype=31, Used=0, DeviceID=pUnitname).Create() #
  elif (pTypeName=="selSwitch"):     
     lOption = {"Scenes": "|||||", "LevelNames": getSelSwitchLevelNames(pUnitname) , "LevelOffHidden": "false", "SelectorStyle": "0"} #
     Domoticz.Device(Name=pUnitname, Unit=iUnit, Type=244, Subtype=62, Switchtype=18, Options=lOption, Image=getSelSwitchImage(pUnitname), Used=0,DeviceID=pUnitname).Create() # create Selector Switch    
  else:
   Domoticz.Device(Name=pUnitname, Unit=iUnit, TypeName=pTypeName, Used=0, DeviceID=pUnitname).Create() # create Device
  Domoticz.Debug("Created : " + pUnitname + " of type " + pTypeName)
  return iUnit
 except Exception as e:
  Domoticz.Debug(str(e))
  return -1

class BasePlugin:
    mqttClient = None
    
    thermostat_devices = ["Z1_Heat_Request_Temp", "Z1_Cool_Request_Temp", "Z2_Heat_Request_Temp", "Z2_Cool_Request_Temp", "DHW_Target_Temp"]
    switch_devices = ["Quiet_Mode_Schedule", "Main_Schedule_State", "Force_Heater_State", "DHW_Heater_State", "Room_Heater_State", "External_Heater_State", "Internal_Heater_State"]
    command_switch_devices = ["Heatpump_State", "Defrosting_State", "Sterilization_State"]
    command_sel_devices = ["Force_DHW_State", "Quiet_Mode_Level", "Powerful_Mode_Time", "Operating_Mode_State"]   
    sel_switch_devices = [ "ThreeWay_Valve_State", "Holiday_Mode_State", "Cooling_Mode","Heating_Mode"]       
    kWh_devices =["Cool_Energy_Consumption", "Cool_Energy_Production", "DHW_Energy_Consumption", "DHW_Energy_Production", "Heat_Energy_Consumption", "Heat_Energy_Production"]
    counter_devices = ["Operations_Counter", "Operations_Hours", "DHW_Heater_Operations_Hours", "Room_Heater_Operations_Hours", "Sterilization_Max_Time"] 
    speed_devices = ["Pump_Speed", "Fan1_Motor_Speed", "Fan2_Motor_Speed"]   
    pressure_devices = ["Low_Pressure", "High_Pressure"]   
    kelvin_devices = ["Cool_Delta", "DHW_Heat_Delta", "Heat_Delta"]
    text_devices = ["Error"]
    COP_devices = ["Cool_Energy_COP", "DHW_Energy_COP", "Heat_Energy_COP"]
    
    def __init__(self):
     return

    def onStart(self):
     global errmsg
     if errmsg =="":
      try:
        Domoticz.Heartbeat(10)
        self.homebridge = Parameters["Mode2"]
        try:
         self.powerread  = int(Parameters["Mode3"])
        except:
         self.powerread  = 0
        self.debugging = Parameters["Mode6"]
        if self.debugging == "Verbose":
            Domoticz.Debugging(2+4+8+16+64)
        if self.debugging == "Debug":
            Domoticz.Debugging(2)
        self.base_topic = "panasonic_heat_pump" # hardwired
        self.mqttserveraddress = Parameters["Address"].strip()
        self.mqttserverport = Parameters["Port"].strip()
        self.mqttClient = MqttClientSH2(self.mqttserveraddress, self.mqttserverport, "", self.onMQTTConnected, self.onMQTTDisconnected, self.onMQTTPublish, self.onMQTTSubscribed)
        
        for dev in self.COP_devices:
         iUnit = getDevice(dev)         
         if iUnit<0: # if device does not exists in Domoticz, than create it
          iUnit = createDevice(dev, "COP")  
        
      except Exception as e:
        Domoticz.Error("MQTT client start error: "+str(e))
        self.mqttClient = None
     else:
        Domoticz.Error("Your Domoticz Python environment is not functional! "+errmsg)
        self.mqttClient = None

    def checkDevices(self):
        Domoticz.Debug("checkDevices called")

    def onStop(self):
        Domoticz.Debug("onStop called")
    
    def onCommand(self, Unit, Command, Level, Color):  # react to commands arrived from Domoticz       
                
        if self.mqttClient is None:
         return False
         
        try:
         device = Devices[Unit]
         devname = device.DeviceID        
        except Exception as e:
         Domoticz.Debug(str(e))
         return False
        Domoticz.Debug("DevName: " + devname + " Command: " + Command + " " + str(Level) )                       
        if ( devname in self.command_sel_devices ):
         try:           
            cmd = int(Level / 10)
            mqttpath = self.base_topic+"/" + getSelCommand(devname)         
            self.mqttClient.publish(mqttpath, str(cmd) )
         except Exception as e:
          Domoticz.Debug(str(e))
          return False

        if (devname in self.thermostat_devices ):
         try: 
            mqttpath = self.base_topic+"/" + getSelCommand(devname)         
            self.mqttClient.publish(mqttpath, str(Level) )
         except Exception as e:
          Domoticz.Debug(str(e))
          return False         

        if (devname in self.command_switch_devices ):
         try: 
            if (Command == 'off'):
             Level = 0
            else:
             Level = 1          
            mqttpath = self.base_topic + "/" + getSelCommand(devname)         
            self.mqttClient.publish(mqttpath, str(Level) )
         except Exception as e:
          Domoticz.Debug(str(e))
          return False         
          
    def onConnect(self, Connection, Status, Description):
       if self.mqttClient is not None:
        self.mqttClient.onConnect(Connection, Status, Description)

    def onDisconnect(self, Connection):
       if self.mqttClient is not None:
        self.mqttClient.onDisconnect(Connection)

    def onMessage(self, Connection, Data):
       if self.mqttClient is not None:
        self.mqttClient.onMessage(Connection, Data)

    def onHeartbeat(self):
      Domoticz.Debug("Heartbeating...")
      if self.mqttClient is not None:
       try:
        # Reconnect if connection has dropped
        if (self.mqttClient._connection is None) or (not self.mqttClient.isConnected):
            Domoticz.Debug("Reconnecting")
            self.mqttClient._open()
        else:
            self.mqttClient.ping()
       except Exception as e:
        Domoticz.Error(str(e))

    def onMQTTConnected(self):
       if self.mqttClient is not None:
        self.mqttClient.subscribe([self.base_topic + '/#'])

    def onMQTTDisconnected(self):
        Domoticz.Debug("onMQTTDisconnected")

    def onMQTTSubscribed(self):
        Domoticz.Debug("onMQTTSubscribed")
        
    def onMQTTPublish(self, topic, message): # process incoming MQTT statuses

        try:
         topic = str(topic)
         message = str(message)
        except:
         Domoticz.Debug("MQTT message is not a valid string!") #if message is not a real string, drop it
         return False 
        mqttpath = topic.split('/')      

        #------------------ 1Wire ---------------------------------------------
        #----------------------------------------------------------------------
        if ( (mqttpath[0] == self.base_topic) and (mqttpath[1] == '1wire') ):
         #Domoticz.Debug("MQTT 1wire message: " + topic + " " + str(message))
         unitname = mqttpath[2]
         unitname = unitname.strip()
         #Domoticz.Debug("MQTT 1wire message: " + topic + " " + str(message) + ' ' + unitname)
         iUnit = getDevice(unitname)         
         if iUnit<0: # if device does not exists in Domoticz, than create it
          iUnit = createDevice(unitname, "Temperature")
          if iUnit<0:
           return False           
         try:
          mval = float(message)
         except:
          mval = str(message).strip()
         try:
           Devices[iUnit].Update(nValue=0,sValue=str(mval))
         except Exception as e:
           Domoticz.Debug(str(e))
       
        #------------------ SDC ----------------------------------------------
        #---------------------------------------------------------------------
        if ( (mqttpath[0] == self.base_topic) and (mqttpath[1] == 'sdc') ):
         #Domoticz.Debug("MQTT sdc message: " + topic + " " + str(message))
         unitname = mqttpath[2]
         unitname = unitname.strip()

         iUnit = getDevice(unitname)         
         if iUnit<0: # if device does not exists in Domoticz, than create it
          if ( unitname in self.thermostat_devices ):
           iUnit = createDevice(unitname, "Thermostat")  #always before _Temp
          elif ( "_Temp" in unitname ):
           iUnit = createDevice(unitname, "Temperature")
          elif ( unitname in self.switch_devices ):
           iUnit = createDevice(unitname, "Switch")
          elif ( unitname in self.command_switch_devices ):
           iUnit = createDevice(unitname, "Switch")          
          elif ( unitname in self.kWh_devices ):
           iUnit = createDevice(unitname, "kWh")
          elif ( unitname in self.counter_devices ):
           iUnit = createDevice(unitname, "Counter")
          elif ( unitname in self.speed_devices ):
           iUnit = createDevice(unitname, "Speed")
          elif ( unitname in self.pressure_devices ):
           iUnit = createDevice(unitname, "Pressure")
          elif ( unitname in self.kelvin_devices ):
           iUnit = createDevice(unitname, "Kelvin")
          elif ( unitname in self.sel_switch_devices ):
           iUnit = createDevice(unitname, "selSwitch")           
          elif ( unitname in self.command_sel_devices ):
           iUnit = createDevice(unitname, "selSwitch")  
          elif ( unitname in self.text_devices ):
           iUnit = createDevice(unitname, "Text")            
          elif ( unitname == "Pump_Flow" ):
           iUnit = createDevice(unitname, "Flow")
          elif ( unitname == "Compressor_Current" ):
           iUnit = createDevice(unitname, "Current")
          elif ( unitname == "Compressor_Freq" ):
           iUnit = createDevice(unitname, "Freq")
           
          if iUnit<0:
           return False           
         
         #------------------ Temp ---------------------------------------------
         #---------------------------------------------------------------------
         if ( "_Temp" in unitname ):
          try:
           curval = Devices[iUnit].sValue
          except:
           curval = 0
          try:
           mval = float(message)
          except:
           mval = str(message).strip()
          try:
            Devices[iUnit].Update(nValue=0,sValue=str(mval))
          except Exception as e:
            Domoticz.Debug(str(e))
            
         #------------------ Switch ---------------------------------------------
         #-----------------------------------------------------------------------
         if ( ( unitname in self.switch_devices ) or ( unitname in self.command_switch_devices ) ):        
          try:
            scmd = str(message).strip().lower()
            if (str(Devices[iUnit].nValue).lower() != scmd):
             if (scmd == "1"): # set device status if needed
              Devices[iUnit].Update(nValue=1,sValue="On")
             else:
              Devices[iUnit].Update(nValue=0,sValue="Off")
          except Exception as e:
            Domoticz.Debug(str(e))
            return False

         #------------------ Sel Switch -----------------------------------------
         #-----------------------------------------------------------------------
         if ( ( unitname in self.sel_switch_devices ) or ( unitname in self.command_sel_devices ) ):        
          try:
            scmd = int(message) * 10            
            if (str(Devices[iUnit].nValue).lower() != scmd):              
             Devices[iUnit].Update(nValue=2,sValue=str(scmd))
          except Exception as e:
            Domoticz.Debug(str(e))
            return False
            
         #------------------ kWh ------------------------------------------------
         #-----------------------------------------------------------------------
         if ( unitname in self.kWh_devices ):
          try:
           curval = Devices[iUnit].sValue
           prevdata = curval.split(";")
          except:
           prevdata = []
          if len(prevdata)<2:
           prevdata.append(0)
           prevdata.append(0)
          try:
           mval = float(str(message).strip())
          except:
           mval = str(message).strip()
           sval = ""
          sval = str(mval)+";"+str(prevdata[1])
          try:
           if sval!="":
            Devices[iUnit].Update(nValue=0,sValue=str(sval))
          except Exception as e:
           Domoticz.Debug(str(e))
           return True     
          try: 
           calcCOP(unitname)  
          except Exception as e:
           Domoticz.Debug(str(e))            
         
         # ------------------ Pump_Flow-------------------------------------------
         # -----------------------------------------------------------------------
         if ( unitname == "Pump_Flow"):
        
          try:
           mval = float(message)
          except:
           mval = str(message).strip()
          
          try:
            Devices[iUnit].Update(nValue=0,sValue=str(mval))
          except Exception as e:
            Domoticz.Debug(str(e))            

         # ------------------ Speed | Pressue | Counter |  -----------------------
         # -----------------------------------------------------------------------
         if ( (unitname in self.speed_devices) or (unitname in self.pressure_devices) or (unitname in self.counter_devices) or (unitname in self.kelvin_devices) or (unitname == "Compressor_Current") or (unitname == "Compressor_Freq") ):
          try:
           mval = int(message)
          except:
           mval = str(message).strip()
           
          try:         
           Devices[iUnit].Update(nValue=0,sValue=str(mval))
          except Exception as e:
            Domoticz.Debug(str(e))            
            
         # ------------------  Text  ---------------------------------------------
         # -----------------------------------------------------------------------
         if (unitname in self.text_devices):
          try:
           mval = int(message)
          except:
           mval = str(message).strip()
           
          try:         
           Devices[iUnit].Update(nValue=0,sValue=str(mval))
          except Exception as e:
            Domoticz.Debug(str(e))                    
            
global _plugin
_plugin = BasePlugin()

def onStart():
    global _plugin
    _plugin.onStart()

def onStop():
    global _plugin
    _plugin.onStop()

def onConnect(Connection, Status, Description):
    global _plugin
    _plugin.onConnect(Connection, Status, Description)

def onDisconnect(Connection):
    global _plugin
    _plugin.onDisconnect(Connection)

def onMessage(Connection, Data):
    global _plugin
    _plugin.onMessage(Connection, Data)

def onCommand(Unit, Command, Level, Color):
    global _plugin
    _plugin.onCommand(Unit, Command, Level, Color)

def onHeartbeat():
    global _plugin
    _plugin.onHeartbeat()
